//==============================================================================
// Name        : cuda2.cpp
// Author      : Hendrik Thole & Christian Claus
// Version     :
// Copyright   : GNU Generel Public License v2 or later
// Compile     : Use the nvcc compiler
//               nvcc -O2 cuda2.cu -o calc
//
// Ressources  : The ressources folder has plain text files. Those contain
//				 numbers used to calculate the prefix sum.
//				 Valid format: "12 23 34 4556 0 1234" etc.
//==============================================================================

#include <stdio.h>
#include <cuda.h>

#define FILE_PATH		"./resources/"
#define FILE_MODE		"r"
#define TOKEN 			" "
#define BUF_SIZE		500000
#define EXIT_SUCCESS	0
#define EXIT_FAILURE	1


#define MAX_THREADS 	512

//==============================================================================

__global__ void prefix_block_sum(float *array, int elements);
__global__ void prefix_overall_sum(float *array, int elements);
__global__ void prefix_finalize(float *array, int elements);
__global__ void prefix_finalize_to_x(float *array, int elements);
__device__ __host__ long oplus (const long a, const long b);

void read_file(float[], int, char[]);
void checkCUDAError(const char *msg);

//==============================================================================

int main(int argc, char **argv) {

	if (argc != 4) {
		printf("usage: %s <device id> <size> <filename>\n", argv[0]);
		exit(1);
	}
	
	//--------------------------------------------------------------------------

	/* which device do we want to use? */
	int device = atoi(argv[1]);
	cudaSetDevice(device);
	
	//--------------------------------------------------------------------------

	/* we need arrays on host and device */
	float *array_host, *array_device;

	const int elements = atoi(argv[2]);


	int threads 	   = MAX_THREADS; // or change to whatever you want
	int sub_blocks	   = 10;
	int blocks  	   = elements / threads / sub_blocks;
	int offset		   = elements % threads;
	
	if (offset != 0) {
		blocks++;	
	}
	
	if (blocks == 0) {
		blocks = 1;
	}
	
	printf("blocks:  %d\n", blocks);
	printf("threads: %d\n", threads);
	
	
	/* 
	 * Array für Host und Device allokieren
	 */
	
	size_t size 		 = elements * sizeof(float);
	array_host  = (float *) malloc(size);
	cudaMalloc((void **) &array_device, size);

	if (array_host == NULL) {
		printf("doh! not enough memory...\n");
		
		return EXIT_FAILURE;
	}
	
	//--------------------------------------------------------------------------

	/* let's read the file into the host's array */
	read_file(array_host, elements, argv[3]);


	//--------------------------------------------------------------------------
	
	cudaMemcpy(array_device, array_host, size, cudaMemcpyHostToDevice);
	
	prefix_block_sum <<< blocks, threads >>> (array_device, elements);
	cudaThreadSynchronize();
	checkCUDAError("kernel invocation");
	
	prefix_overall_sum <<< 1, 1 >>> (array_device, elements);
	cudaThreadSynchronize();
	checkCUDAError("kernel invocation");
	
	prefix_finalize <<< blocks, threads >>> (array_device, elements);
	cudaThreadSynchronize();
	checkCUDAError("kernel invocation");
	
	prefix_finalize_to_x <<< 1, threads >>> (array_device, elements);
	cudaThreadSynchronize();
	checkCUDAError("kernel invocation");
	
	
	cudaMemcpy(array_host, array_device, size, cudaMemcpyDeviceToHost);
	
	printf("x = %f\n", array_host[0]);
	
	//--------------------------------------------------------------------------
	
	// cleanup
	free(array_host);
	cudaFree(array_device);
	
	return EXIT_SUCCESS;
}

//==============================================================================

/* each thread calcs a prefix sum in it's own */
__global__ void prefix_block_sum(float *array, int elements) {
	int uid   		 = blockIdx.x * blockDim.x + threadIdx.x;
	int start 		 = uid * 10;
	
	if (start < elements) {
		int end = start + 10;
		long sum = 0;
		
		for (int i = start; i < end && i < elements; i++) {
			sum = oplus(sum, array[i]);
			array[i] = sum;
		}
	}
	
}

//==============================================================================

/* prefix sum over the last element each thread calc'ed is generated */
__global__ void prefix_overall_sum(float *array, int elements) {
	long sum = 0;
	
	for (int i = 10; i < elements; i += 10) {
		sum  = oplus(sum, array[i - 1]);
		array[i - 1] = sum;
	}
}

//==============================================================================

/* now we let each thread add those to 'his' block of numbers */
__global__ void prefix_finalize(float *array, int elements) {
	unsigned int uid   = blockIdx.x * blockDim.x + threadIdx.x;
	unsigned int start = 10 * (uid + 1);

	if (start < elements) {
		unsigned int end = start + 10;

		for (unsigned int i = start; i < end - 1 && i < elements; i++) {
			array[i] = oplus(array[i], array[start - 1]);
		}
	}
}

//==============================================================================

/* calculate the final x */
__global__ void prefix_finalize_to_x(float *array, int elements) {
	
	unsigned int tx = threadIdx.x;
	unsigned int dim = blockDim.x;

	__shared__ float sum[MAX_THREADS];

	sum[tx] = 0;


	unsigned int add_parts = elements / dim;

	unsigned int arr_offset = elements%dim;

	// offset
	if (arr_offset != 0 && tx < arr_offset) {
		sum[tx] += array[dim * add_parts + tx];
	}

	for (unsigned int i = 0; i < add_parts; i++) {
		sum[tx] += array[tx * add_parts + i];
	}
	
	__syncthreads();
	
	// reduce
	for(unsigned int offset=dim>>1; offset>0; offset = offset >>1) {
		if(tx < offset) {
			sum[tx] += sum[tx + offset];
		}
		
		__syncthreads(); 
	} 
	
	if (tx == 0) {
		array[tx] = sum[tx];
	}
	
}
//==============================================================================

/* the function can be changed here, usually prefix *sums* are '+' ;-)
 * this may just give some flexibility
 */
__device__ __host__ long oplus (const long a, const long b) {
	return a + b;
}

//==============================================================================

void read_file(float list[], int count, char *filename) {
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
		printf("file %s could not be opened.\n", abs_filename);
	} else {
		while (fgets(line, sizeof(line), file) != NULL) {
			z = strtok(line, TOKEN);

			while (z != NULL && cols < count) {
				list[cols] = atoi(z);
				z = strtok(NULL, TOKEN);
				cols++;
			}
		}
	}
	free(abs_filename);

	fclose(file);
}

//==============================================================================

void checkCUDAError(const char *msg) {
    cudaError_t err = cudaGetLastError();
    
    if(cudaSuccess != err) {
        fprintf(stderr, "CUDA error: %s: %s.\n", msg, cudaGetErrorString(err));
        exit(EXIT_FAILURE);
    }
}
