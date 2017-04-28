#include "charles_log.h"
#include <time.h>
#include <iostream>
using namespace std;

atomic<CharlesLog *> CharlesLog::instance;
pthread_mutex_t CharlesLog::singleton_mutex = PTHREAD_MUTEX_INITIALIZER;

CharlesLog *CharlesLog::getInstance() {
    CharlesLog *temp = instance.load(std::memory_order_relaxed);
    std::atomic_thread_fence(std::memory_order_acquire);
    if (temp == NULL) {
        pthread_mutex_lock(&singleton_mutex);
        temp = instance.load(std::memory_order_relaxed);
        if (temp == NULL) {
            temp = new CharlesLog;
            std::atomic_thread_fence(std::memory_order_release);
            instance.store(temp, std::memory_order_relaxed);
            cout << "stored >> " << time(0)<<endl;
        }
        pthread_mutex_unlock(&singleton_mutex);
    }

    return temp;
}
