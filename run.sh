#!/bin/bash

echo "======================================"
echo "  SEQUENTIAL vs PARALLEL BENCHMARK"
echo "======================================"

# --------------------------------------
# CLEAN OLD OUTPUTS
# --------------------------------------
rm -f sequential/snap_*.txt sequential/stats.txt
rm -f parallel/snap_*.txt parallel/stats.txt

# --------------------------------------
# COMPILATION
# --------------------------------------
echo "Compiling sequential..."
g++ -O2 -std=c++17 sequential/main.cpp sequential/functions.cpp -o sequential/seq

echo "Compiling parallel..."
g++ -O2 -fopenmp -std=c++17 parallel/main.cpp parallel/functions.cpp -o parallel/par

# --------------------------------------
# RUN SEQUENTIAL
# --------------------------------------
echo
echo "Running SEQUENTIAL..."
SEQ_OUT=$(./sequential/seq)
echo "$SEQ_OUT"

# --------------------------------------
# RUN PARALLEL
# --------------------------------------
echo
echo "Running PARALLEL (OMP_NUM_THREADS=4)..."
export OMP_NUM_THREADS=4
PAR_OUT=$(./parallel/par)
echo "$PAR_OUT"

# --------------------------------------
# FUNCTION TO EXTRACT TIME
# --------------------------------------
get_time () {
    echo "$1" | grep "$2" | awk '{print $4}'
}

# --------------------------------------
# EXTRACT TIMES
# --------------------------------------
SEQ_UPD=$(get_time "$SEQ_OUT" "updateDisplay")
SEQ_STATS=$(get_time "$SEQ_OUT" "printOrderStats")
SEQ_TOTALAMT=$(get_time "$SEQ_OUT" "totalAmountTraded")
SEQ_TOTAL_EXEC=$(echo "$SEQ_OUT" | grep "Total execution time" | awk '{print $5}')

PAR_UPD=$(get_time "$PAR_OUT" "updateDisplay")
PAR_STATS=$(get_time "$PAR_OUT" "printOrderStats")
PAR_TOTALAMT=$(get_time "$PAR_OUT" "totalAmountTraded")
PAR_TOTAL_EXEC=$(echo "$PAR_OUT" | grep "Total execution time" | awk '{print $5}')

# --------------------------------------
# SPEEDUP RESULTS
# --------------------------------------
echo
echo "======================================"
echo "           SPEEDUP RESULTS"
echo "======================================"

printf "updateDisplay speedup        : %.3f x\n" \
       $(echo "$SEQ_UPD / $PAR_UPD" | bc -l)

printf "printOrderStats speedup      : %.3f x\n" \
       $(echo "$SEQ_STATS / $PAR_STATS" | bc -l)

printf "totalAmountTraded speedup    : %.3f x\n" \
       $(echo "$SEQ_TOTALAMT / $PAR_TOTALAMT" | bc -l)

printf "TOTAL PROGRAM SPEEDUP        : %.3f x\n" \
       $(echo "$SEQ_TOTAL_EXEC / $PAR_TOTAL_EXEC" | bc -l)

# --------------------------------------
# OUTPUT VALIDATION
# --------------------------------------
echo
echo "======================================"
echo "         OUTPUT VALIDATION"
echo "======================================"

# Count snapshot files
SEQ_SNAP_COUNT=$(ls sequential/snap_*.txt 2>/dev/null | wc -l)
PAR_SNAP_COUNT=$(ls parallel/snap_*.txt 2>/dev/null | wc -l)

echo "Sequential snapshots: $SEQ_SNAP_COUNT"
echo "Parallel snapshots:   $PAR_SNAP_COUNT"
echo

# Check if snapshot counts match
if [ "$SEQ_SNAP_COUNT" -ne "$PAR_SNAP_COUNT" ]; then
    echo "❌ SNAPSHOT COUNT MISMATCH"
    echo
else
    echo "✓ Snapshot counts match"
    echo
fi

# Validate each snapshot file
SNAP_MATCH_COUNT=0
SNAP_DIFF_COUNT=0

echo "Comparing snapshot files:"
for f in sequential/snap_*.txt; do
    if [ ! -f "$f" ]; then
        continue
    fi
    
    base=$(basename "$f")
    par_file="parallel/$base"
    
    if [ ! -f "$par_file" ]; then
        echo "  ✗ $base - Missing in parallel/"
        SNAP_DIFF_COUNT=$((SNAP_DIFF_COUNT + 1))
        continue
    fi
    
    if diff -q "$f" "$par_file" > /dev/null 2>&1; then
        echo "  ✓ $base - MATCH"
        SNAP_MATCH_COUNT=$((SNAP_MATCH_COUNT + 1))
    else
        echo "  ✗ $base - DIFFER"
        SNAP_DIFF_COUNT=$((SNAP_DIFF_COUNT + 1))
        
        # Show first few lines of difference
        echo "    --- First 5 lines of diff ---"
        diff "$f" "$par_file" | head -10 | sed 's/^/    /'
    fi
done

echo
echo "Snapshot results: $SNAP_MATCH_COUNT matches, $SNAP_DIFF_COUNT differences"
echo

# Validate stats.txt
echo "Comparing stats.txt:"
if [ -f "sequential/stats.txt" ] && [ -f "parallel/stats.txt" ]; then
    if diff -q sequential/stats.txt parallel/stats.txt > /dev/null 2>&1; then
        echo "  ✓ stats.txt - MATCH"
        STATS_MATCH=1
    else
        echo "  ✗ stats.txt - DIFFER"
        STATS_MATCH=0
        echo "    --- Diff output ---"
        diff sequential/stats.txt parallel/stats.txt | head -20 | sed 's/^/    /'
    fi
elif [ ! -f "sequential/stats.txt" ]; then
    echo "  ✗ stats.txt - Missing in sequential/"
    STATS_MATCH=0
elif [ ! -f "parallel/stats.txt" ]; then
    echo "  ✗ stats.txt - Missing in parallel/"
    STATS_MATCH=0
else
    STATS_MATCH=0
fi

echo
echo "======================================"
echo "           FINAL VERDICT"
echo "======================================"

if [ "$SNAP_DIFF_COUNT" -eq 0 ] && [ "$STATS_MATCH" -eq 1 ] && [ "$SEQ_SNAP_COUNT" -eq "$PAR_SNAP_COUNT" ]; then
    echo "✅ ALL OUTPUTS MATCH - PARALLEL IMPLEMENTATION CORRECT"
else
    echo "❌ OUTPUT MISMATCH DETECTED"
    echo "   Snapshot mismatches: $SNAP_DIFF_COUNT"
    if [ "$STATS_MATCH" -eq 0 ]; then
        echo "   Stats file: DIFFER"
    fi
fi

echo "======================================"