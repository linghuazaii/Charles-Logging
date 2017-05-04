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
#include <set>
#include <string>
#include <stdio.h>
#include <vector>
#include <string.h>
#include <errno.h>
#include <algorithm>
#include <queue>
#include "json.h"
using std::atomic;
using std::set;
using std::string;
using std::vector;
using std::queue;

#define MAX_LINE 1024

#define charles_err(fmt, ...) do {\
    fprintf(stderr, fmt, ##__VA_ARGS__);\
    fprintf(stderr, "\n");\
} while (0)

typedef enum {LOG_NONE, LOG_INFO, LOG_WARN, LOG_ERROR} LOG_LEVEL;

typedef struct {
    LOG_LEVEL log_level;
    set<string> log_tags;
    string log_dir;
    string process_name;
} log_conf_t;

/*
 * This Singleton class is implemented under DCLP(Double Checked Locking Pattern).
 * You can refer to [DCLP](http://www.aristeia.com/Papers/DDJ_Jul_Aug_2004_revised.pdf) for
 * more detailed information about DCLP in Singleton.
 * Always do your best!
 */
class CharlesLog {
private:
    CharlesLog() {
        log_conf.log_level = LOG_NONE;
        pthread_mutex_init(&queue_lock, NULL);
        pthread_cond_init(&queue_cond, NULL);
        current_file = "";
        file = NULL;
        log_conf.log_tags.insert(string("charles_logging"));
    }
    ~CharlesLog() {
        CharlesLog *temp = instance.load(std::memory_order_relaxed);
        if (temp != NULL)
            delete temp;

        pthread_mutex_destroy(&queue_lock);
        pthread_cond_destroy(&queue_cond);
    }
private:
    static pthread_mutex_t singleton_mutex;
    static atomic<CharlesLog *> instance;
private:
    log_conf_t log_conf;
    pthread_mutex_t queue_lock;
    pthread_cond_t queue_cond;
    queue<string> messages;
    FILE *file;
    string current_file;
    pthread_t work_thread;
public:
    static CharlesLog *getInstance();
private:
    string stripComments(vector<string> &lines);
    int stripSpaces(string &config); /* ' ', '\t', '\n' */
    int parseJson(string &config);
    bool checkTag(string tag);
    string getLogName();
    int updateFileHandle();
public:
    int loadConfig(const char *conf);
    int info(string tag, string msg, const char *file, int line);
    void run();
    void stop();
    void work(); /* you shouldn't call this function outside even though it is public, call run() */
};

/*
 * you should not use this class directly, use MACROs below.
 */
/*
 * you should use it in main()
 */
#define START_CHARLES_LOGGING(conf) do {\
    CharlesLog *charles_log = CharlesLog::getInstance();\
    charles_log->loadConfig(conf);\
    charles_log->run();\
} while (0)
/*
 * this function just wait for the logging thread to finish, you should
 * call it after all of you jobs is done.
 */
#define STOP_CHARLES_LOGGING() do {\
    CharlesLog *charles_log = CharlesLog::getInstance();\
    charles_log->stop();\
} while (0)
/*
 * logging functions
 */
#define LOG_INFO_T(tag, fmt, ...) do {\
    CharlesLog *charles_log = CharlesLog::getInstance();\
    char message[MAX_LINE];\
    snprintf(message, MAX_LINE, fmt, ##__VA_ARGS__);\
    charles_log->info(tag, message, __FILE__, __LINE__);\
} while(0)

#define LOG_INFO(fmt, ...) LOG_INFO_T("charles_logging", fmt, ##__VA_ARGS__)

#endif
