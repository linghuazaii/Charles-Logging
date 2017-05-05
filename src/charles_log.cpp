#include "charles_log.h"
#include <time.h>
#include <iostream>
#include <sched.h>
#include "json.h"
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

    const char *log_dir = json_object_get_string(json_object_object_get(conf_json, "log_dir"));
    if (log_dir == NULL)
        log_conf.log_dir = "../log";
    else
        log_conf.log_dir = log_dir;
    if (log_conf.log_dir[log_conf.log_dir.length() - 1] == '/')
        log_conf.log_dir.pop_back();
    string cmd = "mkdir -p " + log_conf.log_dir;
    system(cmd.c_str());

    const char *process_name = json_object_get_string(json_object_object_get(conf_json, "process_name"));
    if (process_name != NULL) 
        log_conf.process_name = process_name;
    else {
        string name = getenv("_");
        log_conf.process_name = name.substr(name.rfind('/') + 1);
    }

    json_object_put(conf_json);
    return 0;
}

bool CharlesLog::checkTag(string tag) {
    if (log_conf.log_tags.end() != std::find(log_conf.log_tags.begin(), log_conf.log_tags.end(), tag) ||
            log_conf.log_tags.end() != std::find(log_conf.log_tags.begin(), log_conf.log_tags.end(), "all"))
        return true;

    return false;
}

void *run_callback(void *arg) {
    /*
     * make my thread work in IDLE mode.
     */
    struct sched_param priority;
    priority.sched_priority = 0;
    sched_setscheduler(0, SCHED_IDLE, &priority);

    CharlesLog *handle = (CharlesLog *)arg;
    handle->work();
    return NULL;
}

void CharlesLog::run() {
    pthread_attr_t thread_attr;
    pthread_attr_init(&thread_attr);
    pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&work_thread, &thread_attr, run_callback, this);
    pthread_attr_destroy(&thread_attr);
}

void CharlesLog::stop() {
    while (!messages.empty())
        sleep(1); /* wait for all messages be consumed */
    if (file != NULL) {
        fflush(file); /* write the last piece */
        fsync(fileno(file));
    }
    running = false;
}

void CharlesLog::work() {
    for (; !(messages.empty() && running == false);) {
        string message;
        pthread_mutex_lock(&queue_lock);
        while (messages.empty())
            pthread_cond_wait(&queue_cond, &queue_lock);
        message = messages.front();
        messages.pop();
        pthread_mutex_unlock(&queue_lock);
        if (-1 == updateFileHandle())
            continue;
        fwrite(message.c_str(), message.length(), 1, file);
    }
}

string CharlesLog::getLogName() {
    time_t now = time(0);
    struct tm localtm;
    localtime_r(&now, &localtm);
    char date[256];
    strftime(date, 256, "%F", &localtm);
    string log_name = log_conf.log_dir + "/" + log_conf.process_name + "_" + date + ".log";

    return log_name;
}

int CharlesLog::updateFileHandle() {
    string log_name = getLogName();
    if (current_file != log_name) {
        if (file != NULL)
            fclose(file);
        current_file = log_name;
        file = fopen(current_file.c_str(), "a+");
        if (file == NULL) {
            charles_err("open file %s failed. (%s)", current_file.c_str(), strerror(errno));
            return -1;
        }
    }

    return 0;
}

int CharlesLog::log(LOG_LEVEL lvl, string tag, string msg, const char *file, int line) {
    if (log_conf.log_level < lvl || !checkTag(tag))
        return -1;

    time_t now = time(0);
    char time_buffer[256];
    ctime_r(&now, time_buffer);
    time_buffer[strlen(time_buffer) - 1] = 0;
    char buffer[MAX_LINE];
    snprintf(buffer, MAX_LINE, "[ %s ] [%s] [ %s:%d ] [ %s ] %s\n", time_buffer, level[lvl], file, line, tag.c_str(), msg.c_str());
    pthread_mutex_lock(&queue_lock);
    messages.push(string(buffer));
    pthread_cond_signal(&queue_cond);
    pthread_mutex_unlock(&queue_lock);

    return 0;
}

void CharlesLog::writePiece(string msg) {
    piece[gettid()] += msg;
}

string CharlesLog::readPiece() {
    return piece[gettid()];
}

void CharlesLog::clearPiece() {
    piece[gettid()] = "";
}

uint64_t CharlesLog::gettid() {
	pthread_t ptid = pthread_self();
	uint64_t thread_id = 0;
	memcpy(&thread_id, &ptid, std::min(sizeof(thread_id), sizeof(ptid)));
	return thread_id;
}
