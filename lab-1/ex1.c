#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>

int main(int argc, char* argv[]) {
    if(argc!=2){
        printf("Usage: ./a.out filename\n");
        return 1;
    }
    struct stat buffer;
    if(stat(argv[1],&buffer)==0){
        printf("Error: %s already exists\n", argv[1]);
        return 1;
    }
    if(strcmp(argv[1], "--help")==0){
        printf("./a.out filename\n");
        return 0;
    }
    int fd=open(argv[1], O_CREAT|O_APPEND|O_WRONLY,0644);
    if (fd==-1){
        perror("Couldn't open file\n");
        return 1;
    }
    int status;
    pid_t child = fork();
    char output[50];
    if (child < 0) {
        write(fd,"Failed to create child process", strlen("Failed to create child process"));
    } else if (child == 0) {
        sprintf(output, "[CHILD] getpid()=%d getppid()=%d\n", getpid(), getppid());
        if (write(fd,output, strlen(output))< strlen(output)){
            perror("Couldn't write on file");
            return 1;
        }
        exit(0);
    } else {
        wait(&status);
        sprintf(output, "[PARENT] getpid()=%d getppid()=%d\n", getpid(), getppid());
        if (write(fd,output, strlen(output))< strlen(output)){
            perror("Couldn't write on file");
            return 1;
        }
        close(fd);
        exit(0);
    }
    return 0;
}