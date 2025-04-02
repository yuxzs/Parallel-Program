#include <iostream>
#include <pthread.h>
#include <cstdlib>
#include <random>
#include <vector>
#include <chrono>
#include <ctime>

using namespace std;

class FastRandom16 {
    public:
        // 建構子：若 seed 為 0，則用高解析度時鐘（取低 16 位）作種子
        FastRandom16(uint16_t seed = 0) {
            if (seed == 0) {
                seed = static_cast<uint16_t>(
                    std::chrono::high_resolution_clock::now().time_since_epoch().count() & 0xFFFF
                );
                if (seed == 0)
                    seed = 1;  // 避免 seed 為 0（xorshift 的不良情況）
            }
            state = seed;
        }
        
        // xorshift16 演算法，參數選擇 7, 9, 8（僅作示意，週期有限）
        inline uint16_t next() {
            state ^= (state << 7);
            state ^= (state >> 9);
            state ^= (state << 8);
            return state;
        }
        
        // 將 16 位元整數轉換成 [0, 1) 的 double 值
        inline double nextDouble() {
            return next() / 65536.0;
        }
        
    private:
        uint16_t state;
};
class FastRandom4 {
    public:
        // 如果 seed 為 0，則使用高解析度時鐘的低 4 位元作為種子，並確保非零
        FastRandom4(uint8_t seed = 0) {
            if (seed == 0) {
                uint8_t s = static_cast<uint8_t>(
                    std::chrono::high_resolution_clock::now().time_since_epoch().count() & 0xF
                );
                seed = (s == 0 ? 1 : s);
            }
            state = seed & 0xF;  // 只保留 4 位元
            if (state == 0)
                state = 1;  // 避免全 0 狀態
        }
        
        // 4 位元 LFSR：使用多項式 x^4 + x + 1
        inline uint8_t next() {
            // 取出最低位與最高位的 XOR 作為新位元
            uint8_t lsb = (state ^ (state >> 3)) & 1;
            // 右移 1 位，並將 lsb 放到最高位 (bit 3)
            state = (state >> 1) | (lsb << 3);
            return state;
        }
        
        // 將 4 位元的結果轉換為 [0, 1) 的 double 值
        inline double nextDouble() {
            return next() / 16.0;
        }
        
    private:
        uint8_t state;  // 僅使用 4 位元的狀態，必須非 0
};
class FastRandom {
    public:
        // 建構子若 seed 為 0 則使用高解析度時鐘作為初始值
        FastRandom(uint32_t seed = 0) {
            if (seed == 0) {
                seed = static_cast<uint32_t>(
                    std::chrono::high_resolution_clock::now().time_since_epoch().count()
                );
            }
            state = seed;
        }
        
        // xorshift32 演算法：更新 state 並回傳亂數
        inline uint32_t next() {
            state ^= state << 13;
            state ^= state >> 17;
            state ^= state << 5;
            return state;
        }
        
        // 直接將 32 位整數轉換成 [-1, 1] 的 double 數
        inline double nextDouble() {
            return (next() / 4294967296.0)*2 + 1;
        }
        
    private:
        uint32_t state;
};
int main(){

    random_device rd; // 随机设备产生种子
    default_random_engine gen(rd()); // 梅森旋转引擎
    uniform_real_distribution<> dist_real(-1.0, 1.0);

    FastRandom4 Fastgen;
    long long local_count = 0;

    auto start = chrono::high_resolution_clock::now();
    double x, y;
    for (long long i = 0; i < 100000000; i++) {
        // x = dist_real(gen);
        // y = dist_real(gen);
        x = Fastgen.nextDouble();
        y = Fastgen.nextDouble();
        if (x * x + y * y <= 1.0)
            local_count++;
    }
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = end - start;
    cout << "執行時間：" << elapsed.count() << " 秒;" << endl;
    
    return 0;
}
