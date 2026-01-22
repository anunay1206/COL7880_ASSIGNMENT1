#!/bin/bash

echo "=============================="
echo " Sequential vs Parallel Run "
echo "=============================="
echo

# ------------------------------
# clean old outputs
# ------------------------------
rm -f sequential/snap_*.txt sequential/stats.txt
rm -f parallel/snap_*.txt parallel/stats.txt

# ------------------------------
# compile
# ------------------------------
echo "Compiling sequential..."
g++ -O2 -std=c++17 sequential/main.cpp sequential/functions.cpp -o sequential/seq

echo "Compiling parallel..."
g++ -O2 -fopenmp -std=c++17 parallel/main.cpp parallel/functions.cpp -o parallel/par

echo
echo "Running Sequential..."
echo "------------------------------"

SEQ_OUT=$(./sequential/seq)
echo "$SEQ_OUT"

SEQ_TIME=$(echo "$SEQ_OUT" | grep "Execution time" | awk '{print $4}')

echo
echo "Running Parallel (4 threads)..."
echo "------------------------------"

export OMP_NUM_THREADS=4
PAR_OUT=$(./parallel/par)
echo "$PAR_OUT"

PAR_TIME=$(echo "$PAR_OUT" | grep "Execution time" | awk '{print $4}')

echo
echo "=============================="
echo " Performance Summary"
echo "=============================="

echo "Sequential Time : $SEQ_TIME seconds"
echo "Parallel Time   : $PAR_TIME seconds"

# ------------------------------
# speedup calculation
# ------------------------------
if [[ -n "$SEQ_TIME" && -n "$PAR_TIME" ]]; then
    SPEEDUP=$(echo "$SEQ_TIME / $PAR_TIME" | bc -l)
    printf "Speedup          : %.3f x\n" "$SPEEDUP"
else
    echo "Speedup          : Could not calculate"
fi

echo "=============================="
