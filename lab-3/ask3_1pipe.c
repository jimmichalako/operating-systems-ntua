#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/select.h>

void error(const char *arg) {
    printf("Usage: %s <nChildren> [--random] [--round-robin]\n", arg);
    exit(EXIT_FAILURE);
}

bool isNumber(char *buf) {
    for (int i = 0; i < strlen(buf); i++)
        if (isdigit(buf[i]) == false)
            return false;
    return true;
}

int max(int a, int b) {
    if (a > b)
        return a;
    return b;
}

int main(int argc, char **argv) {
    int n = 0;
    int method = 0; //0->round-robin(default), 1->random
    if (argc < 2 || argc > 3)
        error(argv[0]);
    if (!isNumber(argv[1]))
        error(argv[0]);
    else {
        n = atoi(argv[1]);
        if (n <= 0)
            error(argv[0]);
    }
    if (argc == 3) {
        if (!(strcmp("--round-robin", argv[2]) == 0 || strcmp("--random", argv[2]) == 0))
            error(argv[0]);
        if (strcmp("--random", argv[2]) == 0)
            method = 1;

    }
//////////----------PIPELINES----------//////////
    int pds[n][2];
    for (int i = 0; i < n; ++i) {
        if (pipe(pds[i]) != 0) {
            perror("pipe error");
            exit(EXIT_FAILURE);
        }
    }
//////////----------CHILDREN----------//////////
    int child_list[n];
    for (int i = 0; i < n; i++) {
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork error");
            exit(EXIT_FAILURE);
        }
        if (pid == 0) {
            int val;
            while (1) {
                int fd_in = read(pds[i][0], &val, sizeof(int));
                if (fd_in == -1) {
                    perror("read error");
                    exit(EXIT_FAILURE);
                }
                printf("[Child %d] [%d] Child received %d!\n", i, getpid(), val);
                val++;
                sleep(5);
                int fd_out = write(pds[i][1], &val, sizeof(int));
                if (fd_out == -1) {
                    perror("write error");
                    exit(EXIT_FAILURE);
                }
                printf("[Child %d] [%d] Child finished hard work, writing back %d\n", i, getpid(), val);
            }
        }
        child_list[i] = pid;
    }
//////////----------FATHER----------//////////
    int count = 0;
    while (1) {
        fd_set set;
        int maxfd;
        FD_ZERO(&set);                                  //clear all file descriptors from set
        FD_SET(STDIN_FILENO, &set);                     //check for input from stdin
        for (int i = 0; i < n; i++)
            FD_SET(pds[i][0], &set);                    //check for input from pipe
        maxfd = STDIN_FILENO;
        for (int i = 0; i < n; i++)
            maxfd = max(maxfd, pds[i][0]);
        maxfd += 1;                                     //select works only for fds smaller than the max
        int ready = select(maxfd, &set, NULL, NULL, NULL);  //wait until input is ready
        if (ready <= 0) {
            perror("select error");
            continue;
        }
        if (FD_ISSET(STDIN_FILENO, &set)) {             //read from stdin
            char buff[101];
            int input = read(STDIN_FILENO, buff, 100);
            if (input == -1) {
                perror("read error");
                exit(EXIT_FAILURE);
            }
            buff[input] = '\0';
            if (input > 0 && buff[input - 1] == '\n')   //discard new line character \n
                buff[input - 1] = '\0';
            if (input >= 4 && strcmp(buff, "exit") == 0) { //if user types exit
                for (int i = 0; i < n; ++i) {
                    int w_pid = waitpid(child_list[i], NULL, WNOHANG);
                    if (w_pid == 0) {
                        printf("Waiting for %d\n", n - i);
                        kill(child_list[i], SIGTERM);
                        wait(NULL);
                    }
                    close(pds[i][1]);
                    close(pds[i][0]);
                }
                printf("All children terminated\n");
                exit(0);
            } else if (input >= 1 && isNumber(buff)) { //if user types number
                int num = atoi(buff);
                int child = count % n;
                if (method == 0) { //round-robin method
                    if (write(pds[child][1], &num, sizeof(int)) == -1) {
                        perror("write error");
                        exit(EXIT_FAILURE);
                    }
                    printf("[Parent] Assigned %d to child %d\n", num, child);
                    count += 1;
                } else if (method == 1) { //random method
                    int random = rand() % n;
                    if (write(pds[random][1], &num, sizeof(int)) == -1) {
                        perror("write error");
                        exit(EXIT_FAILURE);
                    }
                    printf("[Parent] Assigned %d to child %d\n", num, random);
                }
            } else if (input >= 1)
                printf("Type a number to send job to a child\n");
        }
        for (int i = 0; i < n; ++i) { //children write to the pipe, parent will read it
            if (FD_ISSET(pds[i][0], &set)) {
                int output;
                if (read(pds[i][0], &output, sizeof(int)) == -1) {
                    perror("read error");
                    exit(EXIT_FAILURE);
                }
                printf("[Parent] Received result from child %d --> %d\n", i, output);
            }
        }
    }
}