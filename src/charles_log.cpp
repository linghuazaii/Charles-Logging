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
    if (0 != parseJson(config))
        return -1;
    run();

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

int CharlesLog::parseJson(string &config) {
    json_tokener_error json_err;
    json_object *conf_json = json_tokener_parse_verbose(config.c_str(), &json_err);
    if (conf_json == NULL) {
        charles_err("Json parse error for %s (%s)", config.c_str(), json_tokener_error_desc(json_err));
        return -1;
    }

    int log_level = json_object_get_int(json_object_object_get(conf_json, "log_level"));
    log_conf.log_level = LOG_LEVEL(log_level);

    json_object *tags = json_object_object_get(conf_json, "log_tags");
    for (size_t i = 0; i < json_object_array_length(tags); ++i) {
        string tag = json_object_get_string(json_object_array_get_idx(tags, i));
        std::transform(tag.begin(), tag.end(), tag.begin(), ::tolower);
        log_conf.log_tags.insert(tag);
    }
    if (log_conf.log_tags.end() != std::find(log_conf.log_tags.begin(), log_conf.log_tags.end(), "all"))
        log_conf.log_tags.clear();
    json_object_put(conf_json);

    return 0;
}

bool CharlesLog::checkTag(string tag) {
    if (log_conf.log_tags.empty() ||
            log_conf.log_tags.end() != std::find(log_conf.log_tags.begin(), log_conf.log_tags.end(), tag))
        return true;

    return false;
}

void *run_callback(void *arg) {
    CharlesLog *handle = (CharlesLog *)arg;
    //handle->work();
    return NULL;
}

void CharlesLog::run() {
    pthread_t thread_id;
    pthread_attr_t thread_attr;
    pthread_attr_init(&thread_attr);
    pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);
    pthread_attr_setinheritsched(&thread_attr, PTHREAD_EXPLICIT_SCHED);
    int min_priority = sched_get_priority_min(SCHED_OTHER);
    cout<<"min: "<<min_priority<<endl;
    int max_priority = sched_get_priority_max(SCHED_OTHER);
    cout<<"max: "<<max_priority<<endl;
    pthread_attr_destroy(&thread_attr);
}
