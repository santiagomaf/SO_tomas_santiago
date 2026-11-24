#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include "constants.h"

typedef struct {
  int     num;
  int*    pinvalues;
  size_t  in_size;
  int*    poutvalues;
  size_t  out_size;
} TArgs;

int cmpint(const void *a, const void *b) {
  return (*(int *)a - *(int *)b);
}

void* thread_func(void *args) {
  TArgs* targs = args;
  assert(targs);

  int *pin = targs->pinvalues, *pout = targs->poutvalues;

  memset(targs->poutvalues, 0, targs->out_size);

  if (!(targs->num % 2)) {
    pin++;
  }

  printf("[child %d, thread %d] Starting.\n",
    getpid(),
    targs->num);

  for (; pin < targs->pinvalues + targs->in_size &&
        pout < targs->poutvalues + targs->out_size;
        pin+=2, pout++) {
    *pout = *pin;
  }

  qsort(targs->poutvalues, targs->out_size, sizeof(int), cmpint);

  printf("[child %d, thread %d] Finished.\n",
    getpid(),
    targs->num);

  return NULL;
}

int avg_array(int* array, size_t size) {
  return (array[0] + array[(size >> 1) - 1] + array[size - 1]) / 3;
}

int main(int argc, char **argv) {
  printf("[child %d] Child initiating.\n", getpid());
  assert(VALUE_ARRAY_SIZE % 2 == 0);

  const int read_end = atoi(argv[0]);
  const int write_end = atoi(argv[1]);

  printf("[child %d] read end: '%d' write end: '%d'\n",
    getpid(), read_end, write_end);

  int bread = 0, rb = 0, count = 1;
  int values[VALUE_ARRAY_SIZE/sizeof(int)];
  char *values_ptr = (char *) values;

  while(bread < VALUE_ARRAY_SIZE) {
    rb = read(read_end, values_ptr + bread, BUFFER_SIZE);
    if (rb == -1) {
      perror("Read error");
      exit(EXIT_FAILURE);
    }
    else if (rb == 0) {
      break;
    }
    bread += rb;
    printf("[child %d] Reading from pipe, read #%d, bytes read %d\n",
      getpid(), count, bread);
    count++;
  }

  close(read_end);

  printf("[child %d] Will launch threads.\n", getpid());

  int *poutarray_1, *poutarray_2;
  poutarray_1 = malloc(sizeof(int)*(VALUE_ARRAY_SIZE/sizeof(int)) >> 1);
  poutarray_2 = malloc(sizeof(int)*(VALUE_ARRAY_SIZE/sizeof(int)) >> 1);

  pthread_t t1, t2;
  TArgs args_1 = { 1, values, (VALUE_ARRAY_SIZE/sizeof(int)),
                  poutarray_1, (VALUE_ARRAY_SIZE/sizeof(int)) >> 1 };
  TArgs args_2 = { 2, values, (VALUE_ARRAY_SIZE/sizeof(int)),
                  poutarray_2, (VALUE_ARRAY_SIZE/sizeof(int)) >> 1 };

  /* TODO (5): completar creación de threads t1 y t2 */
  pthread_create(&t1, NULL, thread_func, &args_1);
  pthread_create(&t2, NULL, thread_func, &args_2);

  // Creación de los threads

  printf("[child %d] Threads launched.\n", getpid());

  /* TODO (6): Esperar a que threads t1 y t2 terminen */
  pthread_join(t1, NULL);
  pthread_join(t2, NULL);

  int sum_t1 = avg_array(poutarray_1, (VALUE_ARRAY_SIZE/sizeof(int)) >> 1);
  int sum_t2 = avg_array(poutarray_2, (VALUE_ARRAY_SIZE/sizeof(int)) >> 1);

  printf("Thread 1 result: %d\n", sum_t1);
  printf("Thread 2 result: %d\n", sum_t2);

  free(poutarray_1);
  free(poutarray_2);

  printf("[child %d] Finished.\n", getpid());
}
