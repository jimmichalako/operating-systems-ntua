#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>

int term = 0, killed = 0, length;
pid_t *child;

void check_neg(int ret, const char *msg) {
    if (ret < 0) {
        perror(msg);
        exit(EXIT_FAILURE);
    }
}

void describe_wait_status(pid_t pid, int status) {
    if (pid < 1) {
        perror("wait() call failed");
    }

    if (pid == 0) {
        printf("Nothing happened");
    }

    if (WIFSTOPPED(status)) {
        printf("Child with PID %d stopped\n", pid);
    } else if (WIFCONTINUED(status)) {
        printf("Child with PID %d continued\n", pid);
    } else if (WIFEXITED(status)) {
        if (term == 0) {
            printf("Child with PID=%d exited\n", pid);
        } else {
            printf("Child with PID=%d terminated successfully with status code %d\n", pid,
                   WEXITSTATUS(status));
        }
    } else if (WIFSIGNALED(status)) {
        printf("Child with PID %d terminated by signal %d with status code %d\n",
               pid, WSTOPSIG(status), WEXITSTATUS(status));
    }
}

void SIGTERM_handler() {
    term = 1;
    for (int i = 0; i < length; i++)
        kill(child[i], SIGTERM);
}

void SIGUSR1_handler() {
    for (int i = 0; i < length; i++)
        kill(child[i], SIGUSR1);
}

void SIGUSR2_handler() {
    for (int i = 0; i < length; i++)
        kill(child[i], SIGUSR2);
}

void SIGCHLD_handler() {
    killed = 1;
}

int main(int argc, char **argv) {
    struct sigaction sigt, sig1, sig2, sigc;
    sigt.sa_handler = SIGTERM_handler;
    sigt.sa_flags = 0;
    sig1.sa_handler = SIGUSR1_handler;
    sig1.sa_flags = 0;
    sig2.sa_handler = SIGUSR2_handler;
    sig2.sa_flags = 0;
    sigc.sa_handler = SIGCHLD_handler;
    sigc.sa_flags = 0;
    sigaction(SIGTERM, &sigt, NULL);
    sigaction(SIGUSR1, &sig1, NULL);
    sigaction(SIGUSR2, &sig2, NULL);
    sigaction(SIGCHLD, &sigc, NULL);
    if (argc != 2) {
        printf("Input error\n");
        return 1;
    }
    length = (int) strlen(argv[1]);
    pid_t child_pid[length];
    int status;
    for (int i = 0; i < length; i++) {
        child_pid[i] = fork();
        check_neg(child_pid[i], "fork error");
        if (child_pid[i] == 0) {
            char gate[100];
            sprintf(gate, "%c", argv[1][i]);
            char number[100];
            sprintf(number, "%d", i);
            char *argv_child[] = {gate, number, NULL};
            status = execv("./child", argv_child);
            check_neg(status, "Failed to create child");
        } else {
            printf("[PARENT/PID=%d] Created child %d (PID=%d) and initial state '%c'\n", getpid(), i, child_pid[i],
                   argv[1][i]);
        }
    }
    child = child_pid;
    while (1) {
        if (term == 0) {
            if (killed == 1) {
                pid_t check = waitpid(-1, &status, WUNTRACED);
                if (WIFSTOPPED(status))
                    kill(check, SIGCONT);
                else {
                    describe_wait_status(check, status);
                    for (int i = 0; i < length; i++) {
                        if (check == child[i]) {
                            child[i] = fork();
                            check_neg(child[i], "fork error");
                            if (child[i] == 0) {
                                printf("[PARENT/PID=%d] Created new child for gate %d (PID=%d) and initial state '%c'\n",
                                       getppid(), i, getpid(), argv[1][i]);
                                char gate[100];
                                sprintf(gate, "%c", argv[1][i]);
                                char number[100];
                                sprintf(number, "%d", i);
                                char *argv_child[] = {gate, number, NULL};
                                status = execv("./child", argv_child);
                                check_neg(status, "Failed to create child");
                            }
                        }
                    }
                }
                killed = 0;
            }
        } else {
            for (int i = 0; i < length; i++) {
                printf("Waiting for %d children to exit\n", length - i);
                pid_t pid = waitpid(child[i], &status, 0);
                describe_wait_status(pid, status);
            }
            printf("All children exited, terminating as well\n");
            break;
        }
    }
    return 0;
}