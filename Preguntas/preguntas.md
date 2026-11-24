# Prueba 1 

### ¿Cuándo ocurren cambios de contexto cuando hay programación (i) cooperativa (ii) expropiativa

Para la multiprogramacion cooperativa es cuando el proceso renuncia voluntariamente a la CPU, cuado pasa de running a waiting o cuando es terminado el proceso. Para la planificacion expopiativa depende de un timer, generalmente un watchdog que hace los cambios de contextos.

### ¿Cuál es la diferencia entre una llamada a procedimiento (procedure call) y una llamada al sistema (system call)? ¿Qué rol juega la operación en modo dual en esto?

La diferencia es que el procedure call es en modousuario entonces la CPU ejecuta el proceso con menos privilegios en cambio los system call son en modo kernel qeu tiene alto privilegio en la dualidad de la CPU (modo kernel y modo usuario).

### ¿Por qué crear un thread es más económico computacionalmente que crear un proceso?
crear un proceso se tiene que reservar la memoria para aquel procedimiento ademas de los datos y demas, esto toma computo de CPU que el thread no necesita.

### Nombra dos diferencias fundamentales entre un pipe ordinario y un Unix domain socket.
El Pipe es unidireccional y el socket no, ademas el pipe se podria decir que es un arreglo predeterminado, el socket puede mandar datos en tiempo real o mandar paquetes que el pipe no lo hace, y el socket se reserva con un nombre en la memoria el pipe no.

### Describe cómo sería posible crear en C un programa tal que al ejecutar genere un proceso hijo que ejecute un programa distinto

lo primero es hacer un fork() al padre para crear al hijo, luego en el hijo con un if que verifique el PID se jecute la system call exec() para transformar el hijo en otro proceso.

### Recuerda el planificador MLFQ con las reglas 4a y 4b que permiten que un programador malicioso pueda aprovecharse del planificador y ejecutar su programa con la máxima prioridad. La regla 4 definitiva discutida en el libro OSTEP permite solucionar este problema. (i) ¿De qué manera un programador puede crear un programa malicioso que mantenga su prioridad siempre al máximo? (ii) ¿En qué consiste la regla 4 definitiva que permite al planificador defenderse de este tipo de ataque?

Alguien puede crear un programa que el proceso nunca acabe el quantum de tiempo determinado, por ende la regla 4b hace que no baje su prioridad entonces el programador puede cortar el proceso antes de esto y queda siempre arriba, la solucion definitiva de la regla 4 es que no se ve el tiempo de cada proceso en estado running sino el tiempo acomulado del proceso entonces si hay muchas interrupciones pero esas pequeñas cantidades suman el quantum a ese proceso se le baja la prioridad 

---

# Prueba 1 — 2025-20 (segunda ronda)

### 1. ¿Cómo se evita en la planificación Multi-Level Feedback Queue (MLFQ) que ocurra inanición de procesos intensivos en uso de la CPU?


### 2. Cada thread que ejecuta un proceso requiere un punto de partida, es decir, una primera instrucción a ejecutar. Para el caso de thread de la API POSIX, ¿cómo se define el punto de partida al crear un thread?


### 3. ¿Es posible usar un pipe para comunicar dos threads dentro de un mismo proceso? Justifica por qué esta no es una práctica recomendable. Hint: ¿Qué funciones son las que se usan para leer y escribir desde/al pipe?


### 4. ¿Por qué la system call exec no retorna a su código cliente (el código que originalmente la invocó) cuando ejecuta en forma exitosa?


### 5. Si dos threads en un proceso hacen incrementos unitarios de un contador compartido, inicializado en 0, por una cierta cantidad de ciclos, digamos, 10000 c/u, y el resultado es distinto a 20000, ¿qué clase de problema es este y cómo podría resolverse? Explica la situación y su solución mencionando al menos cinco conceptos clave vistos en las clases 3 y 4.


### 6. Considera que existe una tabla en una base de datos relacional que es utilizada por numerosos procesos lectores y escritores, y que no tiene ningún tipo de sincronización. ¿Qué problema causaría que haya más de un proceso escritor operando en un mismo conjunto de filas de la tabla a la vez? ¿Qué solución eficiente existe para este tipo de problema, en consideración de que hay probablemente más procesos lectores que escritores que requieren acceso a la tabla?


---

# Actividad 1 — 2024-20 (tercera ronda)

### 1. ¿Por qué un proceso se bloquea (queda en estado waiting) cuando hace una operación de E/S? Da un ejemplo.


### 2. En planificación de procesos, el paso de un proceso a otro es lo que llamamos un context switch. ¿Cuál es el efecto de que la operación de context switch tome un tiempo considerable (no despreciable)? Hint: frecuencia.


### 3. Considera el siguiente código de procesos cooperativos productor y consumidor, que se ejecutan con planificación expropiativa. Si se deja de sincronizar el acceso a counter, ¿cómo se le llama al problema general que ocurre en este caso? ¿por qué un semáforo binario podría resolver el problema?

```c
void* productor(void*) {          void* consumidor(void*) {
    /*...*/                           /*...*/
    while(counter == BUFFER_SIZE);    while(counter == 0);
    /*...*/                           /*...*/
    counter++;                        counter--;
    /*...*/                           /*...*/
}                                 }
```


### 4. ¿Por qué es necesario copiar los valores de los registros de la CPU a la memoria (en el Process Control Block) del último proceso en ejecución cuando ocurre un cambio de contexto?


### 5. El planificador MLFQ mantiene en alta prioridad a los procesos que no alcanzan a agotar el quantum de CPU que se les asigna para ejecutar, porque realizan alguna llamada al sistema y/o alguna operación de E/S bloqueante. Dado que la regla 1 dice, si P(A) > P(B), entonces A ejecuta y B no ejecuta, ¿cómo se comporta el planificador cuando hay un proceso P1 que es de larga ejecución y altamente intensivo en uso de CPU (acotado por CPU), que ejecuta concurrentemente con otro proceso P2 que se bloquea con alta frecuencia y a intervalo regular realizando operaciones de E/S?


### 6. Menciona una fortaleza y una debilidad de (i) planificación tipo SJF (trabajo más corto primero), (ii) planificación round-robin.


---

# Ronda de práctica (cuarta ronda)

> Preguntas de práctica en el estilo de las pruebas anteriores, apuntando a temas
> del temario que las tres rondas previas no cubren.

### 1. Explica por qué dos procesos distintos pueden usar la misma dirección de memoria (por ejemplo, la 0x1000) sin interferir entre sí. ¿Qué rol juega el sistema operativo en esto y cómo se llama la abstracción involucrada?
es por la virtualizaciond e la memoria, el sistema operativo lo que hace es que agarra espacio de memoria fisica y otro virtual que es como un arreglo compartido para todos los procesesos.


### 2. Un programa declara una variable global, la inicializa, y luego hace fork(). Si el padre y el hijo modifican esa variable, ¿ven el mismo valor? Contrasta este comportamiento con lo que ocurre cuando dos threads del mismo proceso modifican una variable global.
el hijo resive una copia y no afecta, se le pasa por decir una copia de eso, el thread si comparte el espacio de memoria de el proceso 


### 3. En los ejemplos de pipes vistos en clase, la llamada a pipe() siempre se hace antes de fork(). ¿Qué ocurriría si se invirtiera el orden? Justifica tu respuesta en términos de qué es exactamente lo que fork() duplica.

si se hace despues del fork se crearia una copia del pipe por lo que habrian dos pipe y no uno que se comuinique entre procesos 


### 4. Describe las tres formas de interacción entre la CPU y los dispositivos de E/S (programada o polling, por interrupciones, y DMA). ¿Cuál permite el mejor aprovechamiento de la CPU y por qué?

- Programada: la cpu controla directamente la transferencia mediante instrucciones(polling), sistemas simples.
- interrupciones: la CPU delega la operacion y sigue con otras tarea, el dispositivo interrumpe cuando termina o necesita atencion
- directo a la memoria: se transfiere datos entre la memoria y un dispoitivo sin intervencion directa de la cpu, DMA es el que hace esto que actua como una mini CPU

siendo dma el mejor


### 5. Los threads de un proceso comparten el heap y el segmento de datos, pero cada uno requiere su propio stack. (i) ¿Por qué es necesario un stack separado por thread? (ii) ¿Qué consecuencia práctica tiene esto sobre la cantidad máxima de threads que un proceso puede crear?

es que el stack es independiente por los procesoso y funciones que ejecutan, al tener distintos stack por threads va consumiendo recursos por lo que si hay infinitos se va a tomar la memoria y CPU completa


### 6. Compara los planificadores FIFO y STCF respecto al efecto convoy. ¿Por qué STCF no sufre el problema que sí tiene SJF cuando un trabajo corto llega al sistema justo después de que uno largo comenzó a ejecutarse?

fifo el efecto convoy dice que si un proceso grande llega a la CPU, este se lo toma y los procesos chicos quedan atrapados, el STCF, tiene un quantum de tiempo que va revisando los procesos que estan en la cola, entonces cada cierto tiempo hay u watch dog que ve canto le quedan a los procesos y ve el mas corto y reorganiza la cola.


### 7. En IPC, contrasta el paso de mensajes con la memoria compartida en términos de: (i) velocidad, (ii) quién es responsable de la sincronización, y (iii) cuánto participa el kernel en cada operación de intercambio de datos.

no tengo idea


### 8. Un kernel monolítico y un microkernel difieren en dónde se ejecutan los servicios del sistema. ¿Qué ventaja de confiabilidad ofrece el microkernel y a qué costo? Relaciona tu respuesta con el modo dual de operación de la CPU.

microkernel la ventaja que tiene es que modulariza y aisla el sistema y no afecta si algo se rompe pero es mas costoso, y el modo dual es con los privilegios 


