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

enum direction {
	rechts, links, oben, unten
};

int get_entry_or_exit(row entry_line);
int find_next(matrix *maze);
void add_queue(int x_pos, int y_pos, direction dir_to_walk,
				int last_step_count);
int walk_maze(int x_pos, int y_pos, direction dir_to_walk, int last_step_count,
		matrix *my_maze);

int main() {
	std::string filename =
			"/Users/nmrd/Devel/workspaces/uni/abgabe2a/src/simple10.txt";
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
				if (tmp_row.size() > 0) {
					maze.push_back(tmp_row);
					tmp_row.clear();
				}
				break;
			}
		}

		fin.close();
	} else {
		std::cout << "konnte datei " << filename << " nicht finden / oeffnen"
				<< std::endl;
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
	std::cout << "ausgang: " << get_entry_or_exit(maze.at(maze.size() - 1))
			<< std::endl;

	find_next(&maze);
	walk_maze(get_entry_or_exit(maze.at(0)), 0, unten, 0, &maze);

	// ----- print matrix -----
	for (unsigned int i = 0; i < maze.size(); i++) {
		v = maze.at(i);
		for (unsigned int j = 0; j < v.size(); j++) {
			std::cout << v.at(j);
		}
		std::cout << std::endl;
	}

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
	unsigned int row = 0;
	unsigned int col = 5;

	if (col < (maze->size() - 1) && maze->at(row).at(col + 1) == ' ') {
		std::cout << "rechts gehts weiter " << std::endl;
	}
	if (col > 0 && maze->at(row).at(col - 1) == ' ') {
		std::cout << "links gehts weiter " << std::endl;
	}
	if (row > 0 && maze->at(row - 0).at(col) == ' ') {
		std::cout << "oben gehts weiter " << std::endl;
	}
	if (row < (maze->size() - 1) && maze->at(row + 1).at(col) == ' ') {
		std::cout << "unten gehts weiter " << std::endl;
	}
}

int walk_maze(int x_pos, int y_pos, direction dir_to_walk, int last_step_count,
		matrix *my_maze) {

	my_maze->at(y_pos).at(x_pos) = '0';

	// ersten schritt machen
	if (dir_to_walk == rechts)
		x_pos++;
	if (dir_to_walk == links)
		x_pos--;
	if (dir_to_walk == oben)
		y_pos--;
	if (dir_to_walk == unten)
		y_pos++;

	//my_maze[x_pos][y_pos] = last_step_count + 1;
	my_maze->at(y_pos).at(x_pos) = '1';

	direction came_from = oben;
	std::vector<char> v;
	// laufen bis zur nächsten Kreuzung (später besser: bis ziel gefunden)
	while (true) {
		// ----- print matrix -----
		for (unsigned int i = 0; i < my_maze->size(); i++) {
			v = my_maze->at(i);
			for (unsigned int j = 0; j < v.size(); j++) {
				std::cout << v.at(j);
			}
			std::cout << std::endl;
		}


		/* prüfen ob Kreuzung vorliegt */
		int check_ways = 0;
		int right = 0, left = 0, up = 0, down = 0;
		if (x_pos - 1 >= 0 && my_maze->at(y_pos).at(x_pos - 1) != '#') {
			check_ways++;
			left = 1;
		}
		if (x_pos < my_maze->size() && my_maze->at(y_pos).at(x_pos + 1) != '#') {
			check_ways++;
			right = 1;
		}
		if (y_pos - 1 >= 0 && my_maze->at(y_pos - 1).at(x_pos) != '#') {
			check_ways++;
			up = 1;
		}
		if (y_pos < my_maze->size() && my_maze->at(y_pos + 1).at(x_pos) != '#') {
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
				add_queue(x_pos, y_pos, links, last_step_count + 1);
			if (right == 1)
				add_queue(x_pos, y_pos, rechts, last_step_count + 1);
			if (up == 1)
				add_queue(x_pos, y_pos, oben, last_step_count + 1);
			if (down == 1)
				add_queue(x_pos, y_pos, unten, last_step_count + 1);

			// raus da!
			//return 0;
			std::cout << "raus da" << std::endl;
		}

		std::cout << up << " "
					<< down << " "
					<< left << " "
					<< right << " " << std::endl;

		bool made_step = false;
		// weiterlaufen, da keine Kreuzung
		// unten
		if (down != 0 && y_pos >= my_maze->size() && came_from != oben && my_maze->at(y_pos + 1).at(x_pos) != '#'
				&& (my_maze->at(y_pos + 1).at(x_pos) == ' '
				|| (int)my_maze->at(y_pos + 1).at(x_pos) > last_step_count + 1)) {
			y_pos += 1;
			my_maze->at(y_pos).at(x_pos) = '1';
			last_step_count++;
			came_from = unten;
			made_step = true;
			std::cout << "unten" << std::endl;
		}
		// rechts (später optimieren je nachdem wo ziel liegt, also dann uU erst nach links)
		else if (right != 0 && came_from != links && my_maze->at(y_pos).at(x_pos + 1) != '#'
				&& (my_maze->at(y_pos).at(x_pos + 1) == ' '
				|| (int)my_maze->at(y_pos).at(x_pos + 1) > last_step_count + 1)) {
			x_pos += 1;
			my_maze->at(y_pos).at(x_pos) = '2';
			last_step_count++;
			came_from = rechts;
			made_step = true;
			std::cout << "rechts" << std::endl;
		}
		// links (später optimieren je nachdem wo ziel liegt, also dann uU erst nach rechts)
		if (left != 0 && came_from != rechts && my_maze->at(y_pos).at(x_pos - 1) != '#'
				&& (my_maze->at(y_pos).at(x_pos - 1) == ' '
				|| (int)my_maze->at(y_pos).at(x_pos - 1) > last_step_count + 1)) {
			x_pos -= 1;
			my_maze->at(y_pos).at(x_pos) = '3';
			last_step_count++;
			came_from = links;
			made_step = true;
			std::cout << "links" << std::endl;
		}
		// oben
		if (up != 0 && y_pos - 1 > my_maze->size()
						&& came_from != unten && my_maze->at(y_pos - 1).at(x_pos) != '#'
						&& (my_maze->at(y_pos - 1).at(x_pos) == ' '
						|| (int)my_maze->at(y_pos - 1).at(x_pos) > last_step_count + 1)) {
			y_pos -= 1;
			my_maze->at(y_pos).at(x_pos) = '4';
			last_step_count++;
			came_from = oben;
			made_step = true;
			std::cout << "oben" << std::endl;
		}

		// kein schritt gemacht -> sackgasse
		if (!made_step)
			return 0;

	}
}

void add_queue(int x_pos, int y_pos, direction dir_to_walk, int last_step_count) {

}
