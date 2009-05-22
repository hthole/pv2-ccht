/*
 * search.c
 *
 *  Created on: 19.05.2009
 *      Author: hendrik, christian
 */

#include <stdio.h>
#include <stdlib.h>

#define FILE_NAME	"../resources/100K"
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
		printf("usage: ./search <size> <search for>\n");
		return 1;
	}

	// konvertiere die uebergabe parameter von char zu int
	int count = atoi(argv[1]);
	int numbers[count];
	struct node *first_node;

	read_file(&numbers, count, &first_node);

	char **searchfor = argv[2];

	return 0;

}


void read_file(int *numbers, int count, struct node *first_node)
{
	FILE *file;
	char line[BUF_SIZE];
	char *z;
	long cols = 0;

	struct node *new_node;

	first_node = NULL;

	file = fopen(FILE_NAME, "r");
	if (file == NULL)
		printf("Datei konnte nicht geöffnet werden.\n");
	else
	{
		while (fgets(line, sizeof(line), file) != NULL)
		{ // für jede Zeile
			z = strtok(line, TOKEN);
			while (z != NULL && cols <= count)
			{ // für jede Spalte in der Zeile
				new_node = malloc(sizeof(*new_node));
				new_node->value = z;
				new_node->next = NULL;

				append(&first_node, new_node);

				z = strtok(NULL, TOKEN);
				cols++;
			}
		}
		fclose(file);
	}

	print_debug(&first_node);

}

void print_debug(struct node **first_node)
{
	struct node *iter = *first_node;
	int i = 1;

	if(iter != NULL)
	{
		while(iter->next != NULL)
		{
			printf("%i.\t->\t%s\n", i++, iter->value);
			iter = iter->next;
		}
	}
	else
		printf("list is empty\n");
}

void append(struct node **first_node, struct node *new_node)
{
	struct node *iter = *first_node;

	if(iter != NULL) 						// wenn kinder vorhanden
	{
		while(iter->next != NULL)			// suche letzten knoten
			iter = iter->next;

		iter->next = new_node;				// haenge neuen knoten an
	}
	else									// wenn liste leer
		*first_node = new_node;				// erste element = letztes element
}

