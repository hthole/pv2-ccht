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


__global__ void prefix_sum(int *array, int *tmp_array, int elements);
__device__ __host__ long oplus (const long a, const long b);
void read_file(int[], int, char[]);
void print_debug(int list[], int tmp[], int count);






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
	int *array_host, *array_host_tmp, *array_device, *array_device_tmp;

	// konvertiere die Uebergabeparameter von char zu int
	const int elements = atoi(argv[2]);

	/* Array für Host und Device allokieren */
	size_t size = elements * sizeof(int);
	array_host = (int *) malloc(size);
	array_host_tmp = (int *) malloc(size);
	cudaMalloc((void **) &array_device, size);
	cudaMalloc((void **) &array_device_tmp, size);


	/* Datei in Hostarray einlesen */
	read_file(array_host, elements, argv[3]);
	read_file(array_host_tmp, elements, argv[3]);

	/* Hostarray zu CUDA-Device kopieren */
	cudaMemcpy(array_device, array_host, size, cudaMemcpyHostToDevice);
	cudaMemcpy(array_device_tmp, array_host, size, cudaMemcpyHostToDevice);

	int i=0;
	for (i = 1; i < elements; i++) {
		array_host_tmp[i] = array_host_tmp[i - 1] + array_host_tmp[i];
	}



	// Do calculation on device:
	prefix_sum <<< 1, elements >>> (array_device, array_device_tmp, elements);
	// Retrieve result from device and store it in host array
	cudaMemcpy(array_host, array_device, size, cudaMemcpyDeviceToHost);

	// Print results
	//print_debug(array_host, array_host_tmp, elements);

	for (int i = 0; i < elements; i++) {
		assert (array_host[i] == array_host_tmp[i]);
	}

	// Cleanup
	free(array_host);
	cudaFree(array_device);
}



/* Unser Kernel */
__global__ void prefix_sum(int *array, int *array_tmp, int elements) {

	// Dynamically allocated shared memory for scan kernels
	extern __shared__ float temp[];

	int thid = threadIdx.x;

	int offset = 1;

	// Cache the computational window in shared memory
	temp[2 * thid] = array_tmp[2 * thid];
	temp[2 * thid + 1] = array_tmp[2 * thid + 1];

	// build the sum in place up the tree
	for (int steps = elements/2; steps > 0; steps /= 2) {
		__syncthreads();

		if (thid < steps) {
			int first = offset * (2 * thid + 1) - 1;
			int second = offset * (2 * thid + 2) - 1;

			temp[second] += temp[first];
		}

		offset *= 2;
	}

	// scan back down the tree

	// clear the last element
	int tmp_var = 0;
	if (thid == 0) {
		tmp_var = temp[elements - 1];
		temp[elements - 1] = 0;
	}

	// traverse down the tree building the scan in place
	for (int steps = 1; steps < elements; steps *= 2) {
		offset /= 2;
		//__syncthreads(); <-------------------------------- unnoetig??????

		if (thid < steps) {
			int first = offset * (2 * thid + 1) - 1;
			int second = offset * (2 * thid + 2) - 1;

			float t = temp[first];
			temp[first] = temp[second];
			temp[second] += t;
		}
	}



	__syncthreads();

	// write results to global memory
	array[(2 * thid) - 1] = temp[2 * thid];
	array[2 * thid] = temp[2 * thid + 1];

	__syncthreads();

	if (thid == 0) {
			array[elements-1] = tmp_var;
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
