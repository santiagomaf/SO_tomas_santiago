#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include "constants.h"

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    int pd[2];
    pid_t pid;
    enum { READ_END = 0, WRITE_END = 1 };

    if (pipe(pd) == -1) {
        perror("Pipe failed");
        return EXIT_FAILURE;
    }

    char sr[5], sw[5];

    sprintf(sr, "%d", pd[0]);
    sprintf(sw, "%d", pd[1]);
    printf("[parent %d] read end: '%s' write end: '%s'\n",
           getpid(), sr, sw);

    pid = fork();

    if (pid > 0) {
        close(pd[READ_END]);

        FILE *fp;
        fp = fopen("/dev/urandom", "r");

        int reads = 128;
        int byte_count = BUFFER_SIZE;

        char data[BUFFER_SIZE];
        int rb = 0, bwritten = 0;
        for (int i = 0; i < reads; i++) {
            if ((rb = fread(data, 1, byte_count, fp)) > 0)
            {
                write(pd[WRITE_END], data, rb);
                bwritten += rb;

                printf("[parent %d] Writing to pipe, write #%d, bytes written %d\n",
                       getpid(), i+1, bwritten);
            }
        }
        printf("[parent %d] Waiting for child to end", getpid());
        fclose(fp);

        wait(&pid);

        close(pd[WRITE_END]);
    }
    else {
        char* args[] = {sr,sw, NULL};
        execv("./child", args);
    }

    return EXIT_SUCCESS;

}
