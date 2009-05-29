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
#define FILE_PATH	"./resources/"
#define FILE_MODE	"r"
#define TOKEN 		" "
#define BUF_SIZE	500000
#define PVERS       1

void read_file(int[], int, char[]);
void print_debug(int[], int);

int main(int argc, char **argv) {
	/* Deklarationen fuer parallel kram */
	int me, total, tag = 99;
	int sum = 0;
	MPI_Status status;
	MPI_Request request;

	/* Initialisierung */
	if (MPI_Init(&argc, &argv) != MPI_SUCCESS) {
		printf("Konnte MPI nicht initialisieren, verlasse Programm...\n");
		exit(1);
	}

	if (argc != 3) {
		printf("usage: ./calc <size> <filename>\n");
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
	int elements = atoi(argv[1]);
	int send_list[elements];
	int recv_list[elements];
//	int scan_list[elements];
//	int gather_list[elements];
//	int block_size = 1;

	// array aufteilen
	int chunk_size = elements / (total-1);

	if (me == ROOT) {
		read_file(send_list, elements, argv[2]);
		print_debug(send_list, elements);

		int k;
		for (k = 1; k < total; k++) {
			int *p_send = &send_list[elements];
			MPI_Isend(p_send, elements, MPI_INT, k, 99, MPI_COMM_WORLD, &request);
			MPI_Wait(&request, &status);
		}
	} else {
		int *p_recv = &recv_list[elements];
		MPI_Irecv(p_recv, elements, MPI_INT, ROOT, 99, MPI_COMM_WORLD, &request);
		MPI_Wait(&request, &status);

		int l;
		int start = chunk_size * (me - 1);
		for (l = start; l < chunk_size + start; l++) {
			// hier für den rechner aufaddieren
		}
	}

	printf("Hallo!\n");






	if(me == ROOT) {
		printf("macht gesamt: %i\n", sum);
	}

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
				// fuer jede spalte (zahl) in der zeile
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
