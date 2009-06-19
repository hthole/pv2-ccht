//============================================================================
// Name        : abgabe2a.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

typedef std::vector<char> row;
typedef std::vector<row> matrix;

int get_entry_or_exit(row entry_line);
int find_next(matrix *maze);

int main() {
	std::string filename = "/Users/nmrd/Devel/workspaces/uni/abgabe2a/src/bra10.txt";
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
