export LD_LIBRARY_PATH=/opt/intel/cce/10.0.023/lib:$LD_LIBRARY_PATH
module remove mpich2
module add mpich2/1.0.5p4/intel/10.0.023
mpd&

kompilieren mit:
mpicxx -o abgabe2a abgabe2a.cpp
ausführen zZ noch mit:
mpiexec -n 8 ./abgabe2a per10.txt