#include <iostream>
#include <omp.h>
using namespace std;

int main() {
    // This directive tells OpenMP to run the following block in parallel
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        int total_threads = omp_get_num_threads();

        #pragma omp critical
        {
            std::cout << "Hello from thread " << thread_id 
                      << " out of " << total_threads << " threads!" << endl;
        }
    }
    return 0;
}