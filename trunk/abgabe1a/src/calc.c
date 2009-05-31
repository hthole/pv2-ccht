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

#define PARALLEL	1

void read_file(int[], int, char[]);
void print_debug(int[], int);

int main(int argc, char **argv) {
	/* Deklarationen fuer parallel kram */
	int me, total, tag = 99, i;
	MPI_Status status;
	MPI_Request request;
	double start, end;

	/* Initialisierung */
	if (MPI_Init(&argc, &argv) != MPI_SUCCESS) {
		printf("Konnte MPI nicht initialisieren, verlasse Programm...\n");
		exit(1);
	}

	if (argc != 3) {
		printf("usage: ./%s <size> <filename>\n", argv[0]);
		exit(1);
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
	int send_list[count];
	int recv_list[count];
	int scan_list[count];
	int gather_list[count];
	int block_size = 1;

	if (me == ROOT) {
		read_file(send_list, count, argv[2]);

#ifdef DEBUG
		print_debug(send_list, count);
#endif

	}

	start = MPI_Wtime();

	int j = 0;
#ifdef PARALLEL
	while (j < (count - total)) {
		int *p_send = &send_list[j];
		int *p_recv = &recv_list[0];
		int *p_scan = &scan_list[0];
		int *p_gather = &gather_list[j];

		assert(MPI_Scatter(p_send, block_size, MPI_INT, p_recv, block_size,
						MPI_INT, ROOT, MPI_COMM_WORLD) == MPI_SUCCESS);
		assert(MPI_Barrier(MPI_COMM_WORLD) == MPI_SUCCESS);

		assert(MPI_Scan(p_recv, p_scan, block_size, MPI_INT, MPI_SUM,
						MPI_COMM_WORLD) == MPI_SUCCESS);
		assert(MPI_Barrier(MPI_COMM_WORLD) == MPI_SUCCESS);

		assert(MPI_Gather(p_scan, block_size, MPI_INT, p_gather, block_size,
						MPI_INT, ROOT, MPI_COMM_WORLD) == MPI_SUCCESS);
		assert(MPI_Barrier(MPI_COMM_WORLD) == MPI_SUCCESS);

		for (i = j; i < (j + total); i++) {
			send_list[i] = gather_list[i];
		}

		j += total - 1;
	}

	if (me == ROOT) {
		for (i = (j + 1); i < count; i++) {
			send_list[i] = send_list[i - 1] + send_list[i];
		}
#else
	if (me == ROOT) {
		for (i = 1; i < count; i++) {
			send_list[i] = send_list[i - 1] + send_list[i];
		}
#endif
#ifdef DEBUG
		for (i = 0; i < count; i++) {
			printf("%i\t->\t%i\n", i, send_list[i]);
		}
#endif
	}

	j = 0;
	float sum = 0;
	while (j < (count - total)) {
		int *p_send = &send_list[j];
		int *p_recv = &recv_list[0];
		int *p_reduce = &scan_list[0];

		assert(MPI_Scatter(p_send, block_size, MPI_INT, p_recv, block_size,
				MPI_INT, ROOT, MPI_COMM_WORLD) == MPI_SUCCESS);

		assert(MPI_Reduce(p_recv, p_reduce, block_size, MPI_INT, MPI_SUM, ROOT,
				MPI_COMM_WORLD) == MPI_SUCCESS);

		sum += (float) scan_list[0];
		j += total;
	}

	if (me == ROOT) {
		for (i = j; i < count; i++) {
			sum += (float) send_list[i];
		}

		end = MPI_Wtime();

		printf("macht gesamt: %f\n", sum);
		printf("\nZeit gemessen: %f Sekunden\n", (end - start));

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
				// f�r jede spalte (zahl) in der zeile
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
