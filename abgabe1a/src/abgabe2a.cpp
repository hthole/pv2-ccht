//============================================================================
// Name        : abgabe2a.cpp
// Author      :
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream.h>
#include <fstream.h>
#include <string.h>
#include <vector.h>

typedef std::vector<char> row;
typedef std::vector<row> matrix;

int get_entry_or_exit(row entry_line);
int find_next(matrix *maze);

enum direction {
		rechts, links, oben, unten
	};

int main() {
	std::string filename = "./bra10.txt";
	std::ifstream fin;
	matrix maze;

	// ----- read file -----
	fin.open(filename.c_str());

	if (fin) {
		std::vector<char> tmp_row;

		while (!fin.eof()) {
			char c_buf;
			fin.get(c_buf);

			switch (c_buf) {
			case '#':
			case ' ':
				tmp_row.push_back(c_buf);
				break;
			default:
				if(tmp_row.size() > 0) {
					maze.push_back(tmp_row);
					tmp_row.clear();
				}
				break;
			}
		}

		fin.close();
	} else {
		std::cout << "konnte datei " << filename << " nicht finden / oeffnen" << std::endl;
		return EXIT_FAILURE;
	}

	// ----- print matrix -----
	std::vector<char> v;
	for (unsigned int i = 0; i < maze.size(); i++) {
		v = maze.at(i);
		for (unsigned int j = 0; j < v.size(); j++) {
			std::cout << v.at(j);
		}
		std::cout << std::endl;
	}

	std::cout << "eingang: " << get_entry_or_exit(maze.at(0)) << std::endl;
	std::cout << "ausgang: " << get_entry_or_exit(maze.at(maze.size() - 1)) << std::endl;

	find_next(&maze);

	return EXIT_SUCCESS;
}

// ----- returns entry or exit field -----
int get_entry_or_exit(row line) {
	for (unsigned int i = 0; i < line.size(); i++) {
		if (line.at(i) == ' ') {
			return i;
		}
	}

	return -1;
}

int find_next(matrix *maze) {
	unsigned int j = 0;
	unsigned int k = 5;

	if (k < (maze->size() - 1) && maze->at(j).at(k + 1) == ' ') {
		std::cout << "rechts gehts weiter " << std::endl;
	}
	if (k > 0 && maze->at(j).at(k - 1) == ' ') {
		std::cout << "links gehts weiter " << std::endl;
	}
	if (j > 0 && maze->at(j - 0).at(k) == ' ') {
		std::cout << "oben gehts weiter " << std::endl;
	}
	if (j < (maze->size() - 1) && maze->at(j + 1).at(k) == ' ') {
		std::cout << "unten gehts weiter " << std::endl;
	}
}




void walk_maze (int x_pos, int y_pos, direction dir_to_walk, int last_step_count) {

	// ersten schritt machen
	if (dir_to_walk == rechts)
		x_pos++;
	if (dir_to_walk == links)
		x_pos--;
	if (dir_to_walk == oben)
		y_pos--;
	if (dir_to_walk == unten)
		y_pos++;

	my_maze[x_pos][y_pos] = last_step_count + 1;


	direction came_from = oben;
	// laufen bis zur nächsten Kreuzung (später besser: bis ziel gefunden)
	while (true) {
		/* prüfen ob Kreuzung vorliegt */
		int check_ways = 0;
		int right = 0, left = 0, up = 0, down = 0;
		if (x_pos - 1 != '#') {
			check_ways++;
			left = 1;
		}
		if (x_pos + 1 != '#') {
			check_ways++;
			right = 1;
		}
		if (y_pos - 1 != '#') {
			check_ways++;
			up = 1;
		}
		if (y_pos + 1 != '#') {
			check_ways++;
			down = 1;
		}
		if (check_ways > 2) {
			/*
			 * punkt zur queue hinzufügen
			 * mögliche richtungen mitgeben
			 * methode beenden
			 */
			if (left == 1)
				add_queue (x_pos, y_pos, links, last_step_count + 1);
			if (right == 1)
				add_queue (x_pos, y_pos, rechts, last_step_count + 1);
			if (up == 1)
				add_queue (x_pos, y_pos, oben, last_step_count + 1);
			if (down == 1)
				add_queue (x_pos, y_pos, unten, last_step_count + 1);

			// raus da!
			return 0;
		}

		// weiterlaufen, da keine Kreuzung
		// ACHTUNG: sicherstellen, dass nicht zurück gelaufen wird!!! TODO!!
		// unten
		if (came_from != unten && y_pos + 1  != '#' && (y_pos + 1 == ' ' || y_pos + 1 > last_step_count + 1)) {
			y_pos += 1;
			my_maze[x_pos][y_pos] = last_step_count++;
			came_from = unten;
		}
		// rechts (später optimieren je nachdem wo ziel liegt, also dann uU erst nach links)
		else if (came_from != rechts && x_pos + 1  != '#' && (x_pos + 1 == ' ' || x_pos + 1 > last_step_count + 1)) {
			x_pos += 1;
			my_maze[x_pos][y_pos] = last_step_count++;
			came_from = rechts;
		}
		// links (später optimieren je nachdem wo ziel liegt, also dann uU erst nach rechts)
		else if (came_from != links && x_pos - 1  != '#' && (x_pos - 1 == ' ' || x_pos - 1 > last_step_count + 1)) {
			x_pos -= 1;
			my_maze[x_pos][y_pos] = last_step_count++;
			came_from = links;
		}
		// oben
		else if (came_from != oben && y_pos - 1  != '#' && (y_pos - 1 == ' ' || y_pos - 1 > last_step_count + 1)) {
			y_pos -= 1;
			my_maze[x_pos][y_pos] = last_step_count++;
			came_from = oben;
		}

	}
}


