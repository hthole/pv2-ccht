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

typedef std::vector<int> row;
typedef std::vector<row> matrix;

struct q_elem {
	q_elem(int x, int y, int dir, int start) : x(x), y(y), dir(dir), start(start) {};
	int x;
	int y;
	int dir;
	int start;
	
};

std::vector<q_elem> queue;

enum direction {
	rechts, links, oben, unten
};

int get_entry_or_exit(row entry_line);
int find_next(matrix *maze);
void add_queue(int x_pos, int y_pos, int dir_to_walk,
				int last_step_count);
int walk_maze(int x_pos, int y_pos, int dir_to_walk, int last_step_count,
				matrix *my_maze);

bool ROOT = true;

int main() {
	std::string filename =
			"/Users/nmrd/Devel/workspaces/uni/2a/src/bra10.txt";
	std::ifstream fin;
	matrix maze;
	
	// ----- read file -----
	fin.open(filename.c_str());

	if (fin) {
		std::vector<int> tmp_row;

		while (!fin.eof()) {
			char c_buf;
			fin.get(c_buf);

			switch (c_buf) {
			case '#':
				tmp_row.push_back(-1);
				break;
			case ' ':
				tmp_row.push_back(0);
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
	std::vector<int> v;
	for (unsigned int i = 0; i < maze.size(); i++) {
		v = maze.at(i);
		for (unsigned int j = 0; j < v.size(); j++) {
			printf("%3d", v.at(j));
		}
		std::cout << std::endl;
	}

	std::cout << "eingang: " << get_entry_or_exit(maze.at(0)) << std::endl;
	std::cout << "ausgang: " << get_entry_or_exit(maze.at(maze.size() - 1))
			<< std::endl;

	walk_maze(get_entry_or_exit(maze.at(0)), 0, unten, 1, &maze);
	
	while(queue.size() > 0) {
		q_elem elem = queue.front();
		
		std::cout 	<< "walk_maze: " << elem.x << ", " << elem.y 
					<< ", " << elem.dir << ", " << elem.start << std::endl;
		
		queue.erase(queue.begin());
		walk_maze(elem.x, elem.y, elem.dir, elem.start, &maze);
	}

	// ----- print matrix -----
	for (unsigned int i = 0; i < maze.size(); i++) {
		v = maze.at(i);
		for (unsigned int j = 0; j < v.size(); j++) {
			printf("%3d", v.at(j));
		}
		std::cout << std::endl;
	}
	
	return EXIT_SUCCESS;
}

// ----- returns entry or exit field -----
int get_entry_or_exit(row line) {
	for (unsigned int i = 0; i < line.size(); i++) {
		if (line.at(i) == 0) {
			return i;
		}
	}

	return -1;
}

int walk_maze(int x_pos, int y_pos, int dir_to_walk, int last_step_count, matrix *my_maze) {

	if(ROOT) {
		my_maze->at(y_pos).at(x_pos) = last_step_count++;
		ROOT = false;
	}
	
	direction came_from	= oben;
	
	// ersten schritt machen
	if (dir_to_walk == rechts) {
		x_pos++;
		came_from = links;
	} else if (dir_to_walk == links) {
		x_pos--;
		came_from = rechts;
	} else if (dir_to_walk == oben) {
		y_pos--;
		came_from = unten;
	} else if (dir_to_walk == unten) {
		y_pos++;
		came_from = oben;
	}

	my_maze->at(y_pos).at(x_pos) = last_step_count++;
	
	
	
	bool made_step	= false;
	bool right		= false;
	bool left		= false;
	bool up			= false;
	bool down		= false;
	
	// laufen bis zur nächsten Kreuzung (später besser: bis ziel gefunden)
	while (true) {

		/* prüfen ob Kreuzung vorliegt */
		int check_ways = 0;
		made_step = false;
		
		right 	= false;
		left 	= false;
		up 		= false;
		down	= false;
				
		if (came_from != links 
				&& my_maze->at(y_pos).at(x_pos - 1) != -1
				&& (my_maze->at(y_pos).at(x_pos - 1) == 0
						|| my_maze->at(y_pos).at(x_pos - 1) > last_step_count
				)) {
			check_ways++;
			left = true;
		}
		if (came_from != rechts 
				&& my_maze->at(y_pos).at(x_pos + 1) != -1
				&& (my_maze->at(y_pos).at(x_pos + 1) == 0
						|| my_maze->at(y_pos).at(x_pos + 1) > last_step_count
				)) {
			check_ways++;
			right = true;
		}
		if (y_pos - 1 >= 0
				&& came_from != oben && my_maze->at(y_pos - 1).at(x_pos) != -1
				&& (my_maze->at(y_pos - 1).at(x_pos) == 0
						|| my_maze->at(y_pos - 1).at(x_pos) > last_step_count
				)) {
			check_ways++;
			up = true;
		}
		if (came_from != unten
				&& (y_pos + 1) < my_maze->size() 
				&& my_maze->at(y_pos + 1).at(x_pos) != -1
				&& (my_maze->at(y_pos + 1).at(x_pos) == 0
						|| my_maze->at(y_pos + 1).at(x_pos) > last_step_count
				)) {
			check_ways++;
			down = true;
		}
		
		if (check_ways >= 2) {
			/*
			 * punkt zur queue hinzufügen
			 * mögliche richtungen mitgeben
			 * methode beenden
			 */
			if (left)
				add_queue(x_pos, y_pos, links, last_step_count);
			if (right)
				add_queue(x_pos, y_pos, rechts, last_step_count);
			if (up)
				add_queue(x_pos, y_pos, oben, last_step_count);
			if (down)
				add_queue(x_pos, y_pos, unten, last_step_count);

			std::cout << "last step count: " << last_step_count << std::endl;
			// raus da!
			return 0;
			std::cout << "raus da" << std::endl;
		}

		// weiterlaufen, da keine Kreuzung
		// unten
		if (down) {
			y_pos += 1;
			my_maze->at(y_pos).at(x_pos) = last_step_count++;
			came_from = oben;
			made_step = true;
		}
		// rechts (später optimieren je nachdem wo ziel liegt, also dann uU erst nach links)
		else if (right) {
			x_pos += 1;
			my_maze->at(y_pos).at(x_pos) = last_step_count++;
			came_from = links;
			made_step = true;
		}
		// links (später optimieren je nachdem wo ziel liegt, also dann uU erst nach rechts)
		if (left) {
			x_pos -= 1;
			my_maze->at(y_pos).at(x_pos) = last_step_count++;
			came_from = rechts;
			made_step = true;
		}
		// oben
		if (up) {
			y_pos -= 1;
			my_maze->at(y_pos).at(x_pos) = last_step_count++;
			came_from = unten;
			made_step = true;
		}

		// kein schritt gemacht -> sackgasse
		if (!made_step)		return 0;

	}
}

void add_queue(int x_pos, int y_pos, int dir_to_walk, int last_step_count) {

	q_elem elem = q_elem(x_pos, y_pos, dir_to_walk, last_step_count);
	queue.push_back(elem);
}
