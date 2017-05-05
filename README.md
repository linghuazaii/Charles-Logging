# Charles' Simple Logging System
This project is to be aimed to be simple and just for learning purpose. And you are free to modify and redistribute it under the author's name. It's license is GPL, though I don't know much details of GPL license. View the restrictions here.[GPL](https://github.com/linghuazaii/Charles-Logging/blob/master/LICENSE)

### Design Purpose
I have found logging in my company's project so stupid that logging functions like `LOG_ERROR` or anything else will block business handling. When the logging grows bigger and bigger, then it becomes a monster which eats several milliconds in time cost of request handling. So stupid, Huh!   
Even if `write` with `APPEND` to the same file won't be interleaved in multi-threading situation for now? Won't it be in the future file system? So a wise design is using Producer/Consumer pattern. Multiple threads write logging messages to the logging queue and only one thread consumes messages from that queue. The consumer thread works in `IDLE` mode not to affect the global performance. Just think about it, perfect! **PERFECT!!!**  
Now, let's implement it!

### Make & Install
`git clone https://github.com/linghuazaii/Charles-Logging.git` then  
`cd Charles-Logging` and `make` then   
the header file is in `include` and library in `lib`. 

### How to use
You should `#include "charles_log.h"` when you need to use this simple logging system.  
Then in the main thread, you should call `START_CHARLES_LOGGING(your_log_config_file)` to start the logging system. It will create a background thread to write logs to file.  
When you finish all of your bussiness, call `STOP_CHARLES_LOGGING()` to flush all data to file and then to disk.  
Use these interface to do logging:  
```cpp
#define LOG_INFO_T(tag, fmt, ...) LOG_IMP(LOG_INFO, tag, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) LOG_INFO_T("charles_logging", fmt, ##__VA_ARGS__)
#define LOG_WARN_T(tag, fmt, ...) LOG_IMP(LOG_WARN, tag, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...) LOG_WARN_T("charles_logging", fmt, ##__VA_ARGS__)
#define LOG_ERROR_T(tag, fmt, ...) LOG_IMP(LOG_ERROR, tag, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) LOG_ERROR_T("charles_logging", fmt, ##__VA_ARGS__)

/* 
 * all these macros end with a T means tag is needed, e.g.
 * LOG_INFO_T("http", "http is the tag %s", "http");
 * all these macros without suffix T means normal log, e.g.
 * LOG_ERROR("I don't need a tag");
 */

#define LOG_PIECE(fmt, ...) do {\
    CharlesLog *charles_log = CharlesLog::getInstance();\
    char message[MAX_LINE];\
    snprintf(message, MAX_LINE, fmt, ##__VA_ARGS__);\
    charles_log->writePiece(message);\
} while (0)

#define LOG_INFO_TP(tag) FLUSH_PIECE(LOG_INFO, tag)
#define LOG_INFO_P() FLUSH_PIECE(LOG_INFO, "charles_logging")
#define LOG_WARN_TP(tag) FLUSH_PIECE(LOG_WARN, tag)
#define LOG_WARN_P() FLUSH_PIECE(LOG_WARN, "charles_logging")
#define LOG_ERROR_TP(tag) FLUSH_PIECE(LOG_ERROR, tag)
#define LOG_ERROR_P() FLUSH_PIECE(LOG_ERROR, "charles_logging")

/*
 * all these macros is for writling log piece by piece and then flush once.
 * this is useful in situation you need to write multiple logs and don't want
 * them to be interleaved with logs in other threads. each thread has it's own 
 * pieces, they are thread safe. e.g.
 * Thread1 =>. 
 * LOG_PIECE("thread1 piece 0001");
 * LOG_PIECE("thread1 piece 0002");
 * LOG_INFO_P();
 * Thread2 =>. 
 * LOG_PIECE("thread2 piece 0001");
 * LOG_PIECE("thread2 piece 0002");
 * LOG_ERROR_P();
 * 
 * log in thread1 and thread2 won't interleave.
 */
```
Here is the sample config file, you can find it in `conf/log_conf` writting in JSON.
```
# '#' is for comments, don't use # in key or value
{
    # LOG_NONE, LOG_INFO, LOG_WARN, LOG_ERROR, lower log level will take effect if higher log level is set, use LOG_NONE to shut off logging system
    # LOG_NONE = 0, LOG_INFO = 1, LOG_WARN = 2, LOG_ERROR = 3
    "log_level": 3,
    # log tags, useful in debugging. if 'all' is contained or no tags specified, all tags will take effect
    # tags are case insensitive, e.g. SSH or Ssh will be convert to ssh
    # there is a default tag named charles-logging. don't use it.
    "log_tags": ["ssh", "tcp", "http"], # you can also comment here
    "log_dir": "../log", # log directory, default is ../log
    "process_name": "logging" # process_name is use in prefix of log name, if not given, name will be extrat from environment
}
```
And you can find a sample in `src/test.cpp`, to run the sample you should `export LD_LIBRARY_PATH=../lib:$LD_LIBRARY_PATH` first for `libcharles_log.so` depends on `libjson-c`. You should set this environment variable to let `libcharles_log.so` find where json-c locates. then run `test`, and check log file in `log/`.

### End
It is very simple and very easy to use and the most important thing is that it won't let io block your bussiness.
