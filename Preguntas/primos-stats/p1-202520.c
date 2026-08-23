#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>
#include <math.h>

typedef struct {
    int *arr;
    size_t size;
    double mean;
    int min, max;
} Params;

int is_prime(int n) {
    if (n < 2) return 0;
    for (int i = 2; i <= sqrt(n); i++) {
        if (n % i == 0) return 0;
    }
    return 1;
}

void *fill_primes(void *args) {
    Params *p = (Params*) args;
    int num = 2, idx = 0;
    while (idx < p->size) {
        if (is_prime(num)) {
            p->arr[idx++] = num;
        }
        num++;
    }
    pthread_exit(NULL);
}

void *compute_stats(void *args) {
    Params *p = (Params*) args;
    int sum = 0;
    p->min = p->arr[0];
    p->max = p->arr[0];
    for (size_t i = 0; i < p->size; i++) {
        int val = p->arr[i];
        sum += val;
        if (val < p->min) p->min = val;
        if (val > p->max) p->max = val;
    }
    p->mean = (double)sum / p->size;
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    size_t size = 20; // por simplicidad, tamaño fijo
    int *arr = malloc(size * sizeof(int));
    if (!arr) { perror("malloc"); exit(1); }

    Params params = {arr, size, 0, 0, 0};

    pthread_t t1, t2;

    // TODO 1: Crear thread que ejecute fill_primes
    // TODO 2: Esperar a que termine
    // TODO 3: Crear thread que ejecute compute_stats
    // TODO 4: Esperar a que termine
    // TODO 5: Crear proceso hijo con fork
    // El hijo debe imprimir min, max y mean
    // TODO 6: Padre espera a hijo y libera memoria

    return 0;
}
