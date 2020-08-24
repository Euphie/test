#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <errno.h>

#define THREAD_NUM 5 // 线程数
#define THREAD_REQUEST_NUM 100 // 每个线程请求次数

typedef struct report {
    uint s_count;
    uint f_count;
} report_t;

void *connection_test(char **argv);

int main(int argc, char *argv[]) {
    int ret;
    pthread_t tids[THREAD_NUM];
    report_t report, *t_report;

    if(argc < 3) {
        printf("Usage: %s 192.168.1.2 3306", argv[0]);
        exit(1);
    }

    // srand(time(NULL));
    while(1) {
        for (int i = 0; i < THREAD_NUM; i++) {
            ret = pthread_create(&tids[i], NULL, (void *) connection_test, (void *)argv);
            if (ret != 0) {
                printf("thread creation error: %s\n", strerror(ret));
                exit(1);
            }
        }

        for (int i = 0; i < THREAD_NUM; i++) {
            ret = pthread_join(tids[i], (void *)&t_report);
            if (ret != 0) {
                printf("thread joined error: %s\n", strerror(ret));
                exit(1);
            }

            printf("%d\n", t_report->s_count);
        }

        sleep(1);
    }

    return 0;
}

void *connection_test(char **argv) {
    int ret, sock;
    char *ip = argv[1];
    uint port = (uint)strtol(argv[2], NULL, 10);
    struct sockaddr_in serv_addr;
    time_t timep;
    report_t report;

    memset(&serv_addr, 0, sizeof(serv_addr));
    memset(&report, 0, sizeof(report));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(ip);
    serv_addr.sin_port = htons(port);
    for (int i = 0; i < THREAD_REQUEST_NUM; ++i) {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if(sock == -1 ) {
            time(&timep);
            printf("%ssocket error: %s\n\n", ctime(&timep), strerror(errno));
            report.f_count++;
            continue;
        }

        ret = connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
        if(ret == -1) {
            time(&timep);
            printf("%sconnection error: %s\n\n", ctime(&timep), strerror(errno));
            report.f_count++;
            continue;
        } else {
            report.s_count++;
        }

        close(sock);
        usleep(1000);
    }

    pthread_exit((void *)&report);
}
