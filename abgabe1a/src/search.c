/*
 * search.c
 *
 *  Created on: 19.05.2009
 *      Author: hendrik, christian
 */

#define	PROCESSES	4

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "mpi.h"

#ifdef NDEBUG
#undef NDEBUG
#endif

#include <assert.h>

int debug_info(int* , int);
int get_random_number(int);
int find_value(int *, int, int);

int main(int argc, char **argv) {
	/* konvertiere die uebergabe parameter von char zu int	*/
	int array_size = atoi(argv[1]);
	int searchfor = atoi(argv[2]);

	/* deklariere array	*/
	int searcharray[array_size];
	int i = 0;

	/* Deklarationen fuer parallel kram */
	char msg[20];
	int me, total, count, tag = 99;
	MPI_Status status;
	MPI_Request request;

	if (argc != 3) {
		printf("usage: ./search <size> <search for>\n");
		return 1;
	}

	/* Initialisierung */
	if (MPI_Init(&argc, &argv) != MPI_SUCCESS) {
		printf("Konnte MPI nicht initialisieren, verlasse Programm...\n");
		exit(1);
	}

	/* Wer bin ich? */
	assert (MPI_Comm_rank(MPI_COMM_WORLD, &me) == MPI_SUCCESS);
	printf("PROCESS: %i\n", me);

	/* warten, bis alle sich gemeldet haben */
	MPI_Barrier(MPI_COMM_WORLD);

	/* Wieviele von uns gibt es? */
	assert (MPI_Comm_size(MPI_COMM_WORLD, &total) == MPI_SUCCESS);

	/* wie viel kriegt jeder prozess */
	int split_size = array_size / (total - 1);

	if (me == 0) {
		printf("we are %i buddies and i'm the leader\n", total);

		/* initialisieren des arrays */
		for (i = 0; i < array_size; i++) {
			searcharray[i] = 0;
		}

		/* bestimme zufallszahl	*/
		int randnumber = get_random_number(array_size);
		printf("random:%i\n", randnumber);

		/* weise searcharray[zufallszahl] der gesuchten zahl zu (debug)	*/
		searcharray[randnumber] = searchfor;

		/* debug ausgabe */
		// print_debug(&searcharray, array_size);

		for (i = 0; i < array_size; i++) {
			printf("|%i", searcharray[i]);

			if ((i + 1) % 20 == 0) {
				printf("|\n");
			}
		}

		int field_count = 0;

		printf("\nsplit size: %i\n", split_size);

		for (i = 0; i < total; i++) {
			int p_search_array[split_size];
			int j;

			// printf("\nprocess %i:\n", i);

			for (j = 0; j < split_size; j++) {
				p_search_array[j] = searcharray[field_count];
				field_count++;
				// printf("|%i", p_search_array[j]);
			}

			//printf("|");

			assert (MPI_Isend(p_search_array, split_size + 1, MPI_INT, 1, tag,
							MPI_COMM_WORLD, &request) == MPI_SUCCESS);


		}

	} else {
		int p_search_array[split_size];

		assert (MPI_Irecv(&p_search_array, split_size + 1, MPI_INT, 0, tag,
						MPI_COMM_WORLD, &request) == MPI_SUCCESS);

		//int result = find_value(&p_search_array, split_size, searchfor);

		int z = 0;
		for (z = 0; z < split_size; z++) {
			printf("%i.\t%i\t%i\n", me, z, p_search_array[z]);
			if (p_search_array[z] == 42) {
				printf("gefunden von %i\n\n", me);
				break;
			}
		}

		//printf("%i: %i\n", me, result);
	}

	/*


	 if (result != -1)
	 break;
	 */

	/* MPI Laufzeitsystem beenden */
	assert (MPI_Finalize() == MPI_SUCCESS);

	/* --------------- end parallel ----------------- */

	return 0;
}

/* debug ausgabe */
void print_debug(int *debug_info, int array_size) {
	printf("size of array: %i\n", array_size);

	int i;
	for (i = 0; i < array_size; i++) {
		printf("|%i", debug_info[i]);

		if ((i + 1) % 20 == 0) {
			printf("|\n");
		}
	}

	printf("\n");
}

/* erzeugt eine zufallszahl zwischen 0 und max_size - 1 */
int get_random_number(int max_size) {
	srand(time(0));

	return rand() % max_size;
}

int find_value(int *searcharray, int array_size, int search) {
	int i;
	int result = -1;

	for (i = 0; i < array_size; i++) {
		if (searcharray[i] == search) {
			result = i;
			break;
		}
	}

	return result;
}
