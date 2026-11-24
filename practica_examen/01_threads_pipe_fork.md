# 01. Threads + pipe + fork en C (Prueba 1)

> Peso en el examen: 1 de los 4 problemas de código. Formato típico: completar 6 TODOs en `main`; el **TODO 6 vale el doble**.

---

## Parte 1. Explicación

### 1.1 Threads (pthreads)

```c
#include <pthread.h>

typedef struct {
    int *arr;       // entrada
    size_t n;       // entrada
    double mean;    // salida: el thread escribe aquí
} Params;

void *worker(void *args) {
    Params *p = (Params *) args;   // convertir el void* de vuelta al tipo real
    double sum = 0;
    for (size_t i = 0; i < p->n; i++) sum += p->arr[i];
    p->mean = sum / p->n;          // "devolver" el resultado escribiendo en el struct
    return NULL;
}

int main(void) {
    Params params = { arr, n, 0.0 };
    pthread_t t;
    int rc = pthread_create(&t, NULL, worker, &params);  // worker SIN paréntesis
    if (rc != 0) { /* error */ }
    rc = pthread_join(t, NULL);
    if (rc != 0) { /* error */ }
    printf("%f\n", params.mean);   // recién aquí es seguro leer
}
```

Reglas:

- La función del thread tiene firma `void *f(void *)`. Se pasa **el nombre** (puntero a función), no `f()`.
- Un solo argumento `void *`. Si necesitas varios, van en un **struct** y pasas `&struct`.
- Los resultados se escriben **en el mismo struct**.
- **pthreads retorna 0 si todo va bien y un código de error ≠ 0 si falla.** No retorna −1 ni setea `errno`. Para imprimir el error: `fprintf(stderr, "...: %s\n", strerror(rc))`.
- `fork`, `pipe`, `read`, `write`, `waitpid` sí retornan **−1** en error (y setean `errno`, por eso con ellos se usa `perror`).

**Preguntas (responde con el mecanismo):**

1. ¿Por qué hay que hacer `pthread_join` **antes** de leer `params.mean`? ¿Qué podría imprimir el programa si lo lees antes?
2. ¿Qué está mal en este código y por qué puede funcionar "a veces"?
   ```c
   void lanzar(pthread_t *t) {
       Params local = { arr, n, 0 };
       pthread_create(t, NULL, worker, &local);
   }
   ```
3. Un compañero escribe `if (pthread_create(&t, NULL, worker, &p) == -1)`. ¿Qué errores deja pasar?

### 1.2 Pipe

```c
int fd[2];
if (pipe(fd) == -1) { perror("pipe"); /* liberar y salir */ }
// fd[0] = extremo de LECTURA, fd[1] = extremo de ESCRITURA
```

Reglas:

- El pipe se crea **antes** del `fork`; así el hijo hereda los dos descriptores.
- Después del `fork` hay **4 descriptores abiertos** (2 en cada proceso). Cada proceso **cierra el extremo que no usa**.
- Enviar un struct completo: `write(fd[1], &r, sizeof(r))` y verificar que retorne **exactamente** `sizeof(r)`.
- Recibir: `read(fd[0], &r, sizeof(r))` y verificar que retorne **exactamente** `sizeof(r)`. `read` retorna 0 en EOF (todos los escritores cerraron).
- No se mandan structs con **punteros** por un pipe.

**Preguntas:**

4. El padre lee y nunca cierra su `fd[1]`. El hijo termina sin escribir nada. ¿Qué le pasa al `read` del padre? Explica qué condición necesita el kernel para entregar EOF.
5. ¿Por qué un struct con un campo `int *arr` no sirve si lo mandas por el pipe al otro proceso?
6. `read` retornó 4 y `sizeof(r)` es 24. ¿Qué significa y qué haces?

### 1.3 fork / _exit / waitpid

```c
fflush(NULL);              // vaciar buffers de stdio antes de duplicarlos
pid_t pid = fork();
if (pid == -1) { perror("fork"); /* liberar y salir */ }
if (pid == 0) {
    // HIJO: copia de la memoria del padre, sigue desde aquí
    _exit(0);
}
// PADRE: pid = PID del hijo
if (waitpid(pid, NULL, 0) < 0) { perror("waitpid"); }
```

Reglas:

- `fork` retorna **−1** (error, no hay hijo), **0** (en el hijo), **PID del hijo** (en el padre).
- El hijo empieza a ejecutar **desde el retorno de `fork`** con una **copia** de la memoria del padre (no la comparte).
- El hijo termina con `_exit(...)`, no con `return` ni `exit`.
- El padre espera con `waitpid(pid, NULL, 0)` y verifica `< 0`.
- `free(...)` de la memoria dinámica en **todos** los caminos de salida del padre, incluidos los de error.

**Preguntas:**

7. ¿Por qué el hijo usa `_exit` y no `exit`? Explica qué pasa con un `printf("hola")` sin `\n` que quedó en el buffer antes del `fork`.
8. ¿Qué pasa si el hijo hace `return 0` desde `main`? ¿Y si el hijo está dentro de otra función y hace `return`?
9. Si el hijo modifica `params.mean`, ¿el padre ve el cambio? ¿Por qué?
10. ¿Qué es un proceso zombie y qué línea lo evita?

### 1.4 Plantilla mental del TODO 6

No la copies: **reconstrúyela tú** en orden, en una lista de pasos (sin código), antes de pasar a la Parte 2.

```
Tu respuesta (pasos del TODO 6):
1.
2.
...
```

---

## Parte 2. Ejercicios segmentados

Contexto común para los ejercicios 2.1 a 2.5:

```c
typedef struct {
    double *measurements;   // arreglo dinámico de n mediciones
    size_t n;
    double mean, min, max;  // salidas
} Params;

typedef struct {
    double mean, min, max;  // lo que viaja por el pipe (sin punteros)
} Result;

void *fill_measurements(void *args);  // llena p->measurements[0..n-1]
void *compute_stats(void *args);      // calcula p->mean, p->min, p->max

int main(void) {
    size_t n = 1000;
    double *measurements = malloc(n * sizeof(double));
    if (measurements == NULL) { perror("malloc"); return 1; }
    Params params = { measurements, n, 0, 0, 0 };
    pthread_t t1, t2;
    int fd[2];
    // TODO 1 ... TODO 6
}
```

Firmas disponibles:

```c
int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                   void *(*start_routine)(void *), void *arg);
int pthread_join(pthread_t thread, void **retval);
int pipe(int pipefd[2]);
pid_t fork(void);
ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);
int close(int fd);
pid_t waitpid(pid_t pid, int *wstatus, int options);
void _exit(int status);
char *strerror(int errnum);
void perror(const char *s);
```

### 2.1 TODO 1 + 2

Crea `t1` que ejecute `fill_measurements` con `&params` y espéralo. Verifica el error de **cada** llamada; si algo falla, imprime el error, libera `measurements` y retorna 1.

```c
// Tu respuesta:
```

### 2.2 TODO 3 + 4

Lo mismo con `t2` y `compute_stats`. Debería salirte automático. Además responde: ¿por qué `t2` no puede crearse **antes** del join de `t1`?

```c
// Tu respuesta:
```

### 2.3 TODO 5

Crea el pipe con verificación de error. Responde: ¿por qué tiene que ir **antes** del `fork`?

```c
// Tu respuesta:
```

### 2.4 TODO 6a: sólo el hijo

Haz el `fork` (con su error) y escribe la rama del **hijo**: cerrar lo que no usa, armar un `Result` con los datos de `params`, `write` verificado, cerrar, `_exit` (¿con qué código si el `write` falló?).

```c
// Tu respuesta:
```

### 2.5 TODO 6b: sólo el padre

Escribe la rama del **padre**: cerrar lo que no usa, `read` verificado (incluye el caso de que lea menos bytes o 0), cerrar, `waitpid` verificado, imprimir `mean`, `min`, `max`, `free`. Ojo: si el `read` falla, igual tienes que hacer `waitpid` y `free`. ¿Por qué el `waitpid` también en el camino de error?

```c
// Tu respuesta:
```

### 2.6 Diagnóstico

Encuentra **todos** los errores. Para cada uno: línea, qué pasa en ejecución (el síntoma) y la corrección.

```c
 1  int main(void) {
 2      size_t n = 1000;
 3      double *measurements = malloc(n * sizeof(double));
 4      Params params = { measurements, n, 0, 0, 0 };
 5      pthread_t t1, t2;
 6      int fd[2];
 7
 8      if (pthread_create(&t1, NULL, fill_measurements(), &params) == -1) return 1;
 9      pthread_create(&t2, NULL, compute_stats, &params);
10      pthread_join(t1, NULL);
11      pthread_join(t2, NULL);
12
13      pid_t pid = fork();
14      pipe(fd);
15      if (pid == 0) {
16          Result r = { params.mean, params.min, params.max };
17          write(fd[1], &r, sizeof(r));
18          return 0;
19      }
20      Result r;
21      read(fd[0], &r, sizeof(r));
22      printf("%f %f %f\n", r.mean, r.min, r.max);
23      return 0;
24  }
```

```
Tu respuesta (línea → síntoma → corrección):
```

---

## Parte 3. Ejercicios tipo examen

### 3.1 Prueba 1 original (reconstruida): los 6 TODOs

> Tiempo sugerido: 30 min. Pauta: TODO 1–5 valen 1 cada uno, TODO 6 vale 2.

Completa `main`. El thread 1 llena el arreglo, el thread 2 calcula las estadísticas. Luego el proceso **hijo** envía un `Result` con las estadísticas al **padre** por un pipe, y el padre las imprime. Verifica errores y libera la memoria en todos los caminos.

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>

typedef struct {
    double *measurements;
    size_t n;
    double mean, min, max;
} Params;

typedef struct {
    double mean, min, max;
} Result;

void *fill_measurements(void *args);   // implementada
void *compute_stats(void *args);       // implementada

int main(void) {
    size_t n = 1000;
    double *measurements = malloc(n * sizeof(double));
    if (measurements == NULL) { perror("malloc"); return 1; }
    Params params = { measurements, n, 0, 0, 0 };
    pthread_t t1, t2;
    int fd[2];

    // TODO 1: crear t1 que ejecute fill_measurements

    // TODO 2: esperar a t1

    // TODO 3: crear t2 que ejecute compute_stats

    // TODO 4: esperar a t2

    // TODO 5: crear el pipe

    // TODO 6: fork; el hijo envía Result por el pipe y termina;
    //         el padre lo recibe, espera al hijo, imprime y libera

    return 0;
}
```

Preguntas de desarrollo (van con el código):

- (a) ¿Qué pasaría si el padre no cerrara `fd[1]` y el hijo muriera antes de escribir?
- (b) ¿Por qué el hijo termina con `_exit`?
- (c) ¿Por qué se envía un `Result` y no el `Params` completo?

```c
// Tu respuesta:
```

### 3.2 Variante A: el padre envía, el hijo calcula

Ahora el **padre** llena el arreglo con un thread (TODO 1–2), crea el pipe, hace `fork`, y **envía las `n` mediciones** por el pipe. El **hijo** las recibe en un arreglo propio (con `malloc`), calcula las estadísticas con un thread que ejecuta `compute_stats`, las imprime, libera y termina.

Condiciones:

- El padre envía primero `n` (un `size_t`) y después el arreglo. El hijo lee `n` y después **exactamente** `n * sizeof(double)` bytes.
- `read` puede retornar **menos bytes** de los pedidos en un pipe cuando el dato es grande. Escribe un ciclo `read_all(int fd, void *buf, size_t count)` que lea hasta completar `count` o retorne −1 si hay error o EOF prematuro.
- ¿Por qué ahora no basta un solo `read`, cuando con el `Result` de 24 bytes sí bastaba?

```c
ssize_t read_all(int fd, void *buf, size_t count);   // tú la implementas
```

```c
// Tu respuesta:
```

### 3.3 Variante B: dos hijos, dos pipes

El padre tiene `measurements` ya llenas. Crea **dos hijos**, cada uno con **su propio pipe**. El hijo 1 calcula `sum`, `min`, `max` de la primera mitad del arreglo y el hijo 2 de la segunda mitad; cada uno envía un `Partial { double sum, min, max; size_t count; }` al padre. El padre combina: `mean = (s1 + s2) / (c1 + c2)`, `min = min(m1, m2)`, `max = max(M1, M2)`.

Cuidado con:

- Qué descriptores tiene abiertos el **hijo 2** (¿hereda los del pipe 1?) y cuáles debe cerrar.
- Si falla el segundo `fork`, qué tiene que limpiar el padre (el primer hijo ya existe).

```c
// Tu respuesta:
```
