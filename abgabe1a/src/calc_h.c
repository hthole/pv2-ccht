/*
 *
 *      Author: C. Claus, H. Thole
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

void read_file(int[], int, char[]);
void print_debug(int[], int);

int main(int argc, char **argv) {
	/* spaeter gebrauchte Variablen deklarieren */
	int me, total, tag = 99, l, k;
	int sum = 0;
	MPI_Status status;
	MPI_Request request;
	double start_time, end_time; // fuer Zeitnahme

	/* MPI-Initialisierung */
	if (MPI_Init(&argc, &argv) != MPI_SUCCESS) {
		printf("Konnte MPI nicht initialisieren, verlasse Programm...\n");
		exit(1);
	}

	/* Parameteranzahl pruefen */
	if (argc != 3) {
		printf("usage: mpiexec -n <process#> %s <size> <filename>\n", argv[0]);
		exit(1);
	}

	/* Wer bin ich? */
	assert(MPI_Comm_rank(MPI_COMM_WORLD, &me) == MPI_SUCCESS);
	printf("Hier meldet sich Prozess: %i\n", me);

	/* warten, bis alle sich gemeldet haben */
	assert(MPI_Barrier(MPI_COMM_WORLD) == MPI_SUCCESS);

	/* Wieviele von uns gibt es? */
	assert(MPI_Comm_size(MPI_COMM_WORLD, &total) == MPI_SUCCESS);

	/* konvertiere die Uebergabeparameter von char zu int */
	int elements = atoi(argv[1]);

	/*  Sende- und Empfangsarray einrichten */
	int send_list[elements];
	int recv_list[elements];

	/* Groesse des Abschnitts bestimmen und evtl Offset berechnen, falls nicht
	 * glatt geteilt werden kann
     */
	int chunk_size = elements / total;
	int offset = elements % total;

	/* Master liest Datei ein, startet die Zeitnahme und sendet alle Zahlen an
	 * jeden Slave
	 */
	if (me == ROOT) {
		read_file(send_list, elements, argv[2]);
		//print_debug(send_list, elements);

		start_time = MPI_Wtime();

		for (k = 0; k < total; k++) {
			assert(MPI_Isend(send_list, elements, MPI_INT, k, 99, MPI_COMM_WORLD,
					&request) == MPI_SUCCESS);
		}
	}

	/* Slaves empfangen die Arrays */
	assert(MPI_Irecv(recv_list, elements, MPI_INT, ROOT, 99, MPI_COMM_WORLD, &request) == MPI_SUCCESS);
	assert(MPI_Wait(&request, &status) == MPI_SUCCESS);

	/* Slave berechnet den Start- und Endpunkt seiner Berechnung */
	int start = chunk_size * me;
	int end = start + chunk_size;

	/* Der 'letzte' Slave muss den Offset beruecksichtigen */
	if (me == (total - 1)) {
		end += offset;
	}

	/* Slave berechnet Startwert */
	int p_sum = 0;
	for (l = 0; l <= start; l++) {
		p_sum += recv_list[l];
	}

	recv_list[start] = p_sum;

	/* Slave berechnet einzelne Summen seine Teilstuecks */
	for (l = start; l < end - 1; l++) {
		recv_list[l + 1] += recv_list[l];
	}



	if (me != ROOT) {
		/* Slaves schicken Ergebnisarrays zurueck */
		MPI_Send(recv_list, elements, MPI_INT, ROOT, 99, MPI_COMM_WORLD);
	} else {
		/* Ergenisarrays werden angenommen und die jeweils berechneten Teilstuecke
		 * in einem Array zusammengefasst */
		for(k = 1; k < total; k++) {
			int tmp[elements];

			/* Master empfaengt von jedem Slave das Ergebnisarray */
			assert(MPI_Recv(tmp, elements, MPI_INT, k, 99, MPI_COMM_WORLD, &status) == MPI_SUCCESS);
			start = chunk_size * k;
			end = start + chunk_size;

			if (k == (total - 1)) {
				end += offset;
			}

			/* Master kopiert das Ergebnisarray eines jeden Slaves zu seinem eigenen */
			for(l = start; l < end; l++) {
				recv_list[l] = tmp[l];
			}
		}
	}


	/* Master schickt Teilstuecke des Ergebnisarrays zurueck an Slaves */
	if (me == ROOT) {
		int this_chunk = chunk_size;
		for (l = 0; l < total; l++) {
			start = chunk_size * l;

			if (l == (total - 1)) {
				this_chunk = chunk_size + offset;
			} else {
				this_chunk = chunk_size;
			}

			int *p_send = &recv_list[start];
			assert(MPI_Isend(p_send, this_chunk, MPI_INT, l, 4711, MPI_COMM_WORLD, &request) == MPI_SUCCESS);
		}
	}

	/* Slaves empfangen ihre Teilstuecke */
	if (me == (total - 1)) {
		chunk_size += offset;
	}
	int tmp_chunk[chunk_size];
	assert(MPI_Irecv(tmp_chunk, chunk_size, MPI_INT, ROOT, 4711, MPI_COMM_WORLD, &request) == MPI_SUCCESS);
	assert(MPI_Wait(&request, &status) == MPI_SUCCESS);


	/* Slaves addieren Felder auf */
	float my_sum = 0.0;
	for (l = 0; l < chunk_size; l++) {
		my_sum += tmp_chunk[l];
	}


	/* Slaves schicken Summe zurueck an den Master */
	if (me != ROOT) {
		assert(MPI_Send(&my_sum, 1, MPI_FLOAT, ROOT, 815, MPI_COMM_WORLD) == MPI_SUCCESS);
	}

	/* Master empfaengt Ergenisse, summiert diese auf und zeigt Gesamtsumme an */
	if (me == ROOT) {
		float the_sum = 0.0;
		for (l = 1; l < total; l++) {
			assert(MPI_Recv(&the_sum, 1, MPI_FLOAT, l, 815, MPI_COMM_WORLD, &status) == MPI_SUCCESS);
			my_sum += the_sum;
		}

		end_time = MPI_Wtime();

		printf("macht gesamt: %f\n", my_sum);
		printf("\nZeit gemessen: %f Sekunden\n", (end_time - start_time));
	}

	/* MPI-Laufzeitsystem beenden */
	assert(MPI_Finalize() == MPI_SUCCESS);

	return 0;
}



void read_file(int list[], int count, char *filename) {
	FILE *file;
	char line[BUF_SIZE];
	char *z;
	char *abs_filename = (char *) malloc(strlen(filename) + strlen(FILE_PATH) + 1);
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

void print_debug(int list[], int count) {
	int i;

	for (i = 0; i < count; i++) {
		fprintf(stderr, "%i. %i\n", i, list[i]);
	}
}
