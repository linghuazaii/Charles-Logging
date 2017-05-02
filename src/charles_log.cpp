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
        }
        pthread_mutex_unlock(&singleton_mutex);
    }

    return temp;
}

int CharlesLog::loadConfig(const char *conf) {
    vector<string> lines;
    FILE *file = fopen(conf, "r");
    if (file == NULL) {
        charles_err("open file %s failed. (%s)", conf, strerror(errno));
        return -1;
    }

    char *line = NULL;
    size_t max_line = 0;
    while (-1 != getline(&line, &max_line, file)) {
        lines.push_back(string(line));
    }
    free(line);

    string config = stripComments(lines);
    stripSpaces(config);
    cout<<config<<endl;

    fclose(file);
    return 0;
}

string CharlesLog::stripComments(vector<string> &lines) {
    string config = "";
    for (int i = 0; i < lines.size(); ++i) {
        size_t pos = lines[i].find('#');
        if (pos != string::npos)
            lines[i].erase(pos);
        config += lines[i];
    }

    return config;
}

int CharlesLog::stripSpaces(string &config) {
    config.erase(std::remove(config.begin(), config.end(), ' '), config.end());
    config.erase(std::remove(config.begin(), config.end(), '\t'), config.end());
    config.erase(std::remove(config.begin(), config.end(), '\n'), config.end());

    return 0;
}
