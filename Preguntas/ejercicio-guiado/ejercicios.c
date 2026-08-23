#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main(void){
    pid_t pid_f;
    pid_t pid_c;
    pid_f = getpid();
    printf("%i \n", pid_f);

    int fd[2];

    pipe(fd);

    pid_c = fork();

    if (pid_c == 0) {
        close(fd[1]);

        char buffer[100];
        read(fd[0], buffer, sizeof(buffer));


        
        printf("es el proceso hijo\n");

        sleep(2);

        printf("el proceso hijo recive esto %s\n", buffer);

        execl(buffer, "test_exec", NULL);

    }
    else{
        
        close(fd[0]);

        char *msg = "./test_exec";

        write(fd[1], msg, strlen(msg) + 1);

        wait(NULL);


        printf("proceso padre espero al hijo\n");
    }





    return 0;
}