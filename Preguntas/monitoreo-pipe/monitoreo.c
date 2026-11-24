#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>

#define SIZE 20
#define THRESHOLD 35

// Parámetros compartidos por los threads
typedef struct {
    int *measurements;
    size_t size;
    int threshold;
    double mean;
    int max;
    int above_threshold;
} Params;

// Resultados enviados mediante el pipe
typedef struct {
    double mean;
    int max;
    int above_threshold;
} Report;

// Llena el arreglo con mediciones deterministas
void *fill_measurements(void *args) {
    Params *p = (Params *) args;
    const int source[SIZE] = {
        15, 18, 20, 22, 24, 26, 28, 30, 32, 34,
        35, 21, 23, 17, 36, 38, 40, 42, 44, 45
    };

    for (size_t i = 0; i < p->size; i++)
        p->measurements[i] = source[i];

    pthread_exit(NULL);
}

// Calcula promedio, máximo y cantidad sobre el umbral
void *analyze_measurements(void *args) {
    Params *p = (Params *) args;
    int sum = 0;

    p->max = p->measurements[0];
    p->above_threshold = 0;

    for (size_t i = 0; i < p->size; i++) {
        int value = p->measurements[i];
        sum += value;

        if (value > p->max)
            p->max = value;
        if (value > p->threshold)
            p->above_threshold++;
    }

    p->mean = (double) sum / p->size;
    pthread_exit(NULL);
}

int main(void) {
    int *measurements = malloc(SIZE * sizeof(int));

    if (measurements == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    Params params = {
        .measurements = measurements, .size = SIZE,
        .threshold = THRESHOLD, .mean = 0.0,
        .max = 0, .above_threshold = 0
    };

    pthread_t producer_thread, analyzer_thread;

    // TODO 1: Crear producer_thread para ejecutar fill_measurements(&params).
    if (pthread_create(&producer_thread, NULL, fill_measurements, &params) != 0) {
        perror("pthread_create (producer)");
        free(measurements);
        exit(EXIT_FAILURE);
    }

    // TODO 2: Esperar el término de producer_thread.
    if (pthread_join(producer_thread, NULL) != 0) {
        perror("pthread_join (producer)");
        free(measurements);
        exit(EXIT_FAILURE);
    }

    // TODO 3: Crear analyzer_thread para ejecutar analyze_measurements(&params).
    if (pthread_create(&analyzer_thread, NULL, analyze_measurements, &params) != 0) {
        perror("pthread_create (analyzer)");
        free(measurements);
        exit(EXIT_FAILURE);
    }

    // TODO 4: Esperar el término de analyzer_thread.
    if (pthread_join(analyzer_thread, NULL) != 0) {
        perror("pthread_join (analyzer)");
        free(measurements);
        exit(EXIT_FAILURE);
    }

    int pipefd[2];

    // TODO 5: Crear un pipe usando pipefd.
    if (pipe(pipefd) == -1) {
        perror("pipe");
        free(measurements);
        exit(EXIT_FAILURE);
    }

    // TODO 6: Crear un proceso hijo mediante fork().
    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        close(pipefd[0]);
        close(pipefd[1]);
        free(measurements);
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        // Hijo: cerrar lectura; crear y escribir Report; cerrar escritura;
        // terminar mediante _exit(EXIT_SUCCESS).
        close(pipefd[0]);

        Report report = {
            .mean = params.mean,
            .max = params.max,
            .above_threshold = params.above_threshold
        };

        ssize_t written = write(pipefd[1], &report, sizeof(report));

        if (written < 0 || (size_t) written != sizeof(report)) {
            perror("write");
            close(pipefd[1]);
            _exit(EXIT_FAILURE);
        }

        close(pipefd[1]);
        _exit(EXIT_SUCCESS);
    }
    else {
        // Padre: cerrar escritura; leer Report; cerrar lectura; usar waitpid;
        // imprimir el reporte con el formato indicado y liberar measurements.
        close(pipefd[1]);

        Report report;
        ssize_t bytes_read = read(pipefd[0], &report, sizeof(report));

        if (bytes_read < 0 || (size_t) bytes_read != sizeof(report)) {
            perror("read");
            close(pipefd[0]);
            free(measurements);
            exit(EXIT_FAILURE);
        }

        close(pipefd[0]);

        waitpid(pid, NULL, 0);

        printf("[parent] Reporte recibido:\n");
        printf("  promedio: %.2f\n", report.mean);
        printf("  máximo: %d\n", report.max);
        printf("  mediciones sobre umbral: %d\n", report.above_threshold);

        free(measurements);
    }

    return EXIT_SUCCESS;
}
