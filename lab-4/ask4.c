#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>

#define HOST_DEFAULT "iot.dslab.pub.ds.open-cloud.xyz"
#define PORT_DEFAULT 18080

bool isNumber(char buf[]) {
    for (int i = 0; i < strlen(buf); i++)
        if ((isdigit(buf[i]) || buf[i] == '\n' || buf[i] == ' ') == false)
            return false;
    return true;
}

int main(int argc, char **argv) {
    bool debug = false;
    char *host = HOST_DEFAULT;
    u_short port = PORT_DEFAULT;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--host") == 0) {
            if (argv[i + 1] != NULL)
                host = argv[i + 1];
            else
                return 1;
        }
        if (strcmp(argv[i], "--port") == 0) {
            if (argv[i + 1] != NULL)
                port = atoi(argv[i + 1]);
            else
                return 1;
        }
        if ((strcmp(argv[i], "--debug") == 0)) {
            debug = true;
        }
    }
    //////////----------SOCKETS----------//////////
    int sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd < 0) {
        perror("socket");
        return -1;
    }
    //////////----------CLIENT SOCKET-PORT BIND----------//////////
    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(0);
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sd, (struct sockaddr *) &sin, sizeof(sin)) < 0) {
        perror("bind");
        return -1;
    }
    //////////----------HOST DATA----------//////////
    char *hostname = host;
    struct hostent *hostp;
    hostp = gethostbyname(hostname);
    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_port = htons((u_short) port);
    bcopy(hostp->h_addr, &sa.sin_addr, hostp->h_length);
    if (connect(sd, (struct sockaddr *) &sa, sizeof(sa)) < 0) {
        perror("connect");
        return -1;
    } else {
        printf("Connecting!\n");
        printf("Connected to %s:%d\n", host, port);
    }
    //////////----------INPUT----------//////////
    char buffer[201];
    fd_set inset, copy;
    FD_ZERO(&copy);
    FD_SET(0, &copy);
    FD_SET(sd, &copy);
    while (1) {
        inset = copy;
        if (select(FD_SETSIZE, &inset, NULL, NULL, NULL) < 0) {
            perror("select");
            break;
        }
        if (FD_ISSET(0, &inset)) {
            int input = read(0, buffer, sizeof(buffer));
            if (input == -1) {
                perror("read");
                return -1;
            }
            buffer[input] = '\0';
            if (input > 0 && buffer[input - 1] == '\n')
                buffer[input - 1] = '\0';
            if (strcmp(buffer, "help") == 0) {
                printf("Available commands:\n");
                printf("-> 'help'                   :prints help message\n");
                printf("-> 'exit'                   :exit\n");
                printf("-> 'get'                    :retrieve server data\n");
                printf("-> 'N name surname reason'  :exit permission in quarantine\n");
            } else if (input >= 4 && strcmp(buffer, "exit") == 0) {
                if (shutdown(sd, 2) < 0) {
                    perror("shutdown");
                    return -1;
                }
                close(sd);
                exit(0);
            } else if (input >= 3 && strcmp(buffer, "get") == 0) {
                if (write(sd, buffer, 3) < 0) {
                    perror("write");
                    return -1;
                }
                if (debug)
                    printf("[DEBUG] sent '%s'\n", buffer);
            } else if (input >= 1) {
                if (write(sd, buffer, input) < 0) {
                    perror("write");
                    return -1;
                }
                if (debug)
                    printf("[DEBUG] sent '%s'\n", buffer);
            }
        }
        if (FD_ISSET(sd, &inset)) {
            char server_buffer[200];
            int server_in = read(sd, server_buffer, sizeof(server_buffer));
            server_buffer[server_in - 1] = '\0';
            if (strncmp(server_buffer, "try again", strlen("try again") - 1) == 0) {
                printf("%s", server_buffer);
                memset(server_buffer, 0, sizeof(server_buffer));
                continue;
            } else if (server_in >= 1 && isNumber(server_buffer)) {
                if (debug)
                    printf("[DEBUG] read '%s'\n", server_buffer);
                for (int i = 0; i < 30; ++i) {
                    printf("-");
                }
                printf("\n");
                char interval[50];
                char temperature[50];
                char light[50];
                char clock[50];
                strncpy(interval, server_buffer, 1);
                if (atoi(interval) == 0)
                    printf("boot (0)\n");
                if (atoi(interval) == 1)
                    printf("setup (1)\n");
                if (atoi(interval) == 2)
                    printf("interval (2)\n");
                if (atoi(interval) == 3)
                    printf("button (3)\n");
                if (atoi(interval) == 4)
                    printf("motion (4)\n");
                strncpy(temperature, server_buffer + 6, 4);
                printf("Temperature is: %.2f\n", atof(temperature) / 100.0);
                strncpy(light, server_buffer + 2, 3);
                printf("Light Level is: %d\n", atoi(light));
                strncpy(clock, server_buffer + 11, 10);
                time_t temp = atoi(clock);
                struct tm *timeinfo = localtime((const time_t *) &temp);
                char time_buffer[20];
                strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
                printf("Timestamp is: %s\n", time_buffer);
            } else if (server_in >= 1 && strncmp(server_buffer, "ACK", strlen("ACK") - 1) == 0) {
                if (debug)
                    printf("[DEBUG] read '%s'\n", server_buffer);
                printf("Response: '%s'\n", server_buffer);
            } else {
                if (debug)
                    printf("[DEBUG] read '%s'\n", server_buffer);
                printf("Send verification code: %s\n", server_buffer);
            }
        }
    }
    close(sd);
}