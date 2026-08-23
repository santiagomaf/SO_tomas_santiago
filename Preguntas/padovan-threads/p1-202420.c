#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <sys/wait.h>

typedef struct {
    int *arr;
    size_t size;
    int p_flag;
    size_t first_index;
    size_t last_index;
    int thread_num;
} SParams;

void print_usage(const char *prog_name) {
    printf("Usage: %s -s size -f first_index -e last_index -p parity_flag\n", prog_name);
    printf("  -s size           Size of the array\n");
    printf("  -f first_index    First index of the Padovan sequence\n");
    printf("  -e last_index     Last index of the Padovan sequence\n");
    printf("  -p parity_flag    1 for even indices, 0 for odd indices\n");
    exit(EXIT_FAILURE);
}

void *worker(void *args) {
    SParams *params = (SParams *)args;
    printf("Thread %d started\n", params->thread_num);
    printf("Array size: %zu, First index: %zu, Last index: %zu, Parity flag: %d\n",
           params->size, params->first_index, params->last_index, params->p_flag);

    // Fill the array with Padovan sequence values
    int *padovan = params->arr;
    padovan[0] = padovan[1] = padovan[2] = 1;

    for (size_t i = 3; i < params->size; ++i) {
        padovan[i] = padovan[i-2] + padovan[i-3];
    }

    for (size_t i = params->first_index; i <= params->last_index && i < params->size; i++) {
        if ((params->p_flag == 1 && i % 2 == 0) || (params->p_flag == 0 && i % 2 != 0)) {
            printf("Padovan[%zu] = %d\n", i, padovan[i]);
        }
    }

    pthread_exit(NULL);
}

void *compute_statistics(void *args) {
    SParams *params = (SParams *)args;
    int *arr = params->arr;
    size_t size = params->size;

    double sum = 0.0, mean, stddev = 0.0;
    for (size_t i = 0; i < size; ++i) {
        sum += arr[i];
    }
    mean = sum / size;

    for (size_t i = 0; i < size; ++i) {
        stddev += pow(arr[i] - mean, 2);
    }
    stddev = sqrt(stddev / size);

    printf("Thread %d finished\n", params->thread_num);
    printf("Mean: %.2f, Standard Deviation: %.2f\n", mean, stddev);

    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    size_t size = 0, first_index = 0, last_index = 0;
    int p_flag = 0;
    int opt;

    while ((opt = getopt(argc, argv, "s:f:e:p:")) != -1) {
        switch (opt) {
            case 's':
                size = atoi(optarg);
                break;
            case 'f':
                first_index = atoi(optarg);
                break;
            case 'e':
                last_index = atoi(optarg);
                break;
            case 'p':
                p_flag = atoi(optarg);
                break;
            default:
                print_usage(argv[0]);
        }
    }

    if (size == 0 || first_index > last_index || last_index >= size) {
        print_usage(argv[0]);
    }

    printf("Parent PID: %d\n", getpid());

    int *arr = malloc(size * sizeof(int));
    if (!arr) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }

    pthread_t worker_thread, stats_thread;
    SParams params = {arr, size, p_flag, first_index, last_index, 1};

    pthread_create(&worker_thread, NULL, worker, &params);
    // TODO 1: Crear thread que ejecute la función worker.
    pthread_join(worker_thread, NULL);
    pthread_create(&stats_thread,NULL,compute_statistics,&params);
    // TODO 2: Crear thread que ejecute la función compute_statistics.
    // Hacer esto después que el thread del TODO 1 haya terminado.
    

    

    // TODO 3: Una vez que el thread del TODO 2 termine, crear un proceso
    // hijo.
    pthread_join(stats_thread, NULL);

    // Fork and exec
    pid_t pid = fork();

    if (pid == 0){
        char p_flag_arg[10];
        snprintf(p_flag_arg, sizeof(p_flag_arg),"-p%d", !p_flag);
        char *new_argv[] = {argv[0], "-s", argv[2], "-f", argv[4], "-e", argv[6], p_flag_arg, NULL};
        execve(argv[0], new_argv, NULL);

        
    }
    else{
        wait(NULL);
    }

    // TODO 6: ¿Cuántos procesos se crean en total al ejecutar este programa
    // desde la consola?
    // Se genera una cantidad indefinida de procesos, pues cada nueva ejecución
    // del ejecutable (vía execve) vuelve a correr el mismo main desde el
    // principio, y por lo tanto vuelve a hacer fork() de un hijo propio.
    // Cada proceso espera (wait) a su hijo antes de terminar, pero ese hijo
    // nunca termina por sí solo: crea otro hijo, y así sucesivamente. Es una
    // cadena de procesos que nunca converge (no hay condición de corte),
    // por lo que el programa jamás termina y sigue generando procesos nuevos
    // indefinidamente hasta que se interrumpa manualmente o se agoten los
    // recursos del sistema.

    free(arr);
    return 0;
}
