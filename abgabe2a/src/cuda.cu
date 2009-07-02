/*
 *
 *      Author: C. Claus, H. Thole
 *
 *      Kompilieren mit:
 *      nvcc -O2 -g -deviceemu cuda.cu -o cuda
 *
 *      oder
 *
 *      nvcc -O2 -g cuda.cu -o cuda
 *
 *
 *      Ausführen mit:
 *      ./cuda  <Device ID> <Anzahl der Zahlen> <Dateiname>
 *
 *      Die einzulesenden Dateien muessen sich in "../resources/" befinden.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <cuda.h>


#define FILE_PATH	"./resources/"
#define FILE_MODE	"r"
#define TOKEN 		" "
#define BUF_SIZE	500000


__global__ void prefix_sum(int *array, float *all_sums_device);
__global__ void prefix_all_sum(int *array, float *all_sums_device) ;
__device__ __host__ long oplus (const long a, const long b);
void read_file(int[], int, char[]);
void print_debug(int list[], int tmp[], int count);
void checkCUDAError(const char *msg);






int main(int argc, char **argv) {
	/* Parameteranzahl pruefen */
	if (argc != 4) {
		printf("usage: %s <device id> <size> <filename>\n", argv[0]);
		exit(1);
	}

	//double start_time, end_time; // fuer Zeitnahme

	/* Device setzen */
	cudaSetDevice (atoi(argv[1]));

	/*  Host- und Devicearray einrichten */
	int *array_host, *array_host_seq, *array_device;
	float *all_sums, *all_sums_device;

	// konvertiere die Uebergabeparameter von char zu int
	const int elements = atoi(argv[2]);

	int j = 0;
	while (ldexp(1,j) < elements) {
		j++;
		printf("%d ",j);
	}

	if (ldexp(1,j) == elements) {
		printf("zweierpotenz!\n");
	} else {
		printf("\n%d -- keine zweierpotenz!\n", j);
	}

	/* Array für Host und Device allokieren */
	size_t size = elements * sizeof(int);
	array_host = (int *) malloc(size);
	array_host_seq = (int *) malloc(size);
	cudaMalloc((void **) &array_device, size);

	if (array_host == NULL) {
		printf("Nicht genug Speicher...\n");
		exit(0);
	}
	if (array_host_seq == NULL) {
		printf("Nicht genug Speicher...\n");
		exit(0);
	}


	/* Datei in Hostarray einlesen */
	read_file(array_host, elements, argv[3]);
	read_file(array_host_seq, elements, argv[3]);

	/* Hostarray zu CUDA-Device kopieren */
	cudaMemcpy(array_device, array_host, size, cudaMemcpyHostToDevice);

	int i=0;
	for (i = 1; i < elements; i++) {
		array_host_seq[i] = array_host_seq[i - 1] + array_host_seq[i];
	}

	int ergebnis = 0;
	for (i = 0; i < elements; i++) {
		ergebnis += array_host_seq[i];
	}

	// just for developing block-stuff
	int num_blocks = 2;
	int num_threads_per_block = elements / (2* num_blocks);

	// all_sums array einrichten... ein feld pro block
	size_t size_all_sums = num_blocks * sizeof(float);
	all_sums = (float *) malloc(size_all_sums);
	cudaMalloc((void **) &all_sums_device, size_all_sums);

	// und rueber damit...
	//cudaMemcpy(all_sums_device, all_sums, size_all_sums, cudaMemcpyHostToDevice);

//	const int max_threads = 512; // max threads pro block
//	const int num_blocks = elements / max_threads; // anzahl der blocks
//	const int num_threads_per_block = max_threads/2; // anzahl der tatsaechlichen threads pro block

	// Do calculation on device:
	prefix_sum <<< num_blocks, num_threads_per_block >>> (array_device, all_sums_device);
	// Retrieve result from device and store it in host array
	//cudaMemcpy(array_host, array_device, size, cudaMemcpyDeviceToHost);

	// block until the device has completed
	    cudaThreadSynchronize();
	    checkCUDAError("kernel invocation");



	cudaMemcpy(all_sums, all_sums_device, size_all_sums, cudaMemcpyDeviceToHost);

	for (int m = 1; m <= num_blocks; m++) {
		printf("all_sums: %f\n", all_sums[m-1]);
	}

	// zwischensummen berechnen
	for (int m = 1; m <= num_blocks; m++) {
		all_sums[m] = all_sums[m - 1] + all_sums[m];
	}

	// auf dem device weiterrechnen
	cudaMemcpy(all_sums_device, all_sums, size_all_sums, cudaMemcpyHostToDevice);
	prefix_all_sum <<< num_blocks, num_threads_per_block >>> (array_device, all_sums_device);

	// block until the device has completed
	    cudaThreadSynchronize();
	    checkCUDAError("kernel invocation");



	cudaMemcpy(all_sums, all_sums_device, size_all_sums, cudaMemcpyDeviceToHost);

	for (int m = 1; m <= num_blocks; m++) {
		printf("ende all_sums: %f\n", all_sums[m-1]);
	}

	float overall_sum = 0;
	for (int v = 0; v<num_blocks; v++) {
		overall_sum += all_sums[v];
	}







	// Print results
	//print_debug(array_host, array_host_seq, elements);

	printf ("%f <--- korrekt waere %i \n", overall_sum, ergebnis);

	for (int i = 0; i < elements; i++) {
		//assert (array_host[i] == array_host_seq[i]);
	}

	// Cleanup
	free(array_host);
	free(array_host_seq);
	cudaFree(array_device);
}



/* Unser Kernel */
__global__ void prefix_sum(int *array, float *all_sums_device) {

	// shared temp array
	extern __shared__ int temp[];

	int tx = threadIdx.x;
	int bx = blockIdx.x;
	int dim = blockDim.x;

	int offset = 1;

	// schreibe das array in shared speicher
	temp[2 * tx] = array[2 * tx + (bx*dim*2)];
	temp[2 * tx + 1] = array[2 * tx + 1 + (bx*dim*2)];

	// teilsummen bilden
	for (int steps = dim; steps > 0; steps /= 2) {
		__syncthreads();

		if (tx < steps) {
			int first = offset * (2 * tx + 1) - 1;
			int second = offset * (2 * tx + 2) - 1;

			temp[second] += temp[first];
		}

		offset *= 2;
	}

	//
	//
	//

	__syncthreads();

	// letztes element sichern, dann nullen, damit das ergebnis korrekt wird
	int tmp_var = -1;
	if (tx == 0) {
		tmp_var = temp[(2 * dim) - 1];
		all_sums_device[bx] = tmp_var;
		temp[(2 * dim) - 1] = 0;
		//printf("var oben: %i\n", tmp_var);
		//printf("all_sums_device: %f\n", all_sums_device[bx]);
	}

	__syncthreads();

	// werte durchtauschen
	for (int steps = 1; steps <= dim; steps *= 2) {
		offset /= 2;
		__syncthreads(); //<-------------------------------- unnoetig??????

		if (tx < steps) {
			int first = offset * (2 * tx + 1) - 1;
			int second = offset * (2 * tx + 2) - 1;

			int t = temp[first];
			temp[first] = temp[second];
			temp[second] += t;
		}
	}



	__syncthreads(); //<-------------------------------- unnoetig??????

	// ins array zurückschreiben
	array[2 * tx + (bx*dim)] = temp[2 * tx];
	array[2 * tx + 1 + (bx*dim)] = temp[2 * tx + 1];

	__syncthreads();

	if (tx == 0) {
		//printf("stelle im array: %i\n", ntpb-1 + (bx*ntpb));
		//printf("var unten: %i\n", tmp_var);
		array[bx * ((2 * dim) - 1)] = tmp_var;
	}

	//printf("thread %i in block % i -- %i\n", tx, bx, array[2 * tx + (bx*dim)]);
	//printf("thread %i in block % i -- %i\n", tx, bx, array[2 * tx + 1 + (bx*dim)]);
	__syncthreads();
}

__global__ void prefix_all_sum(int *array, float *all_sums_device) {

	// shared temp array
	extern __shared__ int temp[];


	int tx = threadIdx.x;
	int bx = blockIdx.x;
	int dim = blockDim.x;

	int offset = 1;

	// schreibe das array in shared speicher
	temp[2 * tx] = array[2 * tx + (bx*dim*2)];
	temp[2 * tx + 1] = array[2 * tx + 1 + (bx*dim*2)];

	__syncthreads();

	// addiere zwischensummen auf
	if (bx > 0) {
		temp[2 * tx] += all_sums_device[bx - 1];
		temp[2 * tx + 1] += all_sums_device[bx - 1];
	}

	__syncthreads();

	/*
	 * wir haben jetzt korrekte teilsummen... nun wieder obersumme im threadblock bilden, in all_sums schreiben,
	 * zurück zum host und aufaddieren
	 */

	// teilsummen bilden
	for (int steps = dim; steps > 0; steps /= 2) {
		__syncthreads();

		if (tx < steps) {
			int first = offset * (2 * tx + 1) - 1;
			int second = offset * (2 * tx + 2) - 1;

			temp[second] += temp[first];
		}

		offset *= 2;
	}

	__syncthreads();

	if (tx == 0) {
		all_sums_device[bx] = temp[(dim*2) - 1];
		//printf("ende all_sums_device: %f\n", all_sums_device[bx]);
	}


}


/* Eine tolle assoziative Device-Funktion ... */
__device__ __host__ long oplus (const long a, const long b) {
	return a + b;
}



void read_file(int list[], int count, char *filename) {
	FILE *file;
	char line[BUF_SIZE];
	char *z;
	char *abs_filename = (char *) malloc(strlen(filename) + strlen(FILE_PATH)
			+ 1);
	long cols = 0;

	strcpy(abs_filename, FILE_PATH);
	strcat(abs_filename, filename);

	file = fopen(abs_filename, "r");

	if (file == NULL) {
		printf("Datei %s konnte nicht geoeffnet werden.\n", abs_filename);
	} else {
		while (fgets(line, sizeof(line), file) != NULL) {
			// fuer jede zeile
			z = strtok(line, TOKEN);

			while (z != NULL && cols < count) {
				// fuer jede spalte (zahl) in der zeile
				list[cols] = atoi(z);
				z = strtok(NULL, TOKEN);
				cols++;
			}
		}
	}
	free(abs_filename);

	fclose(file);
}

void print_debug(int list[], int tmp[], int count) {
	int i;

	for (i = 0; i < count; i++) {
		fprintf(stderr, "%i. %i sollte sein %i\n", i, list[i], tmp[i]);
	}
}

void checkCUDAError(const char *msg)
{
    cudaError_t err = cudaGetLastError();
    if( cudaSuccess != err)
    {
        fprintf(stderr, "Cuda error: %s: %s.\n", msg,
                                  cudaGetErrorString( err) );
        exit(EXIT_FAILURE);
    }
}

