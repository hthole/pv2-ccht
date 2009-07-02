//============================================================================
// Name        : abgabe2a.cpp
// Author      : Hendrik Thole & Christian Claus
// Version     :
// Copyright   : Steal this code!
// Description : Parallel Maze solving algorithm in C++ & MPI, Ansi-style
// Compile     : Use the mpich2 intel Compiler
//               mpicxx -o abgabe2a abgabe2a.cpp
//============================================================================

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <math.h>

#ifdef	SEEK_SET
#undef	SEEK_SET
#endif

#ifdef	SEEK_END
#undef	SEEK_END
#endif

#ifdef	SEEK_CUR
#undef	SEEK_CUR
#endif

#include "mpi.h"

#define ROOT	0

// ----------------------------------------------------------------------

typedef std::vector<int> row;
typedef std::vector<row> matrix;

struct q_elem {
	q_elem(int x, int y, int dir, int start) : x(x), y(y), dir(dir), value(start) {};
	int x;
	int y;
	int dir;
	int value;
};

enum direction {
	rechts, links, oben, unten
};

bool FIRST_RUN = true;
std::vector<q_elem> queue;

// ----------------------------------------------------------------------

int		get_entry_or_exit(row entry_line);
int		find_next(matrix *maze);
int		walk_maze(int x_pos, int y_pos, int dir_to_walk, int last_step_count, matrix *my_maze, int exit_pos);
void	add_queue(int x_pos, int y_pos, int dir_to_walk, int last_step_count);

// ----------------------------------------------------------------------

int main(int argc, char *argv[]) {
	
	/*
	 * Variablen Deklaration
	 */
	std::ifstream 	fin;
	matrix 			maze;
	row 		 	list_maze;
	double 			start_time, 
					end_time;
	int 			entry_pos, 
					exit_pos, 
					num_procs, 
					maze_size,
					maze_length,
					me;

	/*
	 *  MPI-Initialisierung
	 */ 
	MPI::Init(argc, argv);
	
	/*
	 *  Wer bin ich?
	 */
	me = MPI::COMM_WORLD.Get_rank();
	num_procs = MPI::COMM_WORLD.Get_size();

	// ----------------------------------------------------------------------
	
	/*
	 * Master Prozess liest Datei ein
	 */
	if (me == ROOT) {
		maze_size = 0;
		fin.open(argv[1]);

		if (fin) {
			bool worked_on_line = false;
			while (!fin.eof()) {
				char c_buf;
				fin.get(c_buf);

				switch (c_buf) {
				case '#':
					list_maze.push_back(-1);
					worked_on_line = true;
					
					break;
				case ' ':
					list_maze.push_back(0);
					worked_on_line = true;
					
					break;
				case '\n':
					if (worked_on_line) {
						maze_size++;
						worked_on_line = false;
						
					}
					
					break;
				}
			}

			fin.close();
			
		} else {
			std::cout << "konnte datei " << argv[1]
					  << " nicht finden / oeffnen" << std::endl;
			
			return EXIT_FAILURE;
			
		}
		
		/*
		 * Master bereitet Arrays vor und veschickt diese
		 */
		maze_length = maze_size * maze_size;
		int maze_as_array[maze_length];
		
		for (int i = 0; i < maze_length; i++) {
			maze_as_array[i] = list_maze.at(i);
		}
		for (int i = 1; i < num_procs; i++) {
			MPI::COMM_WORLD.Send(&maze_length, 1, MPI::INT, i, 1);
			MPI::COMM_WORLD.Send(maze_as_array, maze_length, MPI::INT, i, 2);
		}
		
		row tmp_row;
		int col_counter = 1;

		for (int i = 0; i < maze_length; i++) {
			tmp_row.push_back(maze_as_array[i]);
			if (col_counter % maze_size == 0) {
				maze.push_back(tmp_row);
				tmp_row.clear();
			}
			col_counter++;
		}
		
	} else {
		/*
		 * Slaves Empfangen Arraygroesse und anschliessend Array
		 */
		MPI::COMM_WORLD.Recv(&maze_length, 1, MPI::INT, 0, 1);
		int maze_as_array[maze_length];

		MPI::COMM_WORLD.Recv(maze_as_array, maze_length, MPI::INT, 0, 2);
		
		row tmp_row;
		int col_counter = 1;
		maze_size		= (int)sqrt((double) maze_length);
		
		for (int i = 0; i < maze_length; i++) {
			tmp_row.push_back(maze_as_array[i]);
			
			if (col_counter % maze_size == 0) {
				maze.push_back(tmp_row);
				tmp_row.clear();
			}
			col_counter++;
		}
	}

	// ----------------------------------------------------------------------
	
	/*
	 * Auf alle Prozesse warten
	 */
	MPI::COMM_WORLD.Barrier();
	
	/*
	 * Jetzt fŠngt der 'richtige' paralele Teil an: Zeitnahme starten
	 */
	if (me == ROOT) {
		start_time = MPI_Wtime();
	}
	
	/*
	 * Ein- und Ausgang berechnen
	 */
	entry_pos = get_entry_or_exit(maze.at(0));
	exit_pos  = get_entry_or_exit(maze.at(maze.size() - 1));
	
	if (me == ROOT) {
		/*
		 * Master sucht ersten Knoten zum verteilen
		 */
		walk_maze(entry_pos, 0, unten, 1, &maze, exit_pos);

		/*
		 * Queue ueber Prozesse bauen
		 */
		std::vector<int> processes;
		for(int i = 1; i < num_procs; i++) {
			processes.push_back(i);
		}
		
		/*
		 * Starte eine Endlosschleife
		 */
		while (true) {
			int recv_node[12];
			
			/*
			 * Receive im Hintergrund starten um Ergebnisse der Slaves abzufangen
			 */
			MPI::Request req_i = MPI::COMM_WORLD.Irecv(&recv_node, 12, MPI::INT, MPI::ANY_SOURCE, 42);
			MPI::Status status_i;

			/*
			 * Solange die Queue nicht leer ist und noch Prozesse verfŸgbar sind
			 */
			while (queue.size() > 0 && processes.size() > 0) {
				/*
				 * Erste Elemente der Queue holen und danach loeschen
				 */
				q_elem elem = queue.front();
				int node[] = { elem.x, elem.y, elem.dir, elem.value };
				int proc_num = processes.front();
				
				queue.erase(queue.begin());
				processes.erase(processes.begin());
				
				/*
				 * Ersten Job der Job-Queue an ersten Prozess der Prozess-Queue schicken
				 */
				MPI::COMM_WORLD.Isend(node, 4, MPI::INT, proc_num, 23);
			}
			
			/*
			 * Wenn alle Prozesse zurueckgekehrt sind und Job-Queue leer ist
			 */
			if (processes.size() == (num_procs - 1) && queue.size() == 0) {
				int kill_signal[] = { 0, 0, 0, 0 };

				/*
				 * Master sendet an alle Slaves das kill Signal
				 */
				for (int i = 1; i < num_procs; i++) {
					MPI::COMM_WORLD.Send(kill_signal, 4, MPI::INT, i, 23);
				}

				/*
				 * MPI::Request Objekt wird abgebrochen, so dass nicht weiter gewartet wird
				 */
				req_i.Cancel();
				
				break;
			}

			/*
			 * Warten, bis das Irecv etwas empfangen hat
			 */
			req_i.Wait(status_i);
			
			int z      = 0;
			bool go_on = false;
			
			while (z < 12) {
				if (recv_node[z] == 0) {
					go_on = true;
					break;
				}
				
				int x 	  = recv_node[z++];
				int y 	  = recv_node[z++];
				int dir	  = recv_node[z++];
				int value = recv_node[z++];
				
				/* 
				 * Switch ueber die Richtungen, des Empfangspuffers. Hier erfolgt eine Pruefung,
				 * ob der Master die Ergebnisse in die Job-Queue haengen soll - oder ob bereits
				 * 'bessere' Werte in der Matrix stehen
				 */
				switch (dir) {
				case unten:
					/* 
					 * Wenn Element unter dem Element des Empfangspuffers frei - oder groesser
					 * als das im Empfangspuffer - ist wird in die Job-Queue eingereiht  
					 * (rest genauso...)
					 */
					if (maze[y + 1][x] == 0 || maze[y + 1][x] > value + 1) {
						add_queue(x, y, dir, value);
					}
					break;
				case links:
					if (maze[y][x - 1] == 0 || maze[y][x - 1] > value + 1) {
						add_queue(x, y, dir, value);
					}
					break;
				case rechts:
					if (maze[y][x + 1] == 0 || maze[y][x + 1] > value + 1) {
						add_queue(x, y, dir, value);
					}
					break;
				case oben:
					if (maze[y - 1][x] == 0 || maze[y - 1][x] > value + 1) {
						add_queue(x, y, dir, value);
					}
					break;
				}
				
				/*
				 * Wenn Element aus Empfangspuffer geeignet ist, wird es in die Matrix geschrieben
				 */
				if (maze[y][x] == 0 || maze[y][x] > value) {
					maze[y][x] = value;
				}

			}
			
			if (go_on || z >= 12) {
				int proc_num = status_i.Get_source();
				processes.push_back(proc_num);
				
				continue;
			}
				
		}
		
	} else {
		/*
		 * Jeder Slave bekommt ein Job Array
		 */ 
		int node[4];
		
		/*
		 * Starten einer Endlosschleife
		 */
		while(true) {
			/*
			 * Das Job-Array wird durch den Master gefuellt
			 */
			MPI::COMM_WORLD.Recv(node, 4, MPI::INT, 0, 23);
			
			/*
			 * Wenn Master kill Signal geschickt hat, Schleife verlassen
			 */
			if(node[0] == 0) {
				break;
			}
			
			/*
			 * Das Labyrinth wird mit den Daten des Empfangspuffers durchlaufen
			 */
			walk_maze(node[0], node[1], node[2], node[3], &maze, exit_pos);
			
			/* 
			 * Jeder Slave hat eine eigene Queue mit Knotenpunkten, die durch
			 * die walk_maze Prozedur gefuellt werden. Diese werden jetzt nacheinander
			 * an den Master geschickt und aus der Queue entfernt
			 */
			int slave_send_node[12];
			for (int i = 0; i < 12; i++) {
				slave_send_node[i] = 0;
			}
			
			int z = 0;
			while (queue.size() > 0) {
				q_elem slave_node = queue.front();
				
				slave_send_node[z++] = slave_node.x;
				slave_send_node[z++] = slave_node.y;
				slave_send_node[z++] = slave_node.dir;
				slave_send_node[z++] = slave_node.value;
				
				queue.erase(queue.begin());
			}
			
			MPI::COMM_WORLD.Send(slave_send_node, 12, MPI::INT, 0, 42);
		}
	}

	/*
	 * Zum Abschluss noch die Zeitnahme und Weglaenge
	 */
	if (me == ROOT) {
		end_time = MPI_Wtime();

		std::cout << "Weglaenge: " << maze.at(maze.size() - 1).at(exit_pos) << std::endl;
		std::cout << "Zeit gemessen: " << (end_time - start_time) << " Sekunden" << std::endl << std::endl;
		
		
		// ----- print matrix -----
		/*
		for (unsigned int i = 0; i < maze.size(); i++) {
			row v = maze.at(i);
			for (unsigned int j = 0; j < v.size(); j++) {
				printf("%3d", v.at(j));
			}
			std::cout << std::endl;
		}
		*/
	}
	
	/*
	 * Au§erdem wird die MPI Laufzeitumgebung beendet
	 */
	MPI::Finalize(); 
	
	return EXIT_SUCCESS;
}

/*
 * Diese Funktion gibt zurueck, wo sich in einer Zeile ein freies Feld befindet.
 * Somit wird Ein- und Ausgang berechnet
 */
int get_entry_or_exit(row line) {
	for (unsigned int i = 0; i < line.size(); i++) {
		if (line.at(i) == 0) {
			return i;
		}
	}

	return -1;
}

/*
 * Labyrinth ablaufen
 */
int walk_maze(int x_pos, int y_pos, int dir_to_walk, int last_step_count, matrix *my_maze, int exit_pos) {

	if(FIRST_RUN && MPI::COMM_WORLD.Get_rank() == 0) {
		my_maze->at(y_pos).at(x_pos) = last_step_count;
		FIRST_RUN = false;
	}

	direction came_from	= oben;

	/*
	 * ersten schritt machen
	 */
	switch (dir_to_walk) {
	case rechts:
		x_pos++;
		came_from = links;
		break;
	case links:
		x_pos--;
		came_from = rechts;
		break;
	case oben:
		y_pos--;
		came_from = unten;
		break;
	case unten:
		y_pos++;
		came_from = oben;
		break;
	}
	
	my_maze->at(y_pos).at(x_pos) = ++last_step_count;
	
	//add_queue(x_pos, y_pos, came_from, last_step_count);
	
	bool made_step	= false;
	bool right		= false;
	bool left		= false;
	bool up			= false;
	bool down		= false;

	int goal_steps = -1;

	/*
	 * laufen bis zur naechsten Kreuzung (spaeter besser: bis ziel gefunden)
	 */
	while (true) {

		// pruefen, ob abgebrochen werden kann, da sonst mehr schritte als im ziel benoetigt wuerden
		//goal_steps = my_maze->at(my_maze->size() - 1).at(exit_pos);
		//if (goal_steps > 0 && last_step_count > goal_steps) {
		//	return 0;
		//}

		/*
		 * pruefen ob Kreuzung vorliegt
		 */
		int check_ways = 0;
		made_step = false;

		right 	= false;
		left 	= false;
		up 		= false;
		down	= false;

		if (came_from != links
				&& (my_maze->at(y_pos).at(x_pos - 1) == 0
					|| my_maze->at(y_pos).at(x_pos - 1) > last_step_count)) {
			check_ways++;
			left = true;
		}
		if (came_from != rechts
				&& (my_maze->at(y_pos).at(x_pos + 1) == 0
					|| my_maze->at(y_pos).at(x_pos + 1) > last_step_count)) {
			check_ways++;
			right = true;
		}
		if (y_pos - 1 >= 0
				&& came_from != oben 
				&& (my_maze->at(y_pos - 1).at(x_pos) == 0
					|| my_maze->at(y_pos - 1).at(x_pos) > last_step_count)) {
			check_ways++;
			up = true;
		}
		if (came_from != unten
				&& (y_pos + 1) < my_maze->size()
				&& (my_maze->at(y_pos + 1).at(x_pos) == 0
						|| my_maze->at(y_pos + 1).at(x_pos) > last_step_count)) {
			check_ways++;
			down = true;
		}

		if (check_ways >= 2) {
			/*
			 * punkt zur queue hinzufuegen
			 * moegliche richtungen mitgeben
			 * methode beenden
			 */
			if (down)
				add_queue(x_pos, y_pos, unten, last_step_count);
			if (left)
				add_queue(x_pos, y_pos, links, last_step_count);
			if (right)
				add_queue(x_pos, y_pos, rechts, last_step_count);
			if (up)
				add_queue(x_pos, y_pos, oben, last_step_count);

			return 0;
		}
		
		/*
		 * weiterlaufen, da keine Kreuzung
		 */
		
		/*
		 * unten
		 */
		if (down) {
			y_pos += 1;
			my_maze->at(y_pos).at(x_pos) = ++last_step_count;
			came_from = oben;
			made_step = true;
		}
		/*
		 * rechts (spaeter optimieren je nachdem wo ziel liegt, also dann uU erst nach links)
		 */
		else if (right) {
			x_pos += 1;
			my_maze->at(y_pos).at(x_pos) = ++last_step_count;
			came_from = links;
			made_step = true;
		}
		/*
		 * links (spaeter optimieren je nachdem wo ziel liegt, also dann uU erst nach rechts)
		 */
		else if (left) {
			x_pos -= 1;
			my_maze->at(y_pos).at(x_pos) = ++last_step_count;
			came_from = rechts;
			made_step = true;
		}
		/*
		 * oben
		 */
		else if (up) {
			y_pos -= 1;
			my_maze->at(y_pos).at(x_pos) = ++last_step_count;
			came_from = unten;
			made_step = true;
		}

		if(y_pos == (my_maze->size() - 1))
			add_queue(x_pos, y_pos, oben, last_step_count);
		
		/*
		 * kein schritt gemacht -> sackgasse
		 */
		if (!made_step)		return 0;

	}
}

/*
 * Hier wird ein neues Element an die Job-Queue angehaengt
 */
void add_queue(int x_pos, int y_pos, int dir_to_walk, int last_step_count) {

	q_elem elem = q_elem(x_pos, y_pos, dir_to_walk, last_step_count);
	queue.push_back(elem);
}
