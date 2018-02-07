#include <iostream>
#include <vector>
#include <math.h>
#include <climits>
#include <bitset>
#include <cstring>
#include <assert.h>
#include <sys/time.h>
#include <thread>
#include <mutex>
#include "vEB.h"

// ___________________
// |    |             |
// | K  |     SIZE    |
// |____|_____________|
// |    |             |
// | 2  | 4           |
// | 4  | 16          |
// | 8  | 256         |
// | 16 | 65536       |
// | 32 | 429467296-1 |
// |____|_____________|

#define K       32
#define SIZE    4000000

int NUM_OF_THREADS = 4;

using namespace std;

// Таймер
long long mtime()
{
    struct timeval t;
    gettimeofday(&t, NULL);
    long long mt = (long long)t.tv_sec * 1000 + t.tv_usec / 1000;
    return mt;
}

// Код, выполняемый каждым потоком
void ThrFunc(vEB_stm <K> &Tree, int num) {
    unsigned long portion = SIZE / NUM_OF_THREADS;
    unsigned long seed = 345634*num;
    for(unsigned long i = num*portion; i<(num*portion+portion); ++i) {
        Tree.remove(i);
    }
}


int main() {
    vEB_stm<K> Tree;

    for (uint i = 0; i < SIZE; ++i) {
        Tree.insert(i);
    }

    // Запуск таймера
    long long time = mtime();


    std::thread thr[NUM_OF_THREADS];
    for(int i = 0; i<NUM_OF_THREADS; ++i) {
        thr[i] = std::thread(ThrFunc, std::ref(Tree), i);
    }

    for(int i = 0; i<NUM_OF_THREADS; ++i){
        thr[i].join();
    }

    time = mtime() - time;
    printf("%llu ms\n", time);

    return 0;
}