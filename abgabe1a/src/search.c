/*
 * search.c
 *
 *  Created on: 19.05.2009
 *      Author: hendrik, christian
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mpi.h"

#define 	ROOT	0

#ifdef 		NDEBUG
#undef 		NDEBUG
#endif

#include <assert.h>

int main(int argc, char **argv[]) {

	/* Deklarationen fuer parallel kram */
	int me, total, count, tag = 99;
	MPI_Status status;
	MPI_Request request;
	int buf_size;
	int p_buf_size;
	int search_for;
	int was_found;
	int stop = 1337;
	int i;

	/* Initialisierung */
	if (MPI_Init(&argc, &argv) != MPI_SUCCESS) {
		printf("Konnte MPI nicht initialisieren, verlasse Programm...\n");
		exit(1);
	}

	if (argc != 3) {
		printf("usage: ./search <size> <search for>\n");
		return 1;
	}

	/* konvertiere die uebergabe parameter von char zu int	*/
	buf_size = atoi(argv[1]);
	int sendbuf[buf_size];
	search_for = atoi(argv[2]);

	/* Wer bin ich? */
	assert(MPI_Comm_rank(MPI_COMM_WORLD, &me) == MPI_SUCCESS);
	printf("Hier meldet sich Prozess: %i\n", me);

	/* warten, bis alle sich gemeldet haben */
	MPI_Barrier(MPI_COMM_WORLD);

	/* Wieviele von uns gibt es? */
	assert(MPI_Comm_size(MPI_COMM_WORLD, &total) == MPI_SUCCESS);

	p_buf_size = buf_size / total;
	int recvbuf[p_buf_size];

	if(me == ROOT) {
		for(i = 0; i < buf_size; i++) {
			sendbuf[i] = i;
		}
	}

	MPI_Scatter(sendbuf, p_buf_size, MPI_INT, recvbuf, p_buf_size, MPI_INT, ROOT, MPI_COMM_WORLD);
	// MPI_Barrier(MPI_COMM_WORLD);
	MPI_Irecv(&stop, 1, MPI_INT, MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, &request);

	MPI_Test(&request, &was_found, &status);


	printf("%i buf size %i\n", me, p_buf_size);
	for(i = 0; !was_found && i < p_buf_size; i++) {
		// printf("%i.\t%i\t->\t%i\t%i\n", me, i, recvbuf[i], was_found);

		if(recvbuf[i] == search_for) {
			printf("gefunden an position: %i, von prozess: %i\n", (i + me * p_buf_size), me);

			int j;
			for(j = 0; j < total; j++) {
				stop = j;
				MPI_Send(&stop, 1, MPI_INT, j, tag, MPI_COMM_WORLD);
			}
		}

		MPI_Test(&request, &was_found, &status);
	}

	printf("prozess %i bei position %i gestoppt.\n", me, (i + me * p_buf_size));

	/* MPI Laufzeitsystem beenden */
	assert (MPI_Finalize() == MPI_SUCCESS);

	return 0;
}

/* erzeugt eine zufallszahl zwischen 0 und max_size - 1 */
int get_random_number(int max_size) {
	srand(time(0));

	return rand() % max_size;
}
