#include "functions.h"
#include <vector>
#include <cstdint>
#include <iostream>
#include <chrono>

// --------------------------------------------------
// TEST CASES
// --------------------------------------------------

void test_small(std::vector<uint64_t>& orderBook) {
    for (int i = 1; i <= 5; i++) {
        orderBook.push_back(
            (uint64_t)i |
            ((uint64_t)0 << 32) |
            ((uint64_t)5 << 33) |
            ((uint64_t)(40 + i) << 41)
        );

        orderBook.push_back(
            (uint64_t)i |
            ((uint64_t)1 << 32) |
            ((uint64_t)3 << 33) |
            ((uint64_t)(60 + i) << 41)
        );
    }
}

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

void test_contention(std::vector<uint64_t>& orderBook) {
    for (int i = 0; i < 2'000'000; i++) {
        orderBook.push_back(
            (uint64_t)1 |
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

    // ===============================
    // CHOOSE ONE TEST CASE
    // ===============================

    // test_small(orderBook);
    // test_medium(orderBook);
    test_large(orderBook);
    // test_contention(orderBook);

    int32_t freq = 100000;

    // ===============================
    // TOTAL PROGRAM TIMER
    // ===============================
    auto total_start = std::chrono::high_resolution_clock::now();

    // -------------------------------
    // updateDisplay timing
    // -------------------------------
    auto t1_start = std::chrono::high_resolution_clock::now();
    updateDisplay(orderBook, freq);
    auto t1_end = std::chrono::high_resolution_clock::now();

    // -------------------------------
    // printOrderStats timing
    // -------------------------------
    auto t2_start = std::chrono::high_resolution_clock::now();
    printOrderStats(orderBook);
    auto t2_end = std::chrono::high_resolution_clock::now();

    // -------------------------------
    // totalAmountTraded timing
    // -------------------------------
    auto t3_start = std::chrono::high_resolution_clock::now();
    int64_t total = totalAmountTraded(orderBook);
    auto t3_end = std::chrono::high_resolution_clock::now();

    auto total_end = std::chrono::high_resolution_clock::now();

    // ===============================
    // OUTPUT
    // ===============================

    std::cout << "Total Amount Traded = " << total << "\n\n";

    std::cout << "updateDisplay time      = "
              << std::chrono::duration<double>(t1_end - t1_start).count()
              << " s\n";

    std::cout << "printOrderStats time    = "
              << std::chrono::duration<double>(t2_end - t2_start).count()
              << " s\n";

    std::cout << "totalAmountTraded time  = "
              << std::chrono::duration<double>(t3_end - t3_start).count()
              << " s\n\n";

    std::cout << "Total execution time    = "
              << std::chrono::duration<double>(total_end - total_start).count()
              << " s\n";

    return 0;
}
