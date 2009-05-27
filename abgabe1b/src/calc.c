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

struct node
{
	struct node *next;
	int value;
};

int main(int argc, char **argv)
{
	if (argc != 3)
	{
		printf("usage: ./search <size> <filename>\n");
		return 1;
	}

	// konvertiere die uebergabe parameter von char zu int
	int count = atoi(argv[1]);
	int numbers[count];

	struct node *first_node;

	read_file(&numbers, count, &first_node, argv[2]);

	return 0;
}

void read_file(int *numbers, int count, struct node *first_node, char *filename)
{
	FILE *file;
	char line[BUF_SIZE];
	char *z;
	char *abs_filename = (char *) malloc(strlen(filename) + strlen(FILE_PATH) + 1);
	long cols = 0;

	strcpy(abs_filename, FILE_PATH);
	strcat(abs_filename, filename);

	struct node *new_node;
	first_node = NULL;

	file = fopen(abs_filename, "r");

	if (file == NULL)
		printf("Datei %s konnte nicht geoeffnet werden.\n", abs_filename);
	else
	{
		while (fgets(line, sizeof(line), file) != NULL)
		{
			// fuer jede zeile
			z = strtok(line, TOKEN);

			while (z != NULL && cols++ <= count)
			{
				// fuer jede spalte (zahl) in der zeile
				new_node = malloc(sizeof(*new_node));
				new_node->value = (int) z;
				new_node->next = NULL;

				append(&first_node, new_node);
				z = strtok(NULL, TOKEN);
			}
		}
		fclose(file);
		free(file);
	}

	print_debug(&first_node);

}

void print_debug(struct node **first_node)
{
	struct node *iter = *first_node;
	int i = 1;

	while (iter != NULL)
	{
		printf("%i.\t->\t%s\n", i++, iter->value);
		iter = iter->next;
	}
}

void append(struct node **first_node, struct node *new_node)
{
	struct node *iter = *first_node;

	if (iter != NULL) // wenn kinder vorhanden
	{
		while (iter->next != NULL) // suche letzten knoten
			iter = iter->next;

		iter->next = new_node; // haenge neuen knoten an
	}
	else
	{
		// wenn liste leer
		*first_node = new_node; // erste element = letztes element
	}
}

