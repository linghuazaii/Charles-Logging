#include "charles_log.h"

int main(int argc, char **argv) {
    CharlesLog *log = CharlesLog::getInstance();
    log->loadConfig("../conf/log_conf");

    return 0;
}
