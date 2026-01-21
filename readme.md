Compilation
Sequential
cd sequential
g++ -O2 -std=c++17 main.cpp functions.cpp -o seq
./seq

Parallel
cd parallel
g++ -O2 -fopenmp -std=c++17 main.cpp functions.cpp -o par
OMP_NUM_THREADS=4 ./par