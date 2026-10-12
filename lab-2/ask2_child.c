#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

int time = 0, term = 0, al = 0, peek = 0;
char gate;

void SIGTERM_handler() {
    term = 1;
}

void SIGUSR1_handler() {
    if (gate == 't')
        gate = 'f';
    else gate = 't';
    peek = 1;
}

void SIGUSR2_handler() {
    peek = 1;
}

void SIGALRM_handler() {
    al = 1;
}

int main(int argc, char **argv) {
    usleep(50000);
    struct sigaction sigt, sig1, sig2, siga;
    sigt.sa_handler = SIGTERM_handler;
    sigt.sa_flags = 0;
    sig1.sa_handler = SIGUSR1_handler;
    sig1.sa_flags = 0;
    sig2.sa_handler = SIGUSR2_handler;
    sig2.sa_flags = 0;
    siga.sa_handler = SIGALRM_handler;
    siga.sa_flags = 0;
    sigaction(SIGTERM, &sigt, NULL);
    sigaction(SIGUSR1, &sig1, NULL);
    sigaction(SIGUSR2, &sig2, NULL);
    sigaction(SIGALRM, &siga, NULL);
    gate = argv[0][0];
    char number = argv[1][0];
    raise(SIGALRM);
    int i = -1;
    while (1) {
        if (al == 1) {
            i++;
            printf("[GATE=%c/PID=%d/TIME=%ds] The gates are ", number, getpid(), time);
            if (gate == 't') {
                printf("open\n");
            } else printf("closed\n");
            al = 0;
            time += 15;
            alarm(15);
        }
        if (peek == 1) {
            int remaining = alarm(0);
            int current = (i + 1) * 15 - remaining;
            printf("[GATE=%c/PID=%d/TIME=%ds] The gates are ", number, getpid(), current);
            if (gate == 't') {
                printf("open\n");
            } else printf("closed\n");
            peek = 0;
            alarm(remaining);
        }
        if (term == 1)
            break;
    }
    return 0;
}