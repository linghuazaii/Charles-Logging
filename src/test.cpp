#include "charles_log.h"
#include "unistd.h"

void *thread_routine1(void *arg) {
    for (int i = 0; i < 1000; ++i) {
        LOG_INFO("thread routine 1> %d", time(0));
        printf("1\n");
    }

    return NULL;
}

void *thread_routine2(void *arg) {
    for (int i = 0; i < 1000; ++i) {
        LOG_INFO_T("http", "thread routine 2> %d", time(0));
        printf("2\n");
    }

    return NULL;
}

void *thread_routine3(void *arg) {
    for (int i = 0; i < 1000; ++i) {
        LOG_INFO_T("not exist", "thread routine 3> %d", time(0));
        printf("3\n");
    }

    return NULL;
}

void *thread_routine4(void *arg) {
    for (int i = 0; i < 1000; ++i) {
        LOG_INFO("thread routine 4> normal logging");
        printf("4\n");
    }

    return NULL;
}

int main(int argc, char **argv) {
    START_CHARLES_LOGGING("../conf/log_conf");

    pthread_t thread1, thread2, thread3, thread4;
    pthread_create(&thread1, NULL, thread_routine1, NULL);
    pthread_create(&thread2, NULL, thread_routine2, NULL);
    pthread_create(&thread3, NULL, thread_routine3, NULL);
    pthread_create(&thread4, NULL, thread_routine4, NULL);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    pthread_join(thread3, NULL);
    pthread_join(thread4, NULL);

    STOP_CHARLES_LOGGING();

    return 0;
}
