/*
 * search.c
 *
 *  Created on: 19.05.2009
 *      Author: hendrik, christian
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mpi.h"

#ifdef          NDEBUG
#undef          NDEBUG
#endif

#include <assert.h>

#define ROOT        0
#define FILE_PATH	"../resources/"
#define FILE_MODE	"r"
#define TOKEN 		" "
#define BUF_SIZE	500000
#define PVERS       1

void read_file(int[], int, char[]);
void print_debug(int[], int);

int main(int argc, char **argv) {
	/* Deklarationen fuer parallel kram */
	int me, total, tag = 99, i;
	MPI_Status status;
	MPI_Request request;

	/* Initialisierung */
	if (MPI_Init(&argc, &argv) != MPI_SUCCESS) {
		printf("Konnte MPI nicht initialisieren, verlasse Programm...\n");
		exit(1);
	}

	if (argc != 3) {
		printf("usage: ./search <size> <filename>\n");
		return 1;
	}

	/* Wer bin ich? */
	assert(MPI_Comm_rank(MPI_COMM_WORLD, &me) == MPI_SUCCESS);
	printf("Hier meldet sich Prozess: %i\n", me);

	/* warten, bis alle sich gemeldet haben */
	MPI_Barrier(MPI_COMM_WORLD);

	/* Wieviele von uns gibt es? */
	assert(MPI_Comm_size(MPI_COMM_WORLD, &total) == MPI_SUCCESS);

	// konvertiere die uebergabe parameter von char zu int
	int count = atoi(argv[1]);

	//int *send_list = (int *) malloc(count * sizeof(int));
	//int *recv_list = (int *) malloc(sizeof(int));

	int send_list[count];
	int recv_list[count];

	if (me == ROOT) {
		read_file(send_list, count, argv[2]);
	}

	// -------------

	int p_buf_size = count / total;
	// loesung fuer offset suchen
	// int offset = count % total;

	// array an prozesse verteilen
	assert(MPI_Scatter(send_list, p_buf_size, MPI_INT, recv_list, count,
					MPI_INT, ROOT, MPI_COMM_WORLD) == MPI_SUCCESS);
	assert(MPI_Barrier(MPI_COMM_WORLD) == MPI_SUCCESS);

	for (i = 0; i < p_buf_size; i++) {
		printf("%i.\t%i\t->\t%i\n", me, i, recv_list[i]);
	}

	assert(MPI_Barrier(MPI_COMM_WORLD) == MPI_SUCCESS);

	//int *recv = (int *) malloc(sizeof(int));
	int p_recv_list[p_buf_size];

	assert(MPI_Scan(recv_list, p_recv_list, p_buf_size, MPI_INT, MPI_SUM, MPI_COMM_WORLD) == MPI_SUCCESS);
	assert(MPI_Barrier(MPI_COMM_WORLD) == MPI_SUCCESS);

	for (i = 0; i < p_buf_size; i++) {
		printf("%i. (sum)\t%i\t->\t%i\n", me, i, p_recv_list[i]);
	}

	// -------------


	/* MPI Laufzeitsystem beenden */
	assert(MPI_Finalize() == MPI_SUCCESS);

	return 0;
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

	if (file == NULL)
		printf("Datei %s konnte nicht geoeffnet werden.\n", abs_filename);
	else {
		while (fgets(line, sizeof(line), file) != NULL) {
			// fuer jede zeile
			z = strtok(line, TOKEN);

			while (z != NULL && cols < count) {
				// fŸr jede spalte (zahl) in der zeile
				list[cols] = atoi(z);
				z = strtok(NULL, TOKEN);
				cols++;
			}
		}
	}

	fclose(file);
}

void print_debug(int list[], int count) {
	int i;

	for (i = 0; i < count; i++) {
		fprintf(stderr, "%i. %i\n", i, list[i]);
	}
}
