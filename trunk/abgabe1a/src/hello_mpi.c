/*
 * hello_mpi.c
 *
 *  Created on: 25.05.2009
 *      Author: nmrd
 */

/* Ein simples MPI Hello World Programm. Prozess No. 0 sendet eine
 * Nachricht an Prozess 1 und meldet sich anschliessend, Prozess 1
 * empfaengt die Nachricht und gibt sie aus, alle anderen Prozesse
 * melden sich nur */

#include <stdio.h>
#include <string.h>
#include <stdlib.h> /* for exit () */
#include "mpi.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>

int maain(int argc, char **argv)
{
	char msg[20];
	int me, total, count, tag = 99;
	MPI_Status status;


	/* Initialisierung */
	if (MPI_Init (&argc, &argv) != MPI_SUCCESS) {
		printf ("Konnte MPI nicht initialisieren, verlasse Programm...\n");
		exit (1);
	}

	/* Wer bin ich? */
	assert (MPI_Comm_rank(MPI_COMM_WORLD, &me) == MPI_SUCCESS);

	/* Wieviele von uns gibt es? */
	assert (MPI_Comm_size(MPI_COMM_WORLD, &total) == MPI_SUCCESS);

	/* Prozess 0 */
	if (me == 0) {
		strcpy(msg, "Hallo");
		assert (MPI_Send(msg, strlen(msg) + 1, MPI_CHAR, 1, tag,
                         MPI_COMM_WORLD) == MPI_SUCCESS) ;
		printf("0: Gesendete Nachricht: %s \n", msg);

	/* Prozess 1 */
	} else if (me == 1) {
		assert (MPI_Recv(msg, 20, MPI_CHAR, 0, tag,
                MPI_COMM_WORLD, &status) == MPI_SUCCESS);

		/* Anzahl empfangener Zeichen ermitteln */
		assert (MPI_Get_count (&status, MPI_CHAR, &count) == MPI_SUCCESS);

		printf("1: Empfangene Nachricht: %s mit einer Laenge von %d\n",
               msg, count);

	/* alle Anderen */
	} else
		printf("%d: Huhu, ich bin Prozess Nr. %d \n", me, me);

	/* MPI Laufzeitsystem beenden */
	assert (MPI_Finalize() == MPI_SUCCESS);

	return 0;
}
