/*
 * search.c
 *
 *  Created on: 19.05.2009
 *      Author: hendrik, christian
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILE_PATH	"../resources/"
#define FILE_MODE	"r"
#define TOKEN 		" "
#define BUF_SIZE	500000

void read_file(int[], int, char[]);
void print_debug(int[], int);

int main(int argc, char **argv) {
	FILE *file;
	char line[BUF_SIZE];
	char *z;
	long cols = 0;

	if (argc != 3) {
		printf("usage: ./search <size> <filename>\n");
		return 1;
	}

	// konvertiere die uebergabe parameter von char zu int
	int count = atoi(argv[1]);
	int list[count];

	read_file(list, count, argv[2]);
	print_debug(list, count);

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

			while (z != NULL && cols <= count) {
				// fŸr jede spalte (zahl) in der zeile
				list[cols] = atoi(z);
				z = strtok(NULL, TOKEN);
				cols++;
			}
		}
		fclose(file);
	}
}

void print_debug(int list[], int count) {
	int i;

	for (i = 0; i < count; i++) {
		printf("%i. %i\n", i, list[i]);
	}
}
