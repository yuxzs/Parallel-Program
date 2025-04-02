#include <iostream>
#include <pthread.h>
#include <cstdlib>
#include <random>
#include <vector>
#include <chrono>
#include <ctime>
#include <cstdint>

using namespace std;

long long total_in_circle = 0;
pthread_mutex_t mutex;

struct ThreadData {
    long long tosses;
};

class FastRandom16 {
    public:
        FastRandom16(uint16_t seed = 0) {
            if (seed == 0) {
                seed = static_cast<uint16_t>(
                    std::chrono::high_resolution_clock::now().time_since_epoch().count() & 0xFFFF
                );
                if (seed == 0)
                    seed = 1;
            }
            state = seed;
        }
        
        inline uint16_t next() {
            state ^= (state << 7);
            state ^= (state >> 9);
            state ^= (state << 8);
            return state;
        }
        
        inline double nextDouble() {
            return (next() / 65536.0)*2 -1;
        }
        
    private:
        uint16_t state;
};
void* monte_carlo(void* arg) {
    
    ThreadData* data = (ThreadData*) arg;
    long long tosses = data->tosses;
    long long local_count = 0;

    FastRandom16 Fastgen;
    double x, y;

    for (long long i = 0; i < tosses; i++) {
        x = Fastgen.nextDouble();
        y = Fastgen.nextDouble();
        if (x * x + y * y <= 1.0)
            local_count++;
    }

    pthread_mutex_lock(&mutex);
    total_in_circle += local_count;
    pthread_mutex_unlock(&mutex);
    return nullptr;
}

int main(int argc, char* argv[]){
    if (argc != 3) {
        cerr << "用法: " << argv[0] << " <threads> <number_of_tosses>" << endl;
        return 1;
    }
    
    int num_threads = atoi(argv[1]);
    long long total_tosses = atoll(argv[2]);

    pthread_mutex_init(&mutex, nullptr);

    vector<pthread_t> threads(num_threads);
    vector<ThreadData> threadData(num_threads);

    long long tosses_per_thread = total_tosses / num_threads;
    long long remainder = total_tosses % num_threads;

    for (int i = 0; i < num_threads; i++){
        threadData[i].tosses = tosses_per_thread + (i < remainder ? 1 : 0);
        if (pthread_create(&threads[i], nullptr, monte_carlo, &threadData[i]) != 0) {
            cerr << "建立 thread " << i << " 失敗" << endl;
            return 1;
        }
    }

    for (int i = 0; i < num_threads; i++){
        pthread_join(threads[i], nullptr);
    }
    pthread_mutex_destroy(&mutex);

    double pi_estimate = 4.0 * total_in_circle / total_tosses;
    // cout << pi_estimate << endl;
    printf("%lf\n", pi_estimate);
    
    return 0;
}
