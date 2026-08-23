#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

int contador = 0;

void* func(void *arg){
    int *cap = (int *) arg;
    printf("soy el thread, mi pid es %d\n", getpid()); 

    for (int i = 0; i < *cap; i++){
        contador++;
    }
    return NULL;
}
int main(){
    pid_t id = getpid(); 
    printf("test exec() mi pid es: %d\n", id);
    pthread_t A,B;
    int iteraciones = 10000000;

    pthread_create(&A, NULL, func, &iteraciones);
    pthread_create(&B, NULL, func, &iteraciones);

    pthread_join(A,NULL);
    pthread_join(B,NULL);

    printf("%d\n", contador);
    return 0;
}