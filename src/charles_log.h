#ifndef _CHARLES_LOG_H
#define _CHARLES_LOG_H
/*
 * File: charles_log.h
 * Description: Singleton class for Logging
 * Author: Charles, Liu.
 * Mail: charlesliu.cn.bj@gmail.com
 */
#include <pthread.h>
#include <atomic>
using std::atomic;

/*
 * This Singleton class is implemented under DCLP(Double Checked Locking Pattern).
 * You can refer to [DCLP](http://www.aristeia.com/Papers/DDJ_Jul_Aug_2004_revised.pdf) for
 * more detailed information about DCLP in Singleton.
 * Always do your best!
 */
class CharlesLog {
private:
    CharlesLog() {
    }
    ~CharlesLog() {
        CharlesLog *temp = instance.load(std::memory_order_relaxed);
        if (temp != NULL)
            delete temp;
    }
private:
    static pthread_mutex_t singleton_mutex;
    static atomic<CharlesLog *> instance;
public:
    static CharlesLog *getInstance();
};


#endif
