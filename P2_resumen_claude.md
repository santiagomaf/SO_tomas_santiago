# Guía de Estudio — Prueba 2 (ICC4240/ICC3201 Sistemas Operativos y Redes)

> Basada en Clase 04, Clase 05, Clase 06 y Clase 07, con las exclusiones indicadas por el profesor:
> - Clase 4: **NO entran** Transacciones Atómicas ni Monitores (están al final del documento igual, para contexto, pero marcadas como EXCLUIDAS).
> - Clase 5: **NO entran** Grafos de Asignación de Recursos, Algoritmo del Banquero, Algoritmo del Grafo de Asignación de Recursos, Detección/Recuperación de bloqueos. El foco llega **hasta "Caracterización de un Bloqueo Mutuo"**.
> - Clase 6: sin exclusiones.
> - Clase 7: sin exclusiones (paginación, TLB, tablas jerárquicas, Pentium, memoria virtual — todo entra).

Complementado conceptualmente con el libro **Operating Systems: Three Easy Pieces (OSTEP)**, Arpaci-Dusseau (capítulos de concurrencia y memoria).

---

# PARTE 1 — CLASE 04: Procesos y Concurrencia #3

## 1.1 Procesos Cooperativos

**Resumen:** Un **proceso cooperativo** es aquel que puede verse afectado por otros procesos del sistema (a diferencia de un proceso independiente, que no comparte estado con nadie). Esto es habitual cuando los "procesos" son en realidad **threads**, ya que comparten el mismo espacio de memoria. El problema central: cuando varios procesos/threads acceden **concurrentemente** a los mismos datos, pueden producirse **inconsistencias**, porque la ejecución de los threads depende del *scheduler* del sistema operativo y **no es determinista** (no podemos predecir el orden exacto de ejecución de instrucciones de cada thread).

**Palabras clave:** proceso cooperativo, memoria compartida, no determinismo, planificación (scheduling).

---

## 1.2 Ejemplo: Productor-Consumidor (introducción al problema)

**Resumen:** Se usa el clásico ejemplo de un **productor** y un **consumidor** que comparten un **buffer acotado** (bounded buffer) y una variable `counter` que lleva la cuenta de cuántos elementos hay en el buffer.

- El productor mete elementos: cuando el buffer está lleno (`counter == BUFFER_SIZE`), espera (espera activa/ocupada, con un `while` vacío). Si hay espacio, agrega el ítem, avanza el índice `in` de forma circular (`% BUFFER_SIZE`) e incrementa `counter`.
- El consumidor saca elementos: cuando el buffer está vacío (`counter == 0`), espera. Si hay algo, saca el ítem, avanza `out` circularmente y decrementa `counter`.

**Explicación del ejemplo:** El libro/clase muestra que aunque cada programa (productor y consumidor) es correcto **por separado**, al ejecutarse **concurrentemente** pueden fallar, porque ambos modifican la misma variable compartida `counter` sin ningún mecanismo de protección.

**Palabras clave:** buffer acotado (bounded buffer), productor, consumidor, `counter`, espera activa (busy waiting).

---

## 1.3 El problema de `counter++` y `counter--` (condición de carrera en detalle)

**Resumen:** La instrucción de alto nivel `counter++` **no es atómica**: se traduce en varias instrucciones de máquina:

```
registro1 ← counter
registro1 ← registro1 + 1
counter ← registro1
```

Y análogamente `counter--`:
```
registro2 ← counter
registro2 ← registro2 - 1
counter ← registro2
```

**Explicación del ejemplo (traslape de ejecución):** Si el valor inicial de `counter` es 5, y el productor y consumidor se entrelazan así:

```
T0: productor  register1 = counter   → register1 = 5
T1: productor  register1 = register1 + 1  → register1 = 6
T2: consumidor register2 = counter   → register2 = 5
T3: consumidor register2 = register2 - 1  → register2 = 4
T4: productor  counter = register1   → counter = 6
T5: consumidor counter = register2   → counter = 4
```

El valor final de `counter` es **4**, pero el valor correcto debería ser **5** (un productor sumó 1 y un consumidor restó 1, así que debería quedar igual). Esto ocurre porque ambas operaciones leyeron el mismo valor antiguo (5) antes de que la otra terminara de escribir su resultado. **Este es el ejemplo canónico de condición de carrera** que hay que entender a fondo (el profesor lo pide explícitamente).

**Palabras clave:** instrucciones de máquina, traslape (interleaving), registro, condición de carrera.

---

## 1.4 Condición de Carrera (Race Condition)

**Resumen:** Se define formalmente: ocurre una **condición de carrera** cuando varios procesos acceden y manipulan un mismo dato compartido concurrentemente, y **el resultado final depende del orden particular en que se intercalen los accesos**. La solución: garantizar que **solo un proceso a la vez** pueda manipular la variable compartida (es decir, **sincronizar**).

**Palabras clave:** condición de carrera (race condition), exclusión, sincronización.

---

## 1.5 El Problema de la Sección Crítica

**Resumen:** La **sección crítica** es el fragmento de código donde un proceso accede/modifica datos compartidos. El "problema de la sección crítica" consiste en diseñar un protocolo para que procesos concurrentes puedan "cooperar sin estorbarse". Cada proceso tiene una estructura:

```
do {
    entry section       // protocolo de entrada: pide permiso
    critical section     // sección crítica: usa el recurso compartido
    exit section        // protocolo de salida: libera el permiso
    remainder section    // resto del programa
} while (TRUE);
```

- **entry section**: código que solicita el acceso a la sección crítica.
- **exit section**: código que libera el acceso una vez terminada la sección crítica.
- **remainder section**: el resto del código del proceso, fuera de la sección crítica.

**Palabras clave:** sección crítica, entry section, exit section, remainder section.

---

## 1.6 Requisitos para una solución al problema de la S.C.

**Resumen:** Toda solución correcta debe cumplir **tres condiciones**:

1. **Exclusión mutua**: como máximo un proceso puede estar dentro de su sección crítica en un momento dado.
2. **Progreso**: si ningún proceso está en su sección crítica, y hay procesos que quieren entrar, la decisión de quién entra no puede posponerse indefinidamente por procesos que **no** están interesados en entrar (no puede haber un bloqueo artificial causado por procesos ajenos a la sección crítica).
3. **Espera acotada (no inanición)**: debe existir un límite en la cantidad de veces que **otros** procesos pueden pasar antes que un proceso que ya solicitó el acceso lo consiga. Es decir, nadie debe esperar "para siempre" (evitar la **inanición** o *starvation*).

Además, la solución debe ser **portable**: no puede depender de la velocidad de los procesadores, la cantidad de CPUs, ni de la política de scheduling.

**Palabras clave:** exclusión mutua, progreso, espera acotada, inanición (starvation), portabilidad.

---

## 1.7 Propiedades deseables de una solución (no obligatorias, pero valoradas)

**Resumen:**
- **Eficiencia**: el costo (overhead) de los protocolos de entrada/salida debe ser mínimo comparado con el trabajo útil que se hace dentro de la sección crítica.
- **Justicia (fairness)**: si varios procesos esperan, todos deben poder entrar con frecuencia similar (no siempre el mismo gana).
- **Escalabilidad**: la solución debe funcionar bien sin importar cuántos procesos compitan.
- **Simplicidad**: menor esfuerzo de implementación posible.

**Palabras clave:** eficiencia, justicia, escalabilidad, simplicidad.

---

## 1.8 Semáforos

**Resumen:** Un **semáforo** es una abstracción de alto nivel para sincronizar procesos/threads. Se modela como una **variable entera** `S` accedida solo mediante dos operaciones **atómicas**: `wait(S)` (también llamada `P()`, o en el libro de Arpaci-Dusseau `sem_wait()`) y `signal(S)` (también `V()`, o `sem_post()`).

Definición conceptual (con espera activa):
```c
wait(S) {
    while (S <= 0) ;  // espera
    S--;
}

signal(S) {
    S++;
}
```

**Palabras clave:** semáforo, `wait()`/`P()`/`sem_wait()`, `signal()`/`V()`/`sem_post()`, operación atómica.

---

## 1.9 Semáforos con espera ocupada (Spinlock)

**Resumen:** La definición anterior de semáforo usa **espera ocupada** (busy waiting): el proceso queda "girando" (`while S<=0`) consumiendo ciclos de CPU mientras espera. A este tipo de semáforo se le llama **spinlock**.

- **Ventaja:** si el tiempo de espera esperado es corto, vale la pena, porque evita el costo de un cambio de contexto (dormir/despertar el proceso).
- **Desventaja:** si el tiempo de espera es largo, se desperdicia CPU útil.
- **Uso típico:** en sistemas **multiprocesador** es común usar spinlocks, porque otro núcleo puede estar liberando el lock muy pronto.

**Palabras clave:** spinlock, espera ocupada (busy waiting), multiprocesador.

---

## 1.10 Semáforos Binarios (Mutex)

**Resumen:** Un **semáforo binario** o **mutex** solo puede tomar los valores 0 o 1. Se usa exactamente para resolver el problema de la sección crítica entre *n* procesos:

```c
do {
    wait(mutex);
    // sección crítica
    signal(mutex);
    // código restante
} while (true);
```

**Explicación:** Como `mutex` parte en 1, el primer proceso que llega hace `wait()` y lo deja en 0, entrando a la sección crítica. Cualquier otro proceso que intente `wait()` mientras el valor es 0 debe esperar (quedar bloqueado) hasta que el primero haga `signal()` y lo vuelva a 1.

**Palabras clave:** semáforo binario, mutex, exclusión mutua con semáforos.

---

## 1.11 Semáforos sin espera ocupada (con cola de bloqueados)

**Resumen:** Es posible implementar semáforos que **no** hagan espera activa, sino que **bloqueen** al proceso (lo pongan a "dormir", sin consumir CPU) en una lista/cola asociada al semáforo.

```c
typedef struct {
    int value;
    struct process *list;
} semaphore;

wait(semaphore *S) {
    S->value--;
    if (S->value < 0) {
        // agregar el proceso actual a S->list
        block();       // el proceso se bloquea (duerme)
    }
}

signal(semaphore *S) {
    S->value++;
    if (S->value <= 0) {
        // sacar un proceso P de S->list
        wakeup(P);      // se despierta a P
    }
}
```

**Explicación del ejemplo:** Aquí `value` puede volverse **negativo**: su valor absoluto (cuando es negativo) indica cuántos procesos están esperando/bloqueados en la lista del semáforo. Esto requiere soporte de la biblioteca de threads o del sistema operativo para las primitivas `block()` (bloquear/dormir) y `wakeup()` (despertar).

**Palabras clave:** `block()`, `wakeup()`, lista de procesos bloqueados, semáforo sin espera activa.

---

## 1.12 Usos indebidos de semáforos

**Resumen:** Los semáforos son propensos a errores de programación que **no se detectan en tiempo de compilación**, por ejemplo:

```c
signal(mutex);
// Sección crítica (¡sin haber hecho wait antes!)
wait(mutex);
```
o

```c
wait(mutex);
// Sección crítica
wait(mutex);     // ¡olvido de poner signal, o doble wait!
```

**Explicación:** Si se invierte el orden de `wait`/`signal`, se pierde la exclusión mutua (se entra a la sección crítica sin haber "pedido permiso"). Si se hace doble `wait()` sin un `signal()` intermedio, el proceso queda bloqueado para siempre (**deadlock**). Estos errores motivan el uso de abstracciones de más alto nivel, como **monitores y variables de condición** (esto último SÍ entra a la prueba; los monitores como constructo de lenguaje están excluidos, pero las variables de condición se estudian en clase 5).

**Palabras clave:** errores de sincronización, uso indebido de semáforos, abstracciones de alto nivel.

---

## 1.13 Actividad: Barreras (concepto)

**Resumen:** Una **N-barrera** es una primitiva de sincronización que bloquea a N-1 procesos hasta que el N-ésimo proceso llega a la barrera (es decir, todos deben "encontrarse" en el mismo punto antes de continuar). Se pedía implementarla usando semáforos, como ejercicio práctico.

**Palabras clave:** barrera (barrier), sincronización de grupo.

---

## 1.14 Problemas Clásicos de Sincronización

**Resumen:** Se presentan tres problemas históricos usados para poner a prueba mecanismos de sincronización: **Buffer Acotado**, **Lectores y Escritores**, y **Filósofos Comensales**. Más que memorizarlos literalmente, el profesor pide **entender el patrón general** para poder aplicarlo a problemas nuevos.

### 1.14.1 Problema del Buffer Acotado

**Resumen:** Hay un arreglo (pool) de *n* buffers. Se usan **tres variables de sincronización**:
- `mutex` (semáforo binario, inicializado en 1): exclusión mutua para acceder al pool.
- `empty` (inicializado en *n*): cuenta los espacios **vacíos** disponibles.
- `full` (inicializado en 0): cuenta los espacios **llenos** (con datos).

```c
// Productor
do {
    // producir un elemento
    wait(empty);
    wait(mutex);
    // agregar elemento al buffer
    signal(mutex);
    signal(full);
} while (true);

// Consumidor
do {
    wait(full);
    wait(mutex);
    // sacar un elemento del buffer
    signal(mutex);
    signal(empty);
    // consumir el elemento
} while (true);
```

**Explicación del ejemplo:** El orden de `wait(empty)` antes de `wait(mutex)` (y análogamente `wait(full)` antes de `wait(mutex)`) es crucial: si se invirtiera, se podría producir un **deadlock** (por ejemplo, un consumidor toma el mutex, luego intenta `wait(full)` y se bloquea porque no hay elementos, pero como tiene el mutex tomado, ningún productor puede entrar a llenar el buffer). El texto resalta: "el productor produce buffers llenos para el consumidor, y el consumidor produce buffers vacíos para el productor" — es decir, la relación es simétrica.

**Palabras clave:** buffer acotado, `empty`, `full`, `mutex`, orden de adquisición de semáforos.

### 1.14.2 Problema de los Lectores y Escritores

**Resumen:** Varios procesos comparten el acceso a una base de datos (o cualquier recurso). Hay **lectores** (solo leen) y **escritores** (modifican). Dos lectores simultáneos no generan problemas, pero un escritor **nunca** debe coexistir con ningún otro proceso (lector o escritor) accediendo al mismo tiempo. Existen variantes de priorización:

- **Lectores primero**: ningún lector espera a menos que un escritor **ya tenga** el acceso concedido. Puede causar **inanición de escritores**.
- **Escritores primero (lectores después)**: el escritor escribe lo antes posible; si un escritor espera, ningún lector nuevo puede entrar. Puede causar **inanición de lectores**.

**Solución "Lectores primero" mejorada (evita inanición), con variables compartidas:**
```c
semaphore mutex, wrt; // ambos inicializados en 1
int readcount;        // inicializado en 0
```
- `wrt`: exclusión mutua para escritores; también lo usa el **primer** lector que entra y el **último** lector que sale.
- `mutex`: protege la actualización de `readcount` (evita condición de carrera al incrementar/decrementar ese contador).
- `readcount`: cuántos lectores están leyendo actualmente.

```c
// Escritor
do {
    wait(wrt);
    // escritura
    signal(wrt);
} while (true);

// Lector
do {
    wait(mutex);
    readcount++;
    if (readcount == 1)
        wait(wrt);      // el primer lector "cierra la puerta" a escritores
    signal(mutex);
    // lectura
    wait(mutex);
    readcount--;
    if (readcount == 0)
        signal(wrt);    // el último lector "abre la puerta" de nuevo
    signal(mutex);
} while (true);
```

**Explicación del ejemplo:** La idea clave es que **solo el primer lector que llega** debe competir con los escritores por `wrt` (bloqueando a los escritores mientras haya al menos un lector activo), y **solo el último lector que se va** libera `wrt`. Los lectores intermedios no tocan `wrt` en absoluto, por lo que pueden entrar/salir libremente mientras haya lectura en curso. Si un escritor tiene la sección crítica y llegan *n* lectores, el primero de ellos queda esperando en `wrt`, y el resto queda esperando en `mutex`; cuando el escritor libera `wrt`, el scheduler decide si reanuda lectores o el siguiente escritor.

**Palabras clave:** lectores-escritores, `readcount`, `wrt`, inanición de escritores/lectores, prioridad.

### 1.14.3 Locks de lectura-escritura (Read-Write Locks)

**Resumen:** La solución anterior se **generaliza** en un tipo de lock llamado **lock lector-escritor** (read-write lock). Al adquirirlo, un proceso especifica el **modo**: lectura o escritura. **Múltiples** procesos pueden tener el lock en modo lectura simultáneamente, pero **solo uno** puede tenerlo en modo escritura (y de forma exclusiva, sin lectores). Es útil cuando es fácil distinguir qué procesos solo leen y cuáles escriben, especialmente si hay muchos más lectores que escritores (el mayor overhead del lock se compensa con la ganancia de permitir lectura concurrente).

**Palabras clave:** lock lector-escritor (reader-writer lock), modo lectura/escritura.

### 1.14.4 Problema de los Filósofos Comensales

**Resumen:** *N* filósofos (típicamente 5) se sientan en una mesa circular con un plato de arroz al centro y un palito (tenedor) entre cada par de filósofos adyacentes. El ciclo de vida de cada filósofo: **pensar → tener hambre → tomar palitos → comer → soltar palitos**. Reglas: toma un palito a la vez, y no puede tomar un palito que otro filósofo ya tomó.

**"No solución" (versión ingenua):**
```c
void philosopher(int i) {
    while (TRUE) {
        think();
        take_fork(i);
        take_fork((i+1) % N);
        eat();
        put_fork(i);
        put_fork((i+1) % N);
    }
}
```
**Explicación del problema:** Si **todos** los filósofos toman simultáneamente su palito izquierdo, después ninguno puede tomar el derecho (porque el vecino ya lo tiene) → **deadlock** total (espera circular).

**Solución con mutex global (funciona, pero ineficiente):**
```c
semaphore mutex = 1;
void philosopher(int i) {
    while (TRUE) {
        think();
        wait(&mutex);
        take_fork(i);
        take_fork((i+1) % N);
        eat();
        put_fork(i);
        put_fork((i+1) % N);
        signal(&mutex);
    }
}
```
**Explicación:** Se evita el deadlock porque solo un filósofo a la vez puede intentar tomar palitos. **Problema de performance:** solo **un** filósofo puede comer a la vez, aunque en teoría podrían comer hasta 2 simultáneamente sin conflicto (por ejemplo, filósofos no adyacentes). Es una solución **sub-óptima**.

**Solución optimizada (con arreglo de estados y un semáforo por filósofo):**
- Se define un arreglo `state[N]` con posibles valores `THINKING`, `HUNGRY`, `EATING`.
- Un `mutex` protege el acceso al arreglo de estados.
- Un arreglo de semáforos `s[N]`, uno por filósofo, usado para bloquear al filósofo si no puede comer todavía.
- La función `test(i)` revisa si el filósofo `i` puede pasar a `EATING` (si tiene hambre y ninguno de sus dos vecinos está `EATING`); si puede, cambia su estado y hace `signal()` sobre su propio semáforo para "despertarlo".
- `take_forks(i)`: marca al filósofo como `HUNGRY`, llama a `test(i)`, y si no logró comer, se bloquea en `wait(&s[i])`.
- `put_forks(i)`: marca al filósofo como `THINKING` y llama a `test()` sobre **ambos vecinos**, por si alguno de ellos ahora puede comer.

**Explicación del ejemplo:** Esta solución es más eficiente porque permite que coman simultáneamente todos los filósofos que no son vecinos entre sí, y evita el deadlock porque un filósofo solo pasa a `EATING` cuando **ambos** palitos (izquierdo y derecho) están disponibles a la vez (nunca toma "uno y espera el otro").

**Palabras clave:** filósofos comensales, deadlock por espera circular, `state[]`, `test()`, semáforo por proceso.

---

## 1.15 [EXCLUIDO DE LA PRUEBA] Transacciones Atómicas

> Esta sección **no entra** en la Prueba 2, según el mail del profesor. Se deja un resumen breve solo como referencia de contexto, por si aparece mencionada tangencialmente.

**Resumen (referencial, no evaluable):** Una transacción es un conjunto de instrucciones que forman una unidad lógica que debe ejecutarse en términos de "todo o nada" (por ejemplo, transferencias bancarias). Se manejan mediante bitácoras *write-ahead* (registro de valores antiguos/nuevos), operaciones `commit`/`abort`/`rollback`, tipos de almacenamiento (volátil, no volátil, estable), y la propiedad de "serializabilidad" para transacciones concurrentes.

## 1.16 [EXCLUIDO DE LA PRUEBA] Monitores

> Esta sección **no entra** en la Prueba 2 como constructo de lenguaje (aunque las **variables de condición** SÍ entran, y se profundizan en la Clase 5). Resumen breve referencial:

**Resumen (referencial, no evaluable):** Un monitor es un tipo de dato abstracto (o clase) donde todas las operaciones se ejecutan con exclusión mutua automática (el programador no debe implementar el `lock`/`unlock` manualmente). Usa **variables de condición** internas para sincronización adicional (`wait()`, `signal()`). Existe nativamente en Java, .NET, etc.

---

# PARTE 2 — CLASE 05: Procesos y Concurrencia #4

## 2.1 Locks

**Resumen:** Cualquier solución al problema de la sección crítica requiere una herramienta básica: el **lock** (cerrojo). Un proceso debe **adquirir** el lock antes de entrar a la sección crítica, y **liberarlo** al salir. El hardware ha evolucionado específicamente para dar soporte eficiente a la implementación de locks.

**Palabras clave:** lock, adquirir/liberar (acquire/release).

---

## 2.2 Hardware de Sincronización

**Resumen:** Una primera idea para implementar exclusión mutua es **deshabilitar las interrupciones** mientras se ejecuta la sección crítica, de modo que ninguna otra tarea pueda interrumpir la ejecución. Esto es lo que hacen internamente los **kernels no expropiativos**.

**Limitación importante:** esta técnica **no funciona en sistemas multiprocesador o distribuidos**, porque deshabilitar interrupciones en un solo procesador no impide que **otro procesador** siga ejecutando y accediendo a la variable compartida; habría que enviar un mensaje costoso a todos los procesadores. Por eso, en sistemas multiprocesador se necesitan **instrucciones atómicas de hardware** y/o **spinlocks**.

**Palabras clave:** deshabilitar interrupciones, kernel no expropiativo, multiprocesador, instrucciones atómicas.

---

## 2.3 Test-and-Set

**Resumen:** `TestAndSet` es una instrucción de máquina **atómica** que lee el valor actual de una posición de memoria y **al mismo tiempo** escribe un nuevo valor, en una sola operación indivisible (nadie puede "ver" un estado intermedio):

```c
bool TestAndSet(bool *target) {
    bool rv = *target;
    *target = true;
    return rv;
}
```

**Uso como spinlock:**
```c
bool lock = false;
while (true) {
    while (TestAndSet(&lock));  // gira mientras el lock esté ocupado
    // Sección crítica — exclusión mutua garantizada
    lock = false;
    // resto del código
}
```

**Explicación del ejemplo:** Cuando `lock` es `false` (libre), el primer proceso que ejecuta `TestAndSet` obtiene `rv = false` (sale del `while` interno) y deja `lock = true` (ocupado) atómicamente. Cualquier otro proceso que llame `TestAndSet` mientras tanto obtendrá `rv = true` y seguirá "girando" (spinning) hasta que el primero ponga `lock = false`.

**Palabras clave:** Test-and-Set, atomicidad de hardware, spinlock.

---

## 2.4 Swap (instrucción atómica alternativa)

**Resumen:** `Swap` intercambia atómicamente los valores de dos variables:

```c
boolean Swap(bool *a, bool *b) {
    bool temp = *a;
    *a = *b;
    *b = temp;
}
```

**Uso como spinlock:**
```c
while (true) {
    key = true;
    while (key == true) {
        Swap(&lock, &key);
    }
    // Sección crítica
    lock = false;
    // Código restante
}
```

**Explicación del ejemplo:** Análogo a `TestAndSet`: mientras `lock` esté en `true` (ocupado), cada `Swap` deja `key` en `true` y el proceso sigue girando. Cuando `lock` está en `false`, el `Swap` deja `key = false` (permitiendo salir del ciclo) y `lock = true` (tomando el candado).

**Palabras clave:** Swap, exclusión mutua con hardware.

---

## 2.5 Otras instrucciones de hardware: CompareAndSwap

**Resumen:** El libro de Arpaci-Dusseau (cap. 28) presenta otras instrucciones atómicas usadas para construir spinlocks: **CompareAndSwap** y **Load-Linked/Store-Conditional**.

```c
int CompareAndSwap(int *ptr, int expected, int new) {
    int actual = *ptr;
    if (actual == expected)
        *ptr = new;
    return actual;
}

void lock(lock_t *lock) {
    while (CompareAndSwap(&lock->flag, 0, 1) == 1)
        ; // spin
}
```

**Explicación del ejemplo:** `CompareAndSwap(ptr, expected, new)` compara atómicamente el valor actual en `ptr` con `expected`; si coinciden, lo reemplaza por `new`; siempre retorna el valor **que había antes** del intento. En `lock()`, se intenta cambiar `flag` de 0 (libre) a 1 (ocupado); mientras la llamada retorne 1 (ya estaba ocupado, no logró tomar el lock), sigue girando (`spin`).

**Palabras clave:** CompareAndSwap (CAS), Load-Linked/Store-Conditional.

---

## 2.6 Spinlock real en xv6 (ejemplo de sistema operativo real)

**Resumen:** Se muestra la implementación real del spinlock en **xv6** (SO educacional basado en Unix), usando la instrucción de ensamblador `xchg` (intercambio atómico) de x86.

```c
static inline uint xchg(volatile uint *addr, uint newval) {
    uint result;
    asm volatile("lock; xchgl %0, %1" :
        "+m" (*addr), "=a" (result) :
        "1" (newval) : "cc");
    return result;
}

void acquire(struct spinlock *lk) {
    pushcli(); // deshabilita interrupciones para evitar deadlock
    if (holding(lk))
        panic("acquire");
    while (xchg(&lk->locked, 1) != 0)
        ; // el xchg es atómico
}

void release(struct spinlock *lk) {
    asm volatile("movl $0, %0" : "+m" (lk->locked) : );
    popcli();
}
```

**Explicación del ejemplo:** `acquire()` deshabilita interrupciones en ese procesador (`pushcli`, evita que una interrupción reentre y cause deadlock) y luego gira (`while xchg(...) != 0`) intentando poner `locked` en 1 hasta lograrlo. `release()` simplemente pone `locked` en 0 (usando una instrucción atómica de escritura) y reactiva interrupciones (`popcli`). Es un ejemplo concreto de cómo un SO real implementa locks a bajo nivel con hardware.

**Palabras clave:** xv6, `xchg`, `acquire()`/`release()`, deshabilitar interrupciones dentro del kernel.

---

## 2.7 Evaluando una solución de locking

**Resumen:** Al evaluar cualquier mecanismo de locking, se consideran tres criterios:
1. **Exclusión mutua**: ¿se garantiza que solo un thread esté en la sección crítica?
2. **Justicia**: ¿todos los threads que compiten tienen igual oportunidad de obtener el lock cuando se libera? ¿puede algún thread quedar esperando indefinidamente?
3. **Rendimiento**: ¿cuánto overhead introduce el mecanismo?

**Palabras clave:** exclusión mutua, justicia (fairness), rendimiento (performance).

---

## 2.8 Inconveniente de TestAndSet/Swap: no garantizan espera acotada

**Resumen:** Los spinlocks basados en `TestAndSet` y `Swap` mostrados **no cumplen el criterio de espera acotada** (no garantizan ausencia de inanición): en el peor caso, un proceso podría "perder la carrera" por el lock indefinidamente frente a otros procesos más "afortunados" en el scheduling. Recordar la definición: la espera acotada exige un **límite** al número de veces que otros procesos pasan antes que uno que ya pidió el acceso lo consiga.

**Palabras clave:** espera acotada, inanición con spinlocks básicos.

---

## 2.9 Variables de Condición

**Resumen:** Una **variable de condición** es una **cola** que permite a un thread **avisar (señalizar)** a otros threads que **cambió** el estado del sistema; los threads que esperan ese cambio "duermen" en dicha cola (sin gastar CPU, a diferencia de la espera activa).

**Dos semánticas posibles:**
- **Semántica Mesa**: la señal solo **advierte** que el estado *podría* haber cambiado (no es garantía). Es la más simple y la que usan prácticamente todas las implementaciones reales (incluida **POSIX Threads / pthreads**).
- **Semántica Hoare**: la señal garantiza que el estado *efectivamente* cambió y que el thread despertado se ejecutará inmediatamente a continuación. Es más compleja de implementar.

**Tres operaciones básicas:**
- `wait`: el thread se bloquea (duerme) en la variable de condición.
- `signal`: despierta **a un solo** thread que esté esperando (si hay alguno).
- `broadcast`: despierta **a todos** los threads que estén esperando en esa variable de condición.

**Regla de oro (MUY IMPORTANTE, el profesor la resalta):** Bajo semántica Mesa, es **imprescindible** que cada `wait()` se llame **dentro de un loop `while`** que vuelva a chequear la condición al despertar, y **nunca** dentro de un `if`. Esto es porque, al usar semántica Mesa, cuando el thread despierta **no hay garantía** de que la condición que estaba esperando siga siendo verdadera (puede que otro thread se haya "colado" antes y haya vuelto a cambiar el estado). Por eso siempre se debe re-verificar la condición al despertar.

**Palabras clave:** variable de condición, semántica Mesa vs. Hoare, `wait`, `signal`, `broadcast`, re-chequeo de la condición (spurious wakeup).

---

## 2.10 Ejemplo: Padre espera por Hijo con Variable de Condición

**Resumen:** Ejemplo clásico de OSTEP (Fig. 30.3) donde un proceso padre debe esperar a que un hijo termine, usando una variable de condición junto con un mutex y una variable de estado `done`.

```c
int done = 0;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  c = PTHREAD_COND_INITIALIZER;

void thr_exit() {
    Pthread_mutex_lock(&m);
    done = 1;
    Pthread_cond_signal(&c);
    Pthread_mutex_unlock(&m);
}

void *child(void *arg) {
    printf("child\n");
    thr_exit();
    return NULL;
}

void thr_join() {
    Pthread_mutex_lock(&m);
    while (done == 0)
        Pthread_cond_wait(&c, &m);
    Pthread_mutex_unlock(&m);
}

int main() {
    printf("parent: begin\n");
    pthread_t p;
    Pthread_create(&p, NULL, child, NULL);
    thr_join();
    printf("parent: end\n");
    return 0;
}
```

**Explicación detallada del ejemplo (aquí es donde el profesor pide más profundidad):**
- Se usa una variable **compartida** `done` (inicializada en 0), **protegida siempre** por el mutex `m` cada vez que se lee o escribe.
- El hijo, al terminar, hace `thr_exit()`: toma el lock, pone `done = 1`, **señaliza** con `pthread_cond_signal(&c)` (avisa que "algo cambió"), y libera el lock.
- El padre, en `thr_join()`, toma el lock, y **mientras** `done == 0`, llama a `pthread_cond_wait(&c, &m)`.
- **Punto clave de `pthread_cond_wait()`**: esta función hace **dos cosas atómicamente**: (1) **libera el mutex** `m` (para que el hijo pueda entrar a modificar `done` y hacer `signal`), y (2) **pone a dormir** al thread llamante en la cola de la variable de condición `c`. Cuando el thread es despertado (por un `signal`/`broadcast`), **antes de retornar de `wait`**, **vuelve a adquirir el mutex** automáticamente. Esto evita una condición de carrera: si el padre liberara el mutex "a mano" y luego llamara a `wait` por separado, el hijo podría colarse justo en medio y hacer `signal` **antes** de que el padre esté realmente esperando, perdiéndose la señal para siempre (deadlock).
- **Por qué el `while` y no un `if`**: si en vez de `while (done == 0)` se usara `if (done == 0)`, y por alguna razón el thread despertara (bajo semántica Mesa) sin que `done` realmente valiera 1 todavía, el padre continuaría erróneamente. El `while` obliga a re-verificar la condición cada vez que se despierta.

**Palabras clave:** `pthread_cond_wait()`, liberación/readquisición atómica del mutex, `pthread_cond_signal()`, condición de carrera evitada por atomicidad de `wait`.

---

## 2.11 Ejemplo: Productor-Consumidor con Variables de Condición (solución final correcta)

**Resumen:** Esta es la versión **correcta y completa** del problema productor-consumidor usando variables de condición (Fig. 30.12 de OSTEP), con **dos** variables de condición separadas (`empty` y `fill`) y un `mutex`.

```c
cond_t empty, fill;
mutex_t mutex;

int buffer[MAX];
int fill_i = 0, use_i = 0, count = 0;

void put(int value) {
    buffer[fill_i] = value;
    fill_i = (fill_i + 1) % MAX;
    count++;
}

int get() {
    int tmp = buffer[use_i];
    use_i = (use_i + 1) % MAX;
    count--;
    return tmp;
}

void *producer(void *arg) {
    int i;
    for (i = 0; i < loops; i++) {
        Pthread_mutex_lock(&mutex);                  // p1
        while (count == MAX)                          // p2
            Pthread_cond_wait(&empty, &mutex);         // p3
        put(i);                                        // p4
        Pthread_cond_signal(&fill);                    // p5
        Pthread_mutex_unlock(&mutex);                  // p6
    }
}

void *consumer(void *arg) {
    int i;
    for (i = 0; i < loops; i++) {
        Pthread_mutex_lock(&mutex);                  // c1
        while (count == 0)                            // c2
            Pthread_cond_wait(&fill, &mutex);          // c3
        int tmp = get();                               // c4
        Pthread_cond_signal(&empty);                   // c5
        Pthread_mutex_unlock(&mutex);                  // c6
        printf("%d\n", tmp);
    }
}
```

**Explicación detallada del ejemplo (clave para la Parte 2 del examen, que pide analizar/corregir código concurrente):**
- **Por qué DOS variables de condición** (`empty` y `fill`) y no una sola: el productor debe esperar cuando el buffer está **lleno** (`count == MAX`), señalizando en `empty` cuando saca espacio... espera, en realidad: el productor **espera** en `empty` (que se le avise que "hay espacio vacío") y **señaliza** en `fill` (avisando "hay algo lleno para consumir"). El consumidor hace lo simétrico: **espera** en `fill` y **señaliza** en `empty`. Usar una sola variable de condición para ambos roles sería un **error clásico** (versión errónea que aparece en OSTEP cap. 30) porque un `signal()` podría despertar a un thread del **mismo tipo** que no puede hacer nada útil todavía (por ejemplo, el productor señaliza y despierta a otro productor en vez de a un consumidor), lo cual en el peor caso lleva a deadlock con solo 2 threads y espera activa que no soluciona el problema con más de 2 threads.
- **`while` en vez de `if`** (p2/p3 y c2/c3): fundamental para no asumir que la condición se mantiene tras despertar (semántica Mesa). Ejemplo de falla si fuera `if`: dos consumidores esperan porque `count == 0`; llega un productor y hace `put()` + `signal(&fill)` (despierta solo a **uno**); pero si por scheduling **ambos** consumidores llegaran a ejecutar tras un handling con `if`, uno leería un buffer vacío inválido.
- **Un solo `mutex` compartido** entre productor y consumidor: protege la variable compartida `count` (y el arreglo `buffer`) para que su lectura/escritura sea atómica, evitando la condición de carrera vista en la sección 1.3 de esta guía.
- **Patrón general a aprender (más importante que memorizar el código):** (1) proteger todo acceso a estado compartido con un mutex; (2) usar una variable de condición por cada "razón de espera" distinta; (3) siempre re-chequear la condición en un `while` tras el `wait`; (4) señalizar después de modificar el estado, antes de soltar el mutex (o justo antes, en general está bien hacerlo con el mutex tomado o inmediatamente después).

**Palabras clave:** `empty`, `fill`, `count`, patrón productor-consumidor con dos variables de condición, mutex compartido.

---

## 2.12 Semáforos implementados con variables de condición y locks

**Resumen:** Se muestra que un semáforo (llamado aquí `Zem_t`, "zemaphore") puede **construirse** a partir de un mutex y una variable de condición (demostrando que las variables de condición son un mecanismo más primitivo/fundamental):

```c
typedef struct __Zem_t {
    int value;
    pthread_cond_t cond;
    pthread_mutex_t lock;
} Zem_t;

void Zem_init(Zem_t *s, int value) {
    s->value = value;
    Cond_init(&s->cond);
    Mutex_init(&s->lock);
}

void Zem_wait(Zem_t *s) {
    Mutex_lock(&s->lock);
    while (s->value <= 0)
        Cond_wait(&s->cond, &s->lock);
    s->value--;
    Mutex_unlock(&s->lock);
}

void Zem_post(Zem_t *s) {
    Mutex_lock(&s->lock);
    s->value++;
    Cond_signal(&s->cond);
    Mutex_unlock(&s->lock);
}
```

**Explicación del ejemplo:** `Zem_wait` es análogo a `wait()`/`P()` de un semáforo: mientras el valor sea ≤ 0, el thread se bloquea en la variable de condición (dentro de un `while`, por la misma razón de siempre). `Zem_post` es análogo a `signal()`/`V()`: incrementa el valor y señaliza para despertar a algún thread esperando. Esto demuestra que **mutex + variable de condición** son "suficientes" para construir cualquier otra primitiva de sincronización de más alto nivel, como el semáforo.

**Palabras clave:** semáforo implementado con CV+mutex, primitivas fundamentales de sincronización.

---

## 2.13 Estructuras de Datos Sincronizadas

**Resumen:** Se revisan cuatro estructuras de datos clásicas y cómo hacerlas *thread-safe* (seguras para acceso concurrente): **Contadores**, **Lista Ligada**, **Cola**, y **Tabla Hash**. El hilo conductor es: ¿cómo lograr **exclusión mutua** sin sacrificar demasiado el **rendimiento** (escalabilidad a múltiples CPUs)?

### 2.13.1 Contadores (Counters)

**Resumen:** Un contador sincronizado simple usa un `lock` para proteger el incremento/decremento:

```c
typedef struct __counter_t {
    int value;
    pthread_mutex_t lock;
} counter_t;

void increment(counter_t *c) {
    Pthread_mutex_lock(&c->lock);
    c->value++;
    Pthread_mutex_unlock(&c->lock);
}
```

**Problema de performance:** con **un solo procesador**, el impacto es menor. Pero con **múltiples procesadores**, este contador **no escala**: mientras un thread en un CPU incrementa el contador (con el lock tomado), **ningún** thread en **otro** CPU puede hacerlo, aunque físicamente podrían ejecutar en paralelo. El tiempo total crece con la cantidad de threads/CPUs en vez de mantenerse constante.

**Solución: "Sloppy Counters" (contadores descuidados/aproximados):**
- Se define un contador **global** `G` y un contador **local por CPU** `C_i` (cada uno con su propio lock).
- Cada thread incrementa el contador **local** de su CPU (`C_i`), lo cual genera **menos competencia** (cada CPU tiene su propio lock, sin interferir con otros CPUs).
- Cuando el contador local `C_i` acumula un umbral `S` (threshold), el thread transfiere ese valor al contador global `G` (con su propio lock) y resetea `C_i` a 0.
- **Trade-off (la pregunta que hace el profesor: "¿cuál es la desventaja?"):** el valor de `G` en un instante dado es solo una **aproximación**, no el conteo exacto en tiempo real (porque hay incrementos "guardados" en los `C_i` que aún no se han sumado a `G`). Mientras **más grande** sea `S` (el umbral), **mejor escala** el contador (menos actualizaciones a `G`) pero **más impreciso** es en un momento dado; mientras **más pequeño** sea `S`, más se parece en comportamiento (y en costo) al contador sincronizado simple.

**Explicación del ejemplo (tabla de trazas):** Se muestra una tabla con columnas `Time, L1, L2, L3, L4, G` donde `L_i` son los contadores locales de 4 CPUs. Se ve cómo cada `L_i` va subiendo independientemente, y cuando alguno llega al umbral `S` (en el ejemplo, cuando `L1` llega a 5 en el tiempo 6, y `L4` llega a 5 en el tiempo 7), se transfiere a `G` y ese local vuelve a 0. Esto ilustra visualmente cómo `G` "salta" en incrementos de tamaño `S` en vez de aumentar de a uno.

**Palabras clave:** contador sincronizado, sloppy counters, contador afín (per-CPU), umbral (threshold) `S`, escalabilidad vs. precisión.

### 2.13.2 Listas Ligadas (Linked Lists)

**Resumen:** Estructura compuesta por nodos con clave y puntero al siguiente. Operaciones típicas: **insertar** y **buscar**. Si hay varios threads (lectores y al menos un escritor) accediendo concurrentemente, hay condición de carrera al leer/modificar punteros.

- **Idea simple**: un solo lock global protege toda la lista (la convierte en un "monitor"). **Problema**: solo un thread puede acceder a la vez, y las operaciones de búsqueda/inserción son de **tiempo lineal** respecto al tamaño de la lista → mal rendimiento con muchos threads.
- **Mejoras posibles:**
  - **Lock lector-escritor**: permite múltiples lectores simultáneos, un solo escritor exclusivo (hace la lista *thread-safe* con mejor rendimiento en cargas con predominio de lectura).
  - **Hand-over-hand locking**: un lock por cada nodo (se toma el lock del nodo siguiente antes de soltar el del actual, al recorrer la lista). **Problema**: no es tan buena idea si varios threads recorren la lista simultáneamente (el overhead de tomar/soltar locks nodo a nodo puede ser mayor que el beneficio).
  - **Particionar la lista**: poner un lock lector-escritor cada N nodos. Es una solución más robusta: los lectores pueden recorrer rápido si no hay escritores activos en esa partición.

**Palabras clave:** lista ligada, lock global, lock lector-escritor, hand-over-hand locking, particionamiento de locks.

### 2.13.3 Cola Sincronizada (Queue)

**Resumen:** Estructura FIFO con nodo `head` (cabeza) y `tail` (apéndice/cola). Operaciones: `enqueue` (inserta al final) y `dequeue` (extrae desde el inicio).

- Se implementa con **dos locks separados**: uno para modificar la cabeza (`headLock`) y otro para el apéndice (`tailLock`).
- Se inicializa con un **nodo dummy** (ficticio) que actúa como cabeza y cola a la vez al inicio, para simplificar el manejo de casos borde (cola vacía).

**Explicación del ejemplo (Michael and Scott Concurrent Queue):** Al tener locks separados para `head` y `tail`, un `enqueue` (que solo toca el `tail`) y un `dequeue` (que solo toca el `head`) pueden ejecutarse **en paralelo** sin bloquearse mutuamente, siempre que la cola tenga más de un elemento (gracias al nodo dummy). Esto mejora notablemente la concurrencia comparado con un solo lock global.

**Palabras clave:** cola FIFO, `head`/`tail` lock, nodo dummy, Michael-Scott queue.

### 2.13.4 Tablas Hash (Hash Tables)

**Resumen:** Se simplifica asumiendo una tabla hash que **no requiere redimensionarse**. Se implementa de la forma usual: cada "bucket" (cubeta, para las claves que colisionan al mismo hash) es una **lista ligada**, y se pone **un lock por cada lista ligada/bucket** (no un solo lock global para toda la tabla).

**Explicación del ejemplo (gráfico de escalamiento):** El gráfico comparado ("Simple Concurrent List" vs. "Concurrent Hash Table") muestra que la tabla hash con un lock por bucket **escala mucho mejor** (tiempo casi constante al aumentar la cantidad de inserciones) que una lista simple con un solo lock (tiempo crece linealmente), porque distintos threads que insertan en distintos buckets **no compiten entre sí**.

**Palabras clave:** tabla hash concurrente, lock por bucket, escalabilidad comprobada empíricamente.

---

## 2.14 Caracterización de Bloqueos Mutuos (Deadlocks) — HASTA AQUÍ LLEGA CLASE 5 EN LA PRUEBA

### 2.14.1 Procesos y Recursos

**Resumen:** Un sistema tiene un conjunto finito de **recursos** repartidos en clases: memoria, ciclos de CPU, núcleos de CPU, archivos, dispositivos de E/S, etc. Si un proceso pide un recurso de cierta clase, **cualquier instancia** de esa clase debe poder satisfacer el requerimiento (si no, la clase está mal definida).

**Palabras clave:** clases de recursos, instancias de recurso.

### 2.14.2 Patrón de Utilización de Recursos

**Resumen:** El ciclo de vida típico del uso de un recurso por un proceso:
1. **Solicitud**: el proceso pide el recurso; si no está disponible, espera.
2. **Uso**: el proceso opera con el recurso.
3. **Liberación**: el proceso deja de usarlo, quedando disponible para otros.

**Palabras clave:** solicitud, uso, liberación de recursos.

### 2.14.3 Caracterización de un Bloqueo Mutuo (Deadlock) — CONCEPTO CENTRAL

**Resumen:** En un **bloqueo mutuo (deadlock)**, los procesos involucrados **nunca terminan** de ejecutar, y los recursos que tienen tomados **nunca se liberan**, por lo que tampoco pueden iniciarse procesos nuevos que los necesiten. Existen **cuatro condiciones necesarias** que deben cumplirse **simultáneamente** para que ocurra un deadlock (el profesor pide estudiar esto con especial atención):

1. **Exclusión mutua**: al menos un recurso debe ser de uso exclusivo (no compartible); si varios procesos lo requieren, solo uno puede usarlo a la vez.
2. **Toma y espera (hold and wait)**: un proceso debe **tener** al menos un recurso en su poder mientras **espera** por obtener uno o más recursos adicionales que están en poder de otros procesos.
3. **No expropiación (no preemption)**: los recursos no pueden ser arrebatados por la fuerza; solo se liberan **voluntariamente** cuando el proceso que los tiene los suelta.
4. **Espera circular (circular wait)**: debe existir un conjunto de procesos {P0, P1, ..., Pn} tal que P0 espera un recurso que tiene P1, P1 espera uno que tiene P2, ..., Pn-1 espera uno que tiene Pn, y Pn espera uno que tiene P0 (forman un "círculo" de esperas mutuas).

**Punto clave:** las **cuatro condiciones deben darse a la vez** para que exista deadlock; si se logra **romper aunque sea una sola** de ellas (mediante prevención), el deadlock no puede ocurrir. Esta es la base conceptual de las estrategias de prevención de deadlocks (que en las diapositivas siguen después de la diapositiva 23, pero que **quedan excluidas** de esta prueba según el mail del profesor — grafos de asignación de recursos, algoritmo del banquero, detección y recuperación).

**Palabras clave (las más importantes de toda la clase 5, según el mail del profesor):** deadlock, exclusión mutua, toma y espera (hold and wait), no expropiación (no preemption), espera circular (circular wait).

---

# PARTE 3 — CLASE 06: Administración de Memoria #1

## 3.1 Hoy Veremos (contenidos)

**Resumen:** La clase cubre: hardware de memoria principal, espacios de direcciones, direccionamiento físico y lógico, asociación (binding) de direcciones, carga dinámica, asignación de memoria y segmentación.

---

## 3.2 Ciclo de ejecución de un programa

**Resumen:** Todo programa se ejecuta mediante el ciclo clásico **Fetch → Decode → Execute**:
- **Fetch**: buscar la siguiente instrucción en memoria principal.
- **Decode**: decodificar/preparar la instrucción para su ejecución.
- **Execute**: ejecutar la instrucción.

Se muestra también el diagrama de la CPU con sus registros: **PC** (Program Counter, apunta a la próxima instrucción), **IR** (Instruction Register, contiene la instrucción actual), **MAR** (Memory Address Register, dirección de memoria a acceder), **MBR** (Memory Buffer Register, dato leído/escrito), **I/O AR** e **I/O BR** (análogos para dispositivos de E/S), conectados a la memoria principal vía el bus del sistema.

**Palabras clave:** Fetch-Decode-Execute, PC, IR, MAR, MBR, bus del sistema.

---

## 3.3 Hardware de memoria básico

**Resumen:** La CPU **solo** puede acceder directamente a la **memoria principal** y a sus propios **registros**; **no** puede acceder directamente al almacenamiento secundario (disco). Si un dato está en disco, primero debe copiarse a memoria principal.

- **Registros de CPU**: memoria extremadamente rápida, accesible en un solo ciclo de reloj.
- **Memoria dinámica (DRAM)**: requiere pasar por el **bus**, por lo que puede tomar **varios ciclos de CPU**.

**Dos problemas centrales de la memoria principal:**
1. **Diferencial de velocidad de acceso**: puede causar que la CPU quede en estado **stalled** (paralizada, esperando datos) porque un acceso a memoria (fuera de los registros) toma varios ciclos. **Solución**: usar una memoria estática rápida intermedia, la **caché**, que mantiene datos "a la mano".
2. **Protección de memoria**: hay que impedir que un proceso de usuario acceda o modifique memoria de otro proceso o del kernel.

**Palabras clave:** registros de CPU, DRAM, bus, estado *stalled*, memoria caché, protección de memoria.

---

## 3.4 Protección de Memoria: Registros Base y Límite

**Resumen:** Con **multiprogramación**, se necesita proteger tanto el **espacio del kernel** (para que procesos de usuario no lo modifiquen) como el espacio de cada proceso de usuario respecto de los demás. Para lograrlo, cada proceso necesita un rango de direcciones válidas, definido mediante **dos registros**:

- **Registro base**: dirección física donde empieza el espacio del proceso.
- **Registro límite**: rango de direcciones lógicas válidas (tamaño del espacio del proceso).

**Ejemplo numérico:** Registro base = 300040, registro límite = 120900. Esto significa que las direcciones físicas válidas para ese proceso van desde 300040 hasta 300040+120900 = 420940.

**Mecanismo de protección (hardware):** cuando la CPU genera una dirección lógica, el hardware **compara** esa dirección con el registro límite: si la dirección lógica es **menor** que el límite, se **suma** el registro base a la dirección lógica para obtener la dirección física, y se accede a memoria; si la dirección lógica es **mayor o igual** al límite, se genera un **trap** (error de direccionamiento), evitando el acceso indebido.

**Importante:** los registros base y límite **solo pueden modificarse mediante instrucciones privilegiadas** (modo kernel), evitando que un proceso de usuario altere su propio rango de acceso.

**Palabras clave:** registro base, registro límite, trap de direccionamiento, instrucciones privilegiadas, modo kernel.

---

## 3.5 Espacio Lógico vs. Físico — la MMU

**Resumen:** En sistemas operativos y arquitecturas modernas, cada proceso de usuario ve un espacio de direcciones **contiguo, exclusivo y grande** (mayor incluso que la memoria física real disponible). Esto requiere un proceso de **traducción de direcciones**:

- **Dirección lógica (o virtual)**: la que genera la CPU al ejecutar el código del proceso.
- **Dirección física**: la dirección real en la memoria RAM, la que "ve" la MMU (no la CPU directamente).

La **MMU (Memory Management Unit)** es el dispositivo de hardware encargado de traducir direcciones lógicas en físicas. Contiene un **registro de reubicación (relocation register)** cuyo valor se **suma** a toda dirección lógica generada por un proceso de usuario para obtener la dirección física correspondiente. Los programas de usuario **siempre** manipulan direcciones lógicas; nunca "ven" las direcciones físicas reales.

**Explicación del ejemplo numérico:** Si el registro de reubicación tiene el valor 14000, y la CPU genera la dirección lógica 346, la MMU calcula la dirección física como 14000 + 346 = **14346**.

**Palabras clave:** MMU, dirección lógica/virtual, dirección física, registro de reubicación (relocation register).

---

## 3.6 Direcciones de Memoria en Programas

**Resumen:** Inicialmente, un programa reside en disco como un **archivo binario ejecutable**. Para ejecutarse, debe cargarse en memoria física. No necesariamente la primera dirección del programa será la dirección 0. La **asociación (binding)** entre las instrucciones/datos del programa y direcciones concretas de memoria puede hacerse de distintas maneras y en distintos momentos.

**Palabras clave:** archivo binario ejecutable, asociación de direcciones (address binding).

---

## 3.7 Procesamiento paso a paso de un programa de usuario

**Resumen:** Se ilustra el flujo completo desde el código fuente hasta la ejecución:

`source program → (compiler/assembler, "compile time") → object module → (linkage editor, junto a otros módulos objeto, "load time") → load module → (loader, junto a bibliotecas del sistema) → in-memory binary memory image ("execution time" / run time)`

También existe la posibilidad de **enlace dinámico** (*dynamic linking*), donde bibliotecas del sistema se cargan dinámicamente en tiempo de ejecución en vez de en tiempo de carga.

**Palabras clave:** compilador/ensamblador, módulo objeto, editor de enlaces (linkage editor), loader, imagen binaria en memoria.

---

## 3.8 Formas de Asociación de Direcciones (Address Binding)

**Resumen:** Existen tres momentos posibles para fijar la asociación entre instrucciones/datos y direcciones de memoria:

1. **En tiempo de compilación**: se sabe de antemano exactamente en qué direcciones residirá el proceso; el compilador genera **direcciones absolutas** directamente.
2. **En tiempo de carga**: si no se sabe en compilación dónde residirá el proceso, el compilador genera código **"relocalizable"** (con direcciones relativas); la asociación definitiva se realiza recién cuando el programa se **carga** en memoria.
3. **En tiempo de ejecución**: el proceso puede **moverse** de un segmento de memoria a otro incluso mientras está corriendo; requiere **hardware especial** (como la MMU con su registro de reubicación) para hacer la traducción "al vuelo" en cada acceso.

**Palabras clave:** binding en tiempo de compilación, binding en tiempo de carga, binding en tiempo de ejecución, código relocalizable.

---

## 3.9 Carga Dinámica (Dynamic Loading)

**Resumen:** Con **carga dinámica**, las rutinas de un programa se cargan en memoria **solo cuando se necesitan** (no todas de una vez al inicio). Todas las rutinas se mantienen en disco en formato relocalizable. Es útil cuando se usan bibliotecas muy grandes de las que solo se necesita usar una fracción del código, optimizando el uso de memoria. **No requiere soporte especial del sistema operativo**: es responsabilidad del **programador** diseñar el programa para aprovechar esta técnica (aunque el SO puede facilitar algunas funciones de apoyo).

**Palabras clave:** carga dinámica, rutinas cargadas bajo demanda, responsabilidad del programador.

---

## 3.10 Enlace Dinámico y Bibliotecas Compartidas

**Resumen:** Los programas generalmente dependen de bibliotecas (ej: biblioteca estándar de C/C++, llamadas al sistema, etc.).

- **Enlace estático**: al cargar el programa, las bibliotecas de las que depende se **combinan** con el binario del programa por el *loader*, y todo se carga junto en memoria.
- **Enlace dinámico**: el enlace con las bibliotecas se **posterga hasta tiempo de ejecución** (análogo a la carga dinámica de rutinas). Se usa el mecanismo de **stub**: un pequeño fragmento de código que referencia la rutina real de la biblioteca; si al ejecutarse el stub la rutina correspondiente aún no está en memoria, **se carga en ese momento**.
- **Ventaja clave**: el código de una biblioteca cargado en memoria puede ser **compartido** por distintos procesos que la necesiten simultáneamente (ahorro de memoria), y el mecanismo permite que **coexistan varias versiones** de una misma biblioteca en memoria, accedidas por distintos programas según cuál necesiten.

**Palabras clave:** enlace estático vs. dinámico, stub, biblioteca compartida, múltiples versiones de biblioteca.

---

## 3.11 Asignación de Memoria (Memory Allocation)

**Resumen:** En la memoria principal deben convivir el sistema operativo y los procesos de usuario. Generalmente la memoria se divide en dos particiones: una para el SO y otra para los procesos. El SO puede residir en "memoria baja" o "memoria alta" (según dónde esté el vector de interrupciones). Con multiprogramación, se busca mantener varios procesos de usuario en memoria simultáneamente, lo cual exige decidir **cómo** repartir la memoria disponible entre los programas que se van cargando. El mecanismo más común: **asignación contigua de memoria**.

**Palabras clave:** partición SO/usuario, memoria baja/alta, vector de interrupciones, asignación contigua.

---

## 3.12 Asignación Contigua de Memoria

**Resumen:** Dos variantes principales:

### 3.12.1 Particiones fijas (método de particiones múltiples)
- Se divide la memoria en varias particiones de **tamaño fijo**; cada partición aloja exactamente un proceso.
- El **grado de multiprogramación** posible queda limitado por la cantidad de particiones disponibles.
- Cuando un proceso termina, libera su partición, permitiendo cargar otro proceso ahí.
- Fue usado originalmente por **IBM OS/360**.

### 3.12.2 Particiones variables
- El SO mantiene una **tabla** con las porciones de memoria libres ("hoyos"/*holes*) y ocupadas.
- Inicialmente toda la memoria disponible para usuarios es un solo "hoyo" grande.
- A medida que los procesos se cargan y liberan, la tabla se va actualizando.
- Cuando ningún hoyo disponible es suficientemente grande para un nuevo proceso, este debe **esperar** a que se libere memoria.

**Palabras clave:** particiones fijas, particiones variables, "hoyo" (hole), IBM OS/360.

---

## 3.13 Estrategias de Asignación Dinámica: Primer/Mejor/Peor Calce

**Resumen:** Frente al problema "¿cómo satisfacer un requerimiento de tamaño *n* a partir de la lista de hoyos libres?", existen tres estrategias clásicas:

- **Primer calce (First Fit)**: asigna el **primer** hoyo disponible que sea suficientemente grande. La búsqueda puede empezar desde el inicio, el final, o donde terminó la búsqueda anterior. Se detiene apenas encuentra un hoyo que sirve.
- **Mejor calce (Best Fit)**: asigna el hoyo **más pequeño** que aún sea suficientemente grande. Requiere revisar **toda** la lista (a menos que esté ordenada por tamaño).
- **Peor calce (Worst Fit)**: asigna el hoyo **más grande** disponible. También requiere recorrer toda la lista (salvo que esté ordenada).

**Conclusión empírica (importante, mencionada explícitamente en clase y en las preguntas de repaso):** simulaciones muestran que **Primer calce** y **Mejor calce** son **mejores** que **Peor calce** en cuanto a utilización eficiente del espacio de memoria.

**Palabras clave:** Primer calce (First Fit), Mejor calce (Best Fit), Peor calce (Worst Fit).

---

## 3.14 Fragmentación de Memoria (Externa)

**Resumen:** Las estrategias de mejor y peor calce (y en menor medida primer calce) sufren el problema de la **fragmentación externa**: a medida que los procesos se cargan y se sacan de memoria, el espacio libre se va "rompiendo" en porciones pequeñas y dispersas. La fragmentación externa ocurre cuando **existe** suficiente memoria libre total para satisfacer una solicitud, **pero no está contigua**, por lo que no puede aprovecharse.

**Explicación de la analogía del estacionamiento:** Las diapositivas usan fotos de un estacionamiento de autos con espacios ocupados de forma irregular (un auto grande mal estacionado, dejando huecos pequeños inutilizables a los lados) como **analogía visual** de la fragmentación externa: hay "espacio total" suficiente, pero repartido en fragmentos demasiado pequeños o mal ubicados para que quepa un vehículo (proceso) que necesita un espacio contiguo.

**Palabras clave:** fragmentación externa, analogía del estacionamiento.

---

## 3.15 Compactación

**Resumen:** Una solución a la fragmentación externa es la **compactación**: desplazar las particiones ocupadas de memoria de manera que toda la memoria libre quede junta en **un solo bloque** contiguo. Limitaciones importantes:

- No siempre es posible: requiere **relocalización dinámica en tiempo de ejecución** (moverse mientras el proceso corre).
- Se debe mover tanto el programa como sus datos, y **actualizar el registro base** correspondiente.
- El algoritmo de compactación más común: mover **todos** los procesos hacia uno de los extremos de la memoria, de modo que todos los "hoyos" queden juntos en el extremo opuesto, formando un solo bloque libre grande.

**Palabras clave:** compactación, relocalización dinámica, movimiento hacia un extremo.

---

## 3.16 Hacia la solución definitiva: Segmentación y Paginación

**Resumen:** La solución más profunda al problema de la fragmentación es permitir que el espacio de direcciones **físicas** de un proceso **no sea contiguo**, cargándolo en memoria en cualquier "hoyo" disponible, sin importar su ubicación. Para esto se necesitan **dos mecanismos**: **Segmentación** y **Paginación** (esta última se estudia en detalle en la Clase 7).

**Palabras clave:** memoria física no contigua, segmentación, paginación.

---

## 3.17 Segmentación de Memoria

**Resumen:** Pregunta motivadora: ¿cómo ve normalmente la memoria un programador? No como un arreglo lineal de bytes indiferenciado, sino como una **colección de segmentos de tamaño variable**: métodos, variables, estructuras de datos, el stack, el heap, etc. Cada uno de estos elementos de tamaño variable se llama **segmento**.

- Los elementos **dentro** de un segmento se identifican por su **desplazamiento** (offset) dentro de ese segmento.
- En este esquema (**segmentación**), el espacio de direcciones lógicas es una **colección de segmentos**, cada uno con **nombre (o número)** y **largo**.
- Una dirección lógica es una **tupla**: `<número-de-segmento, desplazamiento>`.

**Explicación del ejemplo (vista de usuario vs. memoria física):** El diagrama muestra un programa con segmentos lógicos `S1:main`, `S2:printf`, `S3:call stack`, `S4:globals`, que en la vista del usuario aparecen en cierto orden "lógico", pero en la memoria física real pueden estar ubicados en **cualquier orden y en cualquier posición**, sin necesidad de contigüidad entre segmentos distintos (aunque cada segmento individual sí debe ser contiguo internamente).

**Segmentos generados típicamente por un compilador de C:** el código, las variables globales, el heap, los stacks de cada thread, y la biblioteca estándar de C. Las bibliotecas externas enlazadas estáticamente reciben también sus propios segmentos.

**Palabras clave:** segmento, desplazamiento (offset), tupla `<segmento, desplazamiento>`, segmentos generados por el compilador.

---

## 3.18 Hardware de Segmentación

**Resumen:** La **tabla de segmentos** es la estructura de datos que permite resolver una dirección física a partir de una dirección lógica de segmentación. Tiene **una entrada por segmento**, y cada entrada guarda:
- **base**: dirección física donde comienza el segmento.
- **límite**: tamaño (largo) del segmento.

El hardware, al recibir una dirección lógica `<s, d>` (segmento *s*, desplazamiento *d*), busca la entrada *s* en la tabla, **compara** *d* con el límite del segmento (si *d* ≥ límite, se genera un **trap** de error de direccionamiento), y si es válido, **suma** *d* a la base del segmento para obtener la dirección física.

**Explicación del ejemplo numérico:**
- El segmento 2 tiene largo 400 bytes y comienza en la dirección física 4300.
  - Una referencia al **byte 53** del segmento 2 se traduce a: 4300 + 53 = **4353**.
- El segmento 3 comienza en 3200; una referencia al **byte 852** se traduce a: 3200 + 852 = **4052**.
- El segmento 0 tiene límite 1000; una referencia al **byte 1222** (mayor que el límite 1000) genera un **trap** (acceso inválido, fuera de rango).

**Palabras clave:** tabla de segmentos, base y límite por segmento, trap de segmentación.

---

## 3.19 Ventajas y Limitaciones de la Segmentación

**Resumen:**
- **Ventaja**: la traducción de direcciones es simple (solo una suma y una comparación).
- **Limitación 1**: la segmentación **no resuelve** el problema de la fragmentación **externa** de memoria (los segmentos, al ser de tamaño variable, generan huecos igual que la asignación contigua clásica); la compactación sigue siendo costosa.
- **Limitación 2**: los segmentos pueden ser **arbitrariamente grandes**. Pregunta retórica de la clase: ¿tiene sentido mantener segmentos enteros y muy extensos en memoria si solo se necesita acceder a una parte pequeña de ellos?
- **Solución a ambos problemas**: la **paginación** (tema central de la Clase 7).

**Palabras clave:** fragmentación externa persiste con segmentación, segmentos de tamaño arbitrario, transición a paginación.

---

# PARTE 4 — CLASE 07: Administración de Memoria #2

## 4.1 Hoy Veremos (contenidos)

**Resumen:** Paginación de memoria, hardware de paginación, protección de memoria con paginación, tablas de página jerárquicas, administración de memoria en el Intel Pentium (caso de estudio), y memoria virtual.

---

## 4.2 Paginación

**Resumen:** La **paginación** es un esquema de administración de memoria que permite que el espacio de direcciones **físicas** de un proceso **no sea contiguo**. Previene la **fragmentación externa** y la necesidad de compactación. Es el esquema usado por la **mayoría de los sistemas operativos modernos**.

**Palabras clave:** paginación, no contigüidad física, sin fragmentación externa.

---

## 4.3 Paginación: Método Básico

**Resumen:** La memoria **física** se divide en bloques de **tamaño fijo** llamados **marcos** (*frames*). La memoria **lógica** se divide también en bloques del **mismo tamaño fijo**, llamados **páginas**. Cuando un proceso se ejecuta, sus páginas se cargan en marcos disponibles de memoria física (no necesariamente contiguos entre sí). Cuando se usa *swapping*, el almacenamiento no volátil (disco) también se divide en bloques del mismo tamaño.

**Descomposición de la dirección lógica:** cada dirección lógica generada por la CPU se descompone en dos partes:
- **p** (número de página): usado como **índice** en la tabla de páginas.
- **d** (desplazamiento en la página): posición dentro de esa página.

El **hardware de paginación** toma la dirección lógica `<p, d>`, busca en la **tabla de páginas** (indexada por *p*) el número de **marco** *f* correspondiente, y construye la dirección física como `<f, d>` (concatenación del marco con el mismo desplazamiento).

**Explicación de por qué se usan potencias de 2:** el tamaño de página es siempre una **potencia de 2** (típicamente entre 512 bytes y 16 MB, según la arquitectura) porque esto hace **trivial** la traducción: si el espacio de direcciones lógicas es 2^m y el tamaño de página es 2^n, entonces los **m − n bits más altos** de la dirección son directamente el número de página *p*, y los **n bits más bajos** son directamente el desplazamiento *d* — no se necesita ninguna operación aritmética (solo "cortar" los bits), lo cual es mucho más rápido en hardware que hacer una división.

**Explicación del ejemplo de la diapositiva 8:** Se muestra una memoria lógica con páginas 0-3 mapeadas (según la tabla de páginas: página 0→marco 5, página 1→marco 6, página 2→marco 1, página 3→marco 2) a marcos dispersos en la memoria física. La pregunta del profesor ("¿cuánto valen *n* y *m*?") busca que el alumno identifique, a partir del tamaño total de memoria lógica/física y el tamaño de página mostrados en el diagrama, los valores de estos exponentes usando la fórmula anterior.

**Palabras clave:** marco (frame), página, tabla de páginas, número de página (p), desplazamiento (d), potencia de 2.

---

## 4.4 Paginación y Fragmentación Interna

**Resumen:** La paginación **evita la fragmentación externa** (cualquier marco libre sirve para cualquier proceso), pero **puede generar fragmentación interna**: el **último marco** asignado a un proceso puede desperdiciarse si la memoria realmente usada no es un múltiplo exacto del tamaño de marco.

- Este problema es más relevante cuando el **tamaño de marco es grande** y el proceso usa pocos marcos en total.
- **Peor caso**: un proceso podría tener solo 1 byte ocupado en su última página, desperdiciando **2^n − 1 bytes** de fragmentación interna (siendo 2^n el tamaño de página).
- Si el tamaño del proceso es independiente del tamaño de página, se espera que la fragmentación interna promedie **media página por proceso** — esto sugiere que **páginas pequeñas son mejores** para minimizar este desperdicio.
- **Pero**: la utilización de disco es más eficiente con transferencias de datos **más grandes**, lo que sugiere lo contrario (páginas grandes). Es un **trade-off**.
- Algunos SS.OO. soportan **tamaño de página variable** (ej: Solaris soporta de 8KB a 4MB).

**Palabras clave:** fragmentación interna, trade-off tamaño de página, páginas de tamaño variable.

---

## 4.5 Tabla de Marcos

**Resumen:** Para llevar la contabilidad de qué marcos están asignados/libres, el SO mantiene una estructura llamada **tabla de marcos** (*frame table*), con una entrada por cada marco físico, indicando si está asignado, y en tal caso, a qué página pertenece.

**Palabras clave:** tabla de marcos (frame table).

---

## 4.6 Soporte de Hardware para Paginación: PTBR

**Resumen:** Cada SO implementa su propia forma de almacenar la tabla de páginas.

- **Sistemas antiguos** (con poca memoria): la tabla de páginas se guardaba en **registros especiales** del hardware — razonable solo si la tabla es pequeña (ej: máximo 256 entradas).
- **Sistemas actuales**: una tabla de páginas puede tener **millones de entradas** (ejemplo: memoria de 4GB con páginas de 4KB ≈ 1 millón de entradas), por lo que **debe mantenerse en memoria principal** (no en registros).
- Se usa un registro especial, el **PTBR (Page Table Base Register)**, que **apunta** a la tabla de páginas del proceso actual.
- **Ventaja del PTBR en cambios de contexto**: al cambiar de proceso, solo hay que **cambiar el valor del PTBR** para que apunte a la tabla de páginas del nuevo proceso (no hay que copiar toda la tabla).

**Palabras clave:** PTBR (Page Table Base Register), tabla de páginas en memoria, cambio de contexto.

---

## 4.7 El Problema de Performance del PTBR (doble acceso a memoria)

**Resumen:** El esquema con PTBR tiene un problema serio de **rendimiento**: para acceder a una dirección lógica *i*, primero hay que **acceder a memoria** para consultar la tabla de páginas (encontrar la entrada de la página donde está *i*) — **primer acceso a memoria**. Luego, con el número de marco obtenido, se construye la dirección física y se accede al dato — **segundo acceso a memoria**. En total, **cada acceso "lógico" a un dato requiere 2 accesos físicos a memoria**, lo que reduce la velocidad efectiva de acceso a memoria **a la mitad**.

**Palabras clave:** doble acceso a memoria, degradación de performance por factor 2.

---

## 4.8 Solución: TLB (Translation Look-aside Buffer)

**Resumen:** La solución estándar es una **memoria caché asociativa** de alta velocidad llamada **TLB (Translation Look-aside Buffer)**. Cada entrada del TLB tiene una **llave** (número de página) y un **valor** (número de marco correspondiente). El TLB típicamente tiene **pocas entradas** (entre 64 y 1024).

**Funcionamiento:** cuando la CPU genera una dirección lógica, se presenta al TLB, que **compara simultáneamente** (en paralelo, gracias a ser memoria asociativa) el número de página con **todas** sus llaves. Si hay coincidencia (**TLB hit**), se obtiene inmediatamente el número de marco y se arma la dirección física — sin necesidad de ir a la tabla de páginas en memoria. El acceso vía TLB es solo un **10% más lento** que si no existiera ningún mapeo, en vez del 100% adicional (doble acceso) del esquema sin TLB.

**Qué pasa en un TLB miss:** si el número de página **no** está en el TLB (**TLB miss**), se debe recurrir a la **tabla de páginas** en memoria para obtener el marco; al encontrarlo, se puede **actualizar el TLB** con esa nueva entrada para futuros accesos. Si el TLB está **lleno**, el SO debe elegir una entrada para **reemplazo** (políticas: aleatoria, menos usada recientemente, etc.). Algunas entradas (típicamente del **código del kernel**) se pueden mantener **siempre fijas** en el TLB (no sujetas a reemplazo).

**Palabras clave:** TLB, memoria asociativa, TLB hit, TLB miss, política de reemplazo del TLB.

---

## 4.9 Cálculo del Tiempo Efectivo de Acceso a Memoria (con TLB)

**Resumen:** El **hit ratio** (tasa de acierto) es el porcentaje de veces que se encuentra la página buscada en el TLB. El **tiempo efectivo de acceso** se calcula ponderando ambos escenarios (hit y miss) por sus probabilidades.

**Explicación completa del ejemplo numérico (el profesor menciona explícitamente TLB en la clase de repaso, hay que saber calcular esto):**
- Supongamos que una búsqueda en el TLB toma **20 ns**, y un acceso a memoria toma **100 ns**.
- **Caso TLB hit**: 20 ns (buscar en TLB) + 100 ns (acceder al dato en memoria con el marco ya conocido) = **120 ns**.
- **Caso TLB miss**: 20 ns (buscar en TLB, sin encontrar) + 100 ns (ir a la tabla de páginas en memoria) + 100 ns (acceder finalmente al dato) = **220 ns**.
- Con **hit ratio del 80%**: Tiempo efectivo = 0,8 × 120 + 0,2 × 220 = 96 + 44 = **140 ns**.
- Con **hit ratio del 98%**: Tiempo efectivo = 0,98 × 120 + 0,02 × 220 = 117,6 + 4,4 = **122 ns**.

**Conclusión del ejemplo:** mientras **mayor** sea la tasa de acierto del TLB, el tiempo efectivo de acceso se **acerca más** al mejor caso (120 ns, un solo "acceso lógico" con TLB hit), en vez de acercarse al peor caso (220 ns). Esto justifica por qué el diseño de un buen TLB (tamaño, política de reemplazo) es crítico para el rendimiento del sistema.

**Palabras clave:** hit ratio, tiempo efectivo de acceso a memoria, ponderación por probabilidad.

---

## 4.10 Preguntas de repaso propuestas en la clase (con pistas de cómo resolverlas)

**Resumen de las preguntas planteadas textualmente en las diapositivas** (buena guía de lo que se puede preguntar en el examen):

1. **Con 5 particiones de memoria** (100KB, 500KB, 200KB, 300KB, 600KB) y procesos de 212KB, 417KB, 112KB y 426KB (en ese orden): hay que simular cómo **Primer Calce**, **Mejor Calce** y **Peor Calce** ubicarían cada proceso, e identificar cuál usa la memoria de forma **más eficiente** (recordar: según la clase, primer y mejor calce superan a peor calce empíricamente).
2. **Espacio de direcciones lógicas de 32 bits, páginas de 4KB, memoria física de 512MB**: ¿cuántas entradas tiene la tabla de páginas? (Pista: el número de entradas de la tabla de páginas depende del **espacio de direcciones lógicas** dividido por el **tamaño de página**, es decir 2^32 / 2^12 = 2^20 ≈ 1.048.576 entradas — **no** depende de la memoria física en este cálculo básico de tabla de un solo nivel).
3. **¿Por qué los tamaños de página son siempre potencias de 2?** (Ver sección 4.3: permite separar la dirección en *p* y *d* simplemente "cortando bits", sin necesitar división aritmética).
4. **Diferencia entre fragmentación interna y externa** (Ver secciones 3.14 y 4.4: la externa es memoria libre total suficiente pero no contigua; la interna es desperdicio **dentro** de un bloque/página ya asignado, por no usarlo completo).

**Palabras clave:** primer/mejor/peor calce (aplicado), cálculo de entradas de tabla de páginas, potencias de 2, fragmentación interna vs. externa.

---

## 4.11 Protección de Memoria con Paginación

**Resumen:** La protección se logra con **bits de protección** asociados a cada marco/entrada de la tabla de páginas: por ejemplo, un bit indica si la página es de **solo lectura** o **lectura y escritura**. Cada vez que se traduce una dirección, se **verifican los permisos**; si se intenta escribir en una página de solo lectura, se genera un **trap de hardware** (violación de acceso a memoria). Se puede extender con más bits para distinguir: solo lectura, lectura-escritura, y solo ejecución.

**El bit de validez-invalidez (valid-invalid bit):** es un bit adicional en cada entrada de la tabla de páginas que indica si esa página **pertenece** o no al espacio de direcciones lógicas legítimo del proceso.

**Explicación detallada del ejemplo:** Arquitectura con espacio de direcciones de **14 bits** (direcciones de 0 a 16383). El proceso solo debe poder acceder a direcciones de **0 a 10468**. Con tamaño de página de **2KB**, las páginas válidas cubren direcciones de 0 a 12287 (la página 5 completa es "válida" según el bit, porque el rango de páginas de 2KB llega hasta ahí). **Problema identificado en el ejemplo**: aunque la página 5 está marcada como válida, el proceso en teoría **no debería** poder acceder a las direcciones 10469 a 12287 (que están dentro de esa página 5, pero **fuera** del rango real que el proceso necesita) — esto ilustra que el bit de validez-invalidez protege a **nivel de página completa**, no byte a byte, por lo que puede quedar un pequeño margen de direcciones técnicamente accesibles pero que el proceso no debería tocar (relacionado con la fragmentación interna).

**Palabras clave:** bits de protección, bit de validez-invalidez (valid-invalid bit), trap de violación de acceso.

---

## 4.12 Páginas Compartidas

**Resumen:** Una ventaja importante de la paginación es que **facilita compartir código** entre varios procesos (útil en ambientes de tiempo compartido). Distintos usuarios pueden ejecutar múltiples instancias del mismo programa, y sus procesos pueden **compartir las páginas** que contienen las secciones de código.

**Requisito clave:** el código compartido debe ser **reentrante** (o "puro"): código que **no se modifica a sí mismo**, es decir, nunca cambia durante su ejecución. Esto es lo que permite que múltiples procesos ejecuten simultáneamente las mismas instrucciones sin interferirse.

**Lo que NO se comparte:** las secciones de **datos, stack, heap y valores de registros** de cada proceso son **siempre independientes**, aunque compartan las páginas de código.

**Explicación del ejemplo (diapositiva de páginas compartidas):** Se muestran 3 procesos (P1, P2, P3) que comparten las mismas páginas de código (`ed1`, `ed2`, `ed3` — un editor, por ejemplo), cada uno con su propia página de datos (`data1`, `data2`, `data3`) y su propia tabla de páginas, pero todas las tablas apuntan a los **mismos marcos físicos** para las páginas de código compartido, mientras que los marcos de datos son distintos para cada proceso.

**Palabras clave:** código reentrante (puro), páginas compartidas, ambiente de tiempo compartido.

---

## 4.13 Tablas de Página Jerárquicas (Estructura Jerárquica)

**Resumen:** Con espacios de direcciones lógicos de 32 o 64 bits, la tabla de páginas de **un solo nivel** puede volverse **extremadamente grande**. Ejemplo: con direcciones de 32 bits y páginas de 4KB, la tabla necesita 2^32/2^12 = **1 millón de entradas**; si cada entrada ocupa 4 bytes, cada proceso necesitaría hasta **4MB** solo para su tabla de páginas — esto es muy costoso, especialmente si se multiplica por todos los procesos del sistema.

**Solución: paginar la tabla de páginas** (paginación en dos niveles o más), dividiendo tablas grandes en unidades manejables.

**Explicación del ejemplo de paginación en dos niveles:**
- Se divide la dirección lógica de 32 bits en: número de página de **20 bits** + desplazamiento de **12 bits** (para páginas de 4KB).
- Como la tabla de páginas de 20 bits también se pagina, esos 20 bits se dividen en: **10 bits** para un índice en la **tabla externa** (outer page table / directorio de páginas) y **10 bits** para un índice en la **tabla interna** (page of page table).
- El proceso de traducción: la dirección lógica `<p1, p2, d>` primero usa *p1* para indexar la **tabla externa**, obteniendo un puntero a una tabla interna específica; luego usa *p2* para indexar **esa** tabla interna, obteniendo el número de marco *f*; finalmente se combina *f* con *d* para formar la dirección física.
- **Ventaja**: no es necesario tener en memoria **todas** las tablas internas de una vez — solo las que realmente se usan (muchas entradas de la tabla externa pueden apuntar a "nada" si esa región del espacio de direcciones no se usa), ahorrando memoria significativamente en la práctica.

**Palabras clave:** paginación en dos niveles, tabla externa (outer page table / directorio de páginas), tabla interna, traducción de direcciones en dos niveles.

---

## 4.14 Caso de Estudio: Administración de Memoria en el Intel Pentium

### 4.14.1 Segmentación en el Pentium

**Resumen:** El Pentium soporta **segmentación pura** o **segmentación con paginación** combinadas. Tiene unidades de **Segmentación** y de **Paginación** que juntas implementan la **MMU**. Un segmento puede llegar a tener hasta **4GB**, y el máximo de segmentos por proceso es **16.384**.

**División del espacio de direcciones lógicas:**
- **8.192 segmentos privados** del proceso → información en la **LDT (Local Descriptor Table)**.
- **8.192 segmentos compartidos** por todos los procesos → información en la **GDT (Global Descriptor Table)**.

**Flujo de traducción**: `CPU → dirección lógica → Unidad de Segmentación → dirección lineal → Unidad de Paginación → dirección física → Memoria Física`.

**Estructura del selector de segmento (16 bits):** `s` (13 bits, número de segmento, hasta 2^13=8192), `g` (1 bit, indica si el segmento está en la GDT o en la LDT), `p` (2 bits, atributos de protección). El **desplazamiento** dentro del segmento es un valor de **32 bits**.

**Explicación del ejemplo de formación de dirección lineal:** La dirección lógica del Pentium es un par `<selector, desplazamiento>` de 48 bits en total. Uno de los **6 registros de segmento** de la CPU apunta a la entrada correspondiente en la LDT o GDT (a través de 6 registros de microprograma que actúan como caché para no tener que buscar en memoria cada vez); de esa entrada se obtienen los valores **base y límite** del segmento, que junto con el desplazamiento permiten calcular la **dirección lineal** de 32 bits.

**Palabras clave:** LDT, GDT, selector de segmento, registros de segmento del Pentium, dirección lineal.

### 4.14.2 Paginación en el Pentium

**Resumen:** La arquitectura Pentium soporta tamaños de página de **4KB o 4MB**.

- **Con páginas de 4KB**: se usa paginación en **dos niveles**. Los **10 bits más significativos** de la dirección lineal indexan la tabla externa, llamada en el Pentium **Directorio de Páginas** (*Page Directory*). Cada entrada del directorio apunta a una **tabla de páginas interna**, indexada por los **10 bits siguientes**. Los **12 bits menos significativos** (bits 0-11) son el desplazamiento dentro de la página de 4KB encontrada.
- **Con páginas de 4MB**: el directorio de páginas apunta **directamente** al marco de memoria física (sin tabla interna intermedia), usando los **10 bits más significativos** para direccionar el marco y los **22 bits restantes** como desplazamiento dentro de ese marco de 4MB.

**El registro CR3:** apunta al **Directorio de Páginas** del proceso actualmente en ejecución (es el equivalente al PTBR de la sección 4.6, pero específico del Pentium). El directorio de páginas tiene un **flag "Page Size"**: si vale **1**, se usan páginas de 4MB; si vale **0** (caso contrario), se usan páginas de 4KB.

**Palabras clave:** Directorio de Páginas (Page Directory), registro CR3, flag Page Size, paginación de 4KB vs. 4MB.

---

## 4.15 Memoria Virtual

**Resumen:** Pregunta motivadora: hasta ahora se asumió que **todo** el código de un programa debe estar en memoria física para ejecutarse. Pero la memoria física es un **recurso escaso**. ¿Tiene sentido mantener en memoria código que **casi nunca se ejecuta**, o estructuras de datos sobredimensionadas que nunca se usan a plena capacidad? La respuesta es no — de ahí nace la idea de **memoria virtual**.

**Beneficios de permitir que solo una porción del programa esté en memoria física:**
1. Los programas **dejan de estar limitados** por la cantidad de memoria física disponible; pueden tener un espacio de direcciones virtuales **más grande** que la memoria física real.
2. Al consumir menos memoria física por proceso, se puede **aumentar el grado de multiprogramación**, logrando **mejor utilización de CPU y throughput**, sin necesariamente perjudicar el tiempo de respuesta.
3. No se requiere hacer *swap* de procesos **completos**; los intercambios entre disco y memoria pueden ser **parciales** (solo las páginas que realmente se necesitan).

**Definición:** la **memoria virtual** abstrae al programador de la cantidad real de memoria física disponible: el proceso ve un espacio nominal de memoria virtual que puede ser más grande que la física. El **heap** permite asignación dinámica y crece en dirección **opuesta** al **stack**; el "agujero" entre ambos puede ser enorme en el espacio virtual, pero los marcos físicos solo se asignan **conforme se necesitan** realmente.

**Otro beneficio (compartir páginas):** con memoria virtual, varios procesos pueden compartir páginas para: tener **bibliotecas compartidas** cargadas una sola vez y usadas por varios procesos simultáneamente; **comunicarse** mediante páginas de memoria compartida; y **acelerar** la creación de procesos con `fork()` (compartiendo páginas en vez de copiarlas todas de inmediato).

**Palabras clave:** memoria virtual, espacio de direcciones virtual mayor que la física, grado de multiprogramación, throughput, heap/stack, bibliotecas compartidas, `fork()`.

---

## 4.16 Paginación por Demanda (Demand Paging)

**Resumen:** Con **paginación por demanda**, las páginas se cargan en memoria **solo cuando el programa las necesita** durante su ejecución; las páginas que nunca se usan, **nunca se cargan**. Se implementa con un mecanismo similar al *swapping* tradicional, pero en vez de intercambiar el **proceso completo**, se hace ***swapping* flojo (lazy swapping)**: un "paginador flojo" (*lazy pager*) carga una página individual en memoria únicamente cuando realmente se necesita, en vez de anticiparse cargando todo de una vez.

**Palabras clave:** paginación por demanda, lazy swapping, paginador flojo (lazy pager).

---

## 4.17 Validez e Invalidez de Páginas (aplicado a memoria virtual)

**Resumen:** Se reutiliza el **bit de validez-invalidez** (visto en la sección 4.11), pero ahora con un nuevo propósito: indicar si una página **está actualmente cargada en memoria física** o si **todavía está solo en disco** (no cargada). Si una página está marcada como inválida por esta razón, cualquier intento de acceder a ella disparará el mecanismo de **falta de página** (siguiente sección).

**Palabras clave:** bit de validez-invalidez (reutilizado para memoria virtual).

---

## 4.18 Manejo de Faltas de Página (Page Fault)

**Resumen:** Con memoria virtual y paginación floja, puede ocurrir que un proceso intente acceder a una página que **no está cargada** en memoria física. Cuando la CPU genera una dirección lógica para una página marcada como **inválida**, se produce una **falta de página** (*page fault*).

**Procedimiento estándar para manejar una falta de página (secuencia completa — importante saberla paso a paso):**
1. Se verifica en una **tabla interna del PCB** si la referencia a memoria es **válida** desde el punto de vista del espacio de direcciones del proceso (es decir, si el proceso realmente tiene derecho a esa dirección).
   - Si la referencia **no es válida** → el SO **termina el proceso** (acceso ilegal).
2. Si la referencia **es válida** (el dato simplemente no está en memoria todavía), el hardware genera un **trap**, y el SO debe traer la página desde el disco.
3. El SO busca un **marco disponible** en memoria física.
4. El SO programa una **operación de E/S de disco** para traer la página faltante desde el disco hacia el marco encontrado.
5. El SO **actualiza el bit de validez-invalidez** en la tabla de páginas del proceso, marcando la página como ahora **válida**.
6. Se **reinicia (re-ejecuta) la instrucción** que fue interrumpida por el trap. El proceso continúa como si nada hubiera pasado, ahora accediendo exitosamente a la página recién cargada.

**Explicación del diagrama numerado (1-6):** (1) referencia a memoria (`load M`) → (2) trap → (3) el SO determina que la página está en el *backing store* (disco) → (4) se trae la página faltante (*bring in missing page*) a un marco libre → (5) se resetea/actualiza la tabla de páginas (*reset page table*) → (6) se reinicia la instrucción (*restart instruction*).

**Palabras clave:** falta de página (page fault), PCB, trap, marco libre, restart de instrucción.

---

## 4.19 Actividad Final de la Clase (ejercicio integrador)

**Resumen:** Se plantea un escenario con: 1 CPU, 32MB de RAM, E/S de teclado/video/disco (20GB), un SO que requiere 8MB residentes en memoria, y 4 procesos (A: 20MB, B: 4MB, C: 10MB, D: 20MB) que deben poder ejecutar **concurrentemente** junto al SO — notar que **20+4+10+20+8 = 62MB > 32MB de RAM disponible**, por lo que **no caben todos simultáneamente en memoria física sin algún mecanismo de memoria virtual/paginación por demanda/swapping**. El ejercicio pide diseñar un sistema de administración de memoria (identificar componentes y su función) que resuelva esta situación — es un buen ejercicio de síntesis de **todo** lo visto en la Clase 6 y 7 (paginación, memoria virtual, swapping).

**Palabras clave:** ejercicio integrador, sobre-asignación de memoria, necesidad de memoria virtual/paginación por demanda.

---

# Resumen de Palabras Clave Transversales (para repaso rápido / flashcards)

**Sincronización (Clase 4 y 5):**
condición de carrera · sección crítica (entry/exit/remainder) · exclusión mutua · progreso · espera acotada · semáforo (`wait`/`signal`) · spinlock · mutex · semáforo binario · lock lector-escritor · buffer acotado · lectores-escritores · filósofos comensales · Test-and-Set · Swap · CompareAndSwap · variable de condición · semántica Mesa vs. Hoare · `pthread_cond_wait()` (libera y readquiere el mutex atómicamente) · `signal()` vs `broadcast()` · re-chequeo con `while` (nunca `if`) · sloppy counters · lock por bucket en tabla hash · **deadlock**: exclusión mutua, toma y espera, no expropiación, espera circular.

**Memoria (Clase 6 y 7):**
registro base/límite · MMU · registro de reubicación · binding en compilación/carga/ejecución · carga dinámica · enlace dinámico (stub) · primer/mejor/peor calce · fragmentación interna vs. externa · compactación · segmentación (tabla de segmentos, base/límite por segmento) · paginación (marco, página, tabla de páginas) · PTBR · TLB (hit/miss, hit ratio, tiempo efectivo de acceso) · bit de validez-invalidez · páginas compartidas (código reentrante) · tablas de página jerárquicas (paginación en 2 niveles) · Pentium (LDT, GDT, CR3, Page Directory) · memoria virtual · paginación por demanda (lazy swapping) · falta de página (page fault) y su procedimiento de 6 pasos.

---

*Fin de la guía. Cuando quieras, avísame y vamos revisando punto por punto según el orden del mail del profesor: (1) conceptos de sincronización y memoria, (2) variables de condición y análisis/corrección de código concurrente, (3) preparación para la pregunta IET de la tarea.*