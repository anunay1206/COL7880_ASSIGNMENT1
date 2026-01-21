#include "functions.h"
#include <vector>
#include <cstdint>
#include <iostream>
#include <chrono>

// --------------------------------------------------
// TEST CASE 1: SMALL / CORRECTNESS
// --------------------------------------------------
void test_small(std::vector<uint64_t>& orderBook) {
    for (int i = 1; i <= 5; i++) {
        // buy
        orderBook.push_back(
            (uint64_t)i |
            ((uint64_t)0 << 32) |
            ((uint64_t)5 << 33) |
            ((uint64_t)(40 + i) << 41)
        );

        // sell
        orderBook.push_back(
            (uint64_t)i |
            ((uint64_t)1 << 32) |
            ((uint64_t)3 << 33) |
            ((uint64_t)(60 + i) << 41)
        );
    }
}

// --------------------------------------------------
// TEST CASE 2: MEDIUM WORKLOAD
// --------------------------------------------------
void test_medium(std::vector<uint64_t>& orderBook) {
    for (int i = 0; i < 100000; i++) {
        orderBook.push_back(
            (uint64_t)(i % 100) |
            ((uint64_t)(i % 2) << 32) |
            ((uint64_t)(i % 20 + 1) << 33) |
            ((uint64_t)(i % 100 + 20) << 41)
        );
    }
}

// --------------------------------------------------
// TEST CASE 3: LARGE PERFORMANCE TEST
// --------------------------------------------------
void test_large(std::vector<uint64_t>& orderBook) {
    int N = 3'000'000;

    for (int i = 0; i < N; i++) {
        orderBook.push_back(
            (uint64_t)(i % 1000) |
            ((uint64_t)(i % 2) << 32) |
            ((uint64_t)(i % 50 + 1) << 33) |
            ((uint64_t)(i % 200 + 20) << 41)
        );
    }
}

// --------------------------------------------------
// TEST CASE 4: CONTENTION STRESS TEST
// --------------------------------------------------
void test_contention(std::vector<uint64_t>& orderBook) {
    for (int i = 0; i < 2'000'000; i++) {
        orderBook.push_back(
            (uint64_t)1 |                      // same stock
            ((uint64_t)(i % 2) << 32) |
            ((uint64_t)10 << 33) |
            ((uint64_t)(50 + (i % 20)) << 41)
        );
    }
}

// --------------------------------------------------
// MAIN
// --------------------------------------------------
int main() {

    std::vector<uint64_t> orderBook;

    // ==================================================
    // CHOOSE EXACTLY ONE TEST CASE
    // ==================================================

    // test_small(orderBook);        // correctness
     test_medium(orderBook);    // moderate load
    // test_large(orderBook);     // performance
    // test_contention(orderBook);// race-condition test

    // ==================================================

    int32_t freq = 100000;   // keep large for timing

    auto start = std::chrono::high_resolution_clock::now();

    updateDisplay(orderBook, freq);
    printOrderStats(orderBook);
    int64_t total = totalAmountTraded(orderBook);

    auto end = std::chrono::high_resolution_clock::now();

    std::cout << "Total Amount Traded = " << total << "\n";
    std::cout << "Execution time = "
              << std::chrono::duration<double>(end - start).count()
              << " seconds\n";
     // -------- FUNCTION-WISE TIMING --------
    auto t1 = std::chrono::high_resolution_clock::now();
    updateDisplay(orderBook, freq);
    auto t2 = std::chrono::high_resolution_clock::now();

    printOrderStats(orderBook);
    auto t3 = std::chrono::high_resolution_clock::now();

    totalAmountTraded(orderBook);
    auto t4 = std::chrono::high_resolution_clock::now();

    std::cout << "updateDisplay time      = "
              << std::chrono::duration<double>(t2 - t1).count() << " s\n";

    std::cout << "printOrderStats time    = "
              << std::chrono::duration<double>(t3 - t2).count() << " s\n";

    std::cout << "totalAmountTraded time  = "
              << std::chrono::duration<double>(t4 - t3).count() << " s\n";
    return 0;
}
