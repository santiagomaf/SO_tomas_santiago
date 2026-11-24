# 02. Mutex y variables de condición en C (Prueba 2)

> Peso en el examen: 1 de los 4 problemas de código. Formato típico: lista ligada compartida con `wait_for_key` / `insert_and_notify`; preguntas de por qué el `if` es incorrecto, contraejemplo de `signal` y versión corregida.

---

## Parte 1. Explicación

### 1.1 Mutex y sección crítica (SC)

```c
#include <pthread.h>

typedef struct node_t {
    int key;
    struct node_t *next;
} node_t;

typedef struct {
    node_t *head;
    pthread_mutex_t lock;
} list_t;

void List_Init(list_t *L) {
    L->head = NULL;
    pthread_mutex_init(&L->lock, NULL);   // o: = PTHREAD_MUTEX_INITIALIZER si es global
}
```

- `pthread_mutex_lock(&m)` bloquea al thread hasta obtener el lock; `pthread_mutex_unlock(&m)` lo suelta.
- **Dentro** de la SC va todo lo que lee o escribe **estado compartido**: `L->head`, los `next` de nodos ya enlazados, contadores compartidos.
- **Fuera** de la SC va lo que es **privado** del thread: `malloc` del nodo nuevo y su inicialización (`new->key = key`).
- **Bug clásico:** un `return` (o `break`, `goto`) dentro de la SC sin `unlock` deja el lock tomado para siempre.

**Preguntas:**

1. Da las **3 razones** para hacer el `malloc` fuera de la SC.
2. `new->next = L->head;` ¿va dentro o fuera de la SC? ¿Por qué, si `new` todavía es privado?
3. Muestra con una **intercalación paso a paso** (T1 y T2, instrucción por instrucción) cómo se pierde un nodo si dos threads hacen `Insert` sin lock.

### 1.2 Read-write lock

```c
pthread_rwlock_t rw = PTHREAD_RWLOCK_INITIALIZER;
pthread_rwlock_rdlock(&rw);   // varios lectores a la vez
pthread_rwlock_wrlock(&rw);   // un escritor, sin lectores
pthread_rwlock_unlock(&rw);   // suelta cualquiera de los dos
```

**Preguntas:**

4. ¿Cuándo conviene un rwlock sobre un mutex? ¿Cuándo es peor?
5. ¿Qué problema de **justicia** puede tener un rwlock con muchos lectores y un escritor?

### 1.3 Deadlock por orden de locks

```c
// Thread 1               // Thread 2
lock(&A);                 lock(&B);
lock(&B);                 lock(&A);
```

**Preguntas:**

6. Escribe la intercalación que produce el deadlock y nombra cuáles de las **4 condiciones** de Coffman se cumplen.
7. ¿Cómo lo corriges con un **orden global**? Si los locks son de dos cuentas bancarias `transfer(a, b)`, ¿qué usarías para ordenar?

### 1.4 Spinlock con `atomic_exchange`

```c
#include <stdatomic.h>
atomic_int flag = 0;

void spin_lock(atomic_int *f)   { while (atomic_exchange(f, 1) == 1) ; }
void spin_unlock(atomic_int *f) { atomic_store(f, 0); }
```

**Preguntas:**

8. Muestra con una intercalación de T1 y T2, ambos llamando `spin_lock` con `flag = 0`, por qué **sólo uno** entra. No basta decir "es atómico": muestra qué valor ve cada uno.
9. Escribe la versión **incorrecta** con `while (*f == 1) ; *f = 1;` y la intercalación que rompe la exclusión mutua.
10. ¿Por qué un spinlock es mala idea en un monoprocesador con una SC larga?

### 1.5 Variables de condición (CV)

```c
pthread_cond_t c = PTHREAD_COND_INITIALIZER;
pthread_cond_wait(&c, &m);     // requiere m tomado
pthread_cond_signal(&c);       // despierta a (al menos) uno
pthread_cond_broadcast(&c);    // despierta a todos
```

`pthread_cond_wait(&c, &m)`:

1. Requiere tener `m` tomado.
2. **Suelta `m` y duerme de forma atómica.**
3. Al despertar, **readquiere `m`** antes de retornar.

Patrones:

```c
// El que ESPERA                       // El que MODIFICA
lock(&m);                              lock(&m);
while (!predicado)                     /* cambiar el estado */
    pthread_cond_wait(&c, &m);         pthread_cond_signal(&c);  // o broadcast
/* usar el estado */                   unlock(&m);
unlock(&m);
```

**Preguntas:**

11. ¿Por qué el `wait` tiene que soltar el mutex **atómicamente** con el dormirse? Construye la intercalación del "despertar perdido" si fuera `unlock(&m); sleep();` en dos pasos.
12. Da **3 razones** distintas por las que se usa `while` y no `if` (incluye qué significa semántica **Mesa**).
13. ¿Por qué siempre se espera sobre un **predicado** (una variable de estado) y no sólo sobre la señal? ¿Qué pasa con un `signal` cuando nadie está esperando?
14. ¿Cuándo basta `signal` y cuándo hace falta `broadcast`? Da la regla general.

### 1.6 Padre espera hijo con `done`

```c
int done = 0;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  c = PTHREAD_COND_INITIALIZER;

void thr_exit(void) { /* tú */ }
void thr_join(void) { /* tú */ }
```

15. Escribe `thr_exit` y `thr_join` (se hace en la Parte 2, ejercicio 2.7).

---

## Parte 2. Ejercicios segmentados

Contexto común:

```c
typedef struct node_t { int key; struct node_t *next; } node_t;

typedef struct {
    node_t *head;
    pthread_mutex_t lock;
    pthread_cond_t  changed;   // se usa desde el ejercicio 2.4
} list_t;
```

Firmas disponibles:

```c
int pthread_mutex_lock(pthread_mutex_t *m);
int pthread_mutex_unlock(pthread_mutex_t *m);
int pthread_cond_wait(pthread_cond_t *c, pthread_mutex_t *m);
int pthread_cond_signal(pthread_cond_t *c);
int pthread_cond_broadcast(pthread_cond_t *c);
int pthread_rwlock_rdlock(pthread_rwlock_t *rw);
int pthread_rwlock_wrlock(pthread_rwlock_t *rw);
int pthread_rwlock_unlock(pthread_rwlock_t *rw);
void *malloc(size_t size);
```

### 2.1 `Insert` thread-safe

Inserta al inicio de la lista. Retorna 0 si OK y −1 si falla `malloc`. SC mínima.

```c
int List_Insert(list_t *L, int key);
```

```c
// Tu respuesta:
```

### 2.2 Encuentra el bug (pendiente de una sesión anterior)

```c
int List_Insert(list_t *L, int key) {
    pthread_mutex_lock(&L->lock);
    node_t *new = malloc(sizeof(node_t));
    if (new == NULL) {
        perror("malloc");
        return -1;
    }
    new->key = key;
    new->next = L->head;
    L->head = new;
    pthread_mutex_unlock(&L->lock);
    return 0;
}
```

Responde: (a) cuál es el bug, (b) qué le pasa **al siguiente thread** que llame a `Insert` o `Lookup` y por qué, (c) dos formas de corregirlo y cuál es mejor.

```
Tu respuesta:
```

### 2.3 `Lookup` con rwlock

Cambia `lock` por un `pthread_rwlock_t rw`. Escribe `Lookup` (retorna 1 si encuentra `key`, 0 si no) con modo lectura, e `Insert` con modo escritura. Cuidado con el `return` dentro del ciclo de `Lookup`.

```c
int List_Lookup(list_t *L, int key);
int List_Insert(list_t *L, int key);
```

```c
// Tu respuesta:
```

### 2.4 `wait_for_key`

Bloquea al thread que llama hasta que `key` esté en la lista. Puedes usar `int lookup_unsafe(list_t *L, int key)` (recorre sin tomar locks; el que la llama debe tener el lock).

```c
void wait_for_key(list_t *L, int key);
```

```c
// Tu respuesta:
```

### 2.5 `insert_and_notify` + contraejemplo de `signal`

Escribe `insert_and_notify(L, key)`: inserta y avisa a los que esperan. Después escribe el **contraejemplo con 2 consumidores** que muestra por qué `signal` falla: tabla con columnas `T1 | T2 | T3 | estado de la lista | quién duerme`.

```c
int insert_and_notify(list_t *L, int key);
```

```
Tu respuesta (código + tabla del contraejemplo):
```

### 2.6 `wait_for_size` (pendiente de una sesión anterior)

Bloquea hasta que la lista tenga **al menos `n` nodos**. Usa `int count_unsafe(list_t *L)`. Modifica `insert_and_notify` si hace falta. Varios threads esperan con **distintos `n`**: ¿`signal` o `broadcast`? Justifica con un contraejemplo concreto (por ejemplo, T1 espera 3, T2 espera 1, la lista tiene 0 nodos).

```c
void wait_for_size(list_t *L, int n);
```

```
Tu respuesta:
```

### 2.7 `thr_exit` / `thr_join` con `done`

Escribe las dos funciones del punto 1.6. Después responde:

- (a) ¿Qué pasa si quitas `done` y el hijo corre **completo** antes de que el padre llame `thr_join`? Muestra la intercalación.
- (b) ¿Qué pasa si mantienes `done` pero quitas el mutex? Muestra la intercalación.

```c
// Tu respuesta:
```

---

## Parte 3. Ejercicios tipo examen

### 3.1 Prueba 2 original (reconstruida)

> Tiempo sugerido: 30 min.

Un equipo implementó lo siguiente sobre la lista compartida:

```c
void wait_for_key(list_t *L, int key) {
    pthread_mutex_lock(&L->lock);
    if (!lookup_unsafe(L, key))
        pthread_cond_wait(&L->changed, &L->lock);
    pthread_mutex_unlock(&L->lock);
}

int insert_and_notify(list_t *L, int key) {
    node_t *new = malloc(sizeof(node_t));
    if (new == NULL) return -1;
    new->key = key;
    pthread_mutex_lock(&L->lock);
    new->next = L->head;
    L->head = new;
    pthread_cond_signal(&L->changed);
    pthread_mutex_unlock(&L->lock);
    return 0;
}
```

Responde:

- **(a)** ¿Por qué el `if` en `wait_for_key` es incorrecto? Da un escenario concreto en que `wait_for_key(L, 7)` retorna **sin** que el 7 esté en la lista.
- **(b)** Aún cambiando `if` por `while`, `signal` es incorrecto. Construye el contraejemplo con **dos** threads esperando claves **distintas** y un tercero que inserta. Muestra paso a paso qué thread queda durmiendo para siempre.
- **(c)** Escribe la versión corregida de ambas funciones.

```
Tu respuesta:
```

### 3.2 Variante A: cola acotada productor-consumidor

Implementa una cola circular de capacidad `CAP` compartida por varios productores y varios consumidores, con **dos CVs** (`not_full`, `not_empty`).

```c
#define CAP 8
typedef struct {
    int buf[CAP];
    int head, tail, count;
    pthread_mutex_t m;
    pthread_cond_t not_full, not_empty;
} queue_t;

void queue_put(queue_t *q, int x);   // bloquea si count == CAP
int  queue_get(queue_t *q);          // bloquea si count == 0
```

Preguntas:

- (a) ¿Por qué con **dos** CVs aquí basta `signal`? ¿Qué pasaría si usaras **una sola** CV para ambas condiciones con `signal`? Da el contraejemplo (2 consumidores, 1 productor, `CAP = 1`).
- (b) ¿Por qué el `while` sigue siendo necesario aunque haya dos CVs?

```c
// Tu respuesta:
```

### 3.3 Variante B: barrera reutilizable de N threads

```c
typedef struct {
    int n;            // cuántos threads deben llegar
    int count;        // cuántos han llegado en esta ronda
    int generation;   // ronda actual
    pthread_mutex_t m;
    pthread_cond_t  c;
} barrier_t;

void barrier_wait(barrier_t *b);   // bloquea hasta que lleguen los n
```

- (a) Implementa `barrier_wait`. ¿`signal` o `broadcast`?
- (b) ¿Para qué sirve `generation`? Muestra qué falla si el predicado del `while` es sólo `count < n` y la barrera se usa en un ciclo (un thread rápido entra a la ronda 2 antes de que los demás salgan de la ronda 1).

```c
// Tu respuesta:
```
