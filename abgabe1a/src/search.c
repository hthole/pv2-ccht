/*
 * search.c
 *
 *  Created on: 19.05.2009
 *      Author: hendrik, christian
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// funktionsprototypen
void    print_debug(int *debug_info, int array_size);
int     get_random_number(int max_size);

int main(int argc, char **argv)
{
	if (argc != 3)
	{
		printf("usage: ./search <size> <search for>\n");
		return 1;
	}

	// konvertiere die uebergabe parameter von char zu int
	int array_size = atoi(argv[1]);
	int searchfor = atoi(argv[2]);

	// deklariere array
	int searcharray[array_size];
	int i = 0;

	// initialisieren des arrays
	for (i = 0; i < array_size; i++)
	{
		searcharray[i] = 0;
	}

	// bestimme zufallszahl
	printf("random:%i\n", get_random_number(array_size));
	int randnumber = get_random_number(array_size);

	// weise searcharray[zufallszahl] der gesuchten zahl zu (debug)
	searcharray[randnumber] = searchfor;

	// debug ausgabe
	print_debug(&searcharray, array_size);

	return 0;
}

// debug ausgabe
void print_debug(int *debug_info, int array_size)
{
	printf("size of array: %i\n", array_size);

	int i;
	for (i = 0; i < array_size; i++)
	{
		printf("|%i|", debug_info[i]);

		if ((i + 1) % 20 == 0)
		{
			printf("\n");
		}
	}

	printf("\n");
}

// erzeugt eine zufallszahl zwischen 0 und max_size - 1
int get_random_number(int max_size)
{
	srand(time(0));

	return rand() % max_size;
}
