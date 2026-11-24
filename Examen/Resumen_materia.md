# Procesos, Threads, planificación e IPC

## Procesos
Un programa es un archivo pasivo en el disco, cuando este se ejecuta se combierte en un proceso. Tiene un espacio de direcciones de memoria a las que un proceso puede acceder, este contiene el codigo, datos , el heap que crece hacia arriba y el stack que crece hacia abajo. Entre el heap y stack hay un espacio gigante que se va ocupando despues.\ Tambien estan los registros de CPU que contienen los program counter PC que apunta a la proxima instruccion.

El proceso siempre es el mismo flujo que es fetch -> decode -> execute -> store. El proceso puede estar en diferentes estados: new, ready, running, waiting y terminated.

| estado | descripción |
|--------|-------------|
| New    | El proceso se esta creando |
| Ready  | El proceso puede correr pero esta esperando que le asignen CPU |
| Running| El proceso esta en la CPU |
| Waiting| El proceso esta esperando un evento como algo de Entrada/Salida (E/S) o un trigger|
| Finished| El proceso ya termino su ejecucion |

### PCB 
El PCB es el process control block, es la estructura en el kernel que representa a cada proceso que contiene toda la informacion del proceso.
- El estado del proceso
- El PC (program counter) y los registros cuando un proceso sale de la CPU
- La prioridad del proceso para la planificación
- Información de la memoria, puntero a su tabla de páginas
- Contabilidad: el PID, tiempo de CPU usado
- Estado de E/S

### Cambio de contexto 
Para hacer un cambio de contexto, el kernel guarda todo lo necesario en el PCB del proceso que esta corriendo y carga el PCB del proceso que viene, esto es costoso porque se tiene que guardar y cargar los registros, el PC, la tabla de paginas, etc. Por eso el quentum que gatilla los cambios de contexto no pueden ser muy chicos, porque si no se pierde mucho tiempo en cambiar de contexto y no en ejecutar procesos.

## Planificadores y colas 

El kernel mantiene colas de PCBs, a lo que va es que hay distintas colas dependiendo del estado el proceso, hay una cola para ready, otros para waiting, etc... \
Hay tres tipos de planificadores: 

- **Corto Plazo:** Tambien llamado despacher, decide que proceso que esta en *Ready* recibe la CPU, es muy rapida y se ejecuta seguido.
- **Largo Plazo:** Es el job scheduler, es el planificador que decide que procesos van a entrar del disco duro al sistema, osea a la Memoria principal que es la RAM. Controla el grado de multiprogramacion, osea cuantos procesos viven en la RAM. 
- **Mediano Plazo:** Es el que saca procesos de la RAM y los manda a disco denuevo.

## FORK, EXEC, WAIT y EXIT

### Fork()

Crea un proceso que es la copia del padre, tiene el mismo codigo, copia los datos, heap y stack, solo se pueden distingir por el PID o el valor que retorna fork() que es 0 para el hijo y el PID del hijo para el padre. Tienen memoria separada, asi el hijo si cambia un dato el padre no lo ve y viceversa. Hacer fork es costoso porque se tiene que copiar toda la memoria del padre al hijo, por eso se implementa el *copy on write* que es una optimizacion que hace que el padre y el hijo compartan la memoria hasta que uno de los dos haga un cambio, ahi se copia la pagina de memoria y se hace un cambio. basicamente ambos apuntan a la misma pagina de memoria hasta que uno de los dos hace un cambio y ahi se copia la pagina de memoria que se hace la escritura.

### Exec()
Reemplaza el proceso actual con otro que esat en el programa, se conserva el PID, pero se reemplaza el codigo, datos, heap y stack. Se usa para ejecutar un programa distinto al que se esta ejecutando.

### Exit()
termina el roceso y retorna un estado de salida al padre. 

### Wait()
se bloquea al padre hasta que el hijo termine y recuoere su codigo de salida.

## Threads
Es una unidad de ejecucion dentro de un proceso, es el componente activo osea la concurrencia y el espacio de direcciones es la parte pasiva. 

Los threads de un proceso pueden compartir distintas cosas pero tambien tiene sus cosas privadas.

|comparten|privadas|
|---------|--------|
|codigo|registros y PC (program counter)|
|Variables globales| stack (variables locales)|
|Heap| informacion de planificacion (prioridad, estado, etc)|

dos threads de un mismo proceso eventualmente pueden compartir variables locales si se pasan punteros, y tambine pueden acceder a variables globales y al heap

**Porque usar threads?**\
- La interfaz sigue respondiendo mientras que otro thread se bloquea 
- comparte recursos sin configurar IPC 
- Es mas barato que crear procesos nuevos 
- Permiten paralelismo dentro de la CPU

## Planificación de CPU
Cuando decide un planificador:
1. running -> waiting(E/S)
2. running -> ready (quantum)
3. waiting -> ready (evento)
4. el proceso termina y se va a finished

cuando es la 1 y 4 es cooperativa, en el caso 2 y 3 es preemptiva, osea que el planificador decide cuando un proceso deja de correr y otro empieza a correr. osea expropiativa

#### Metricas de planificación
- Utilizacion de CPU: se busca maximizar
- Throughput: cantidad de procesos terminados por unidad de tiempo
- Tiempo de completitud (turnaround time), desde que llega hasta que termina
- Tiempo de espera, suma de tiempos que pasan en la cola de listos 
- Tiempo de respuesta: desde que llega hasta que corre por priemra vez

### FCFS (First Come First Serve)
No expropieativo: corren en orden de llegada, el problema es que si un proceso va primero que procesos cortos entonces los procesos cortos quedan atascados, esto se llama convoy effect. \
### SJF (Shortest Job First)
Elige el proceso que tiene la rafaga mas corta, minimiza el tiempo de espera promedio, pero necesita conocimiento del futuro.
### SRTF (Shortest Remaining Time First)
Es expropativo, si llega un procesos nuevo que requiere menos tiempo del que se esta corriendo se expopia la CPU corre el proceso nuevo. 


El problema con estos algoritmos es la inanicion, que si llegan puros procesos nuevos de corta rafaga los procesos pesados nunca corren. para evitar eso se hace un sistema de prioridades.

### Round Robin
Es expropiativo, cada proceso recive un quantum de tiempo, si no termina en ese tiempo se expropia y se manda al final de la cola y espera si turno. Cuando el Q es muy grande se comporta como un FCFS y cuando es muy chico hay muchos cambios de contexto lo cual es caro.

### MLFQ 5 reglas finales 

El objetivo es tener un buen turn arround, tiempo de respuesta sin conocer lo que se viene de antemano.
1. Si prioridad(A) > prioridad(B) entonces corre A antes que B
2. si orioridad A = prioridad B entonces corre A y B en round robin
3. Proceso nuevo entra a la cola de maxima prioridad
4. cuando un proceso termina si Q baja de prioridad, el Q se acomula para evitar trampas si suelta la CPU antes de que se agote el Q de tiempo.
5. Cada un cierto periodo S todos los procesos suben a la prioridad maxima.


### IPC (Inter Process Communication)
Hay dos maneras de hacerlo:
- Paso de mensajes: se puede hacer por pipes o por sockets.
- Memoria compartida: una region mapeada en el espacio donde varios procesos acceden.

los pipes son un canal unidireccional basicamente un proceso escribe y el otro lee, cuando es un pipe sin nombre solo se puede hacer con procesos emparentados y se tiene que crear el pipe antes del fork, cuando es un pipe con nombre se puede hacer entre procesos no emparentados y se crea con mkfifo.

# Sincronizacion de procesos
## Condicion de carrera:
cuando varios threads acceden a una variable compartida y al menos uno de ellos la modifica el resultado depende del orden de ejecucion de los threads, que sto puede fallar a nivel anatomico.

## Seccion critica
Es el fragmento de código donde se accede a la variable compartida.
Para evitar esto se usan mecanismos de sincoronizacion que si o si tiene que tener estas reglas:
1. Exclusion mutua: si un thread accede a la seccion critica nadie mas puede acceder a ella.
2. Un thread no puede impedir que otro entre a la seccion critica, osea que no puede bloquear a otro thread.
3. Espera acotada, no puede haber un threafd que nunca pueda entrar, opsea no puede haber inanicion de entrada a la seccion critica. 

## Locks y spinlocks

un lock se adquiere antes de entrara a la seccion critica y se libera al salir.
asi se asegura que solo un thread pueda acceder a la seccion critica sin toparse y se cumple la regla de exclusion mutua.

### Test and Set 

es una instruccion del hardware que lee el valor anterior y escribe uno nuevo en un solo paso indivisible 

```c
int lock = 0;
void acquire(int *lock){
    while (atomic_exchange(lock, 1) == 1);
}
void release(int *lock){ lock = 0;}
```

Si el lock esta libre es 0, si esta tomado el lock es 1, entonces lo que hace es que revisa si el lock esta tomado, si no lo esta lo toma y escribe 1, si esta tomado lo que hace es que queda en un loop que sobreescribe 1 hasta que se libere el lock.

pero el problema de esto es que cmo no hay orden de llegada asegurada un proceso puede estar esperando infinito y pierde entrar a la seccion critica.

el Spinlock se sigue ejecutando en el while, el problema es que igual consume CPU de manera inecesaria mientras que el mutex bloquea el thread y lo manda a waiting y avisa cuando se desocupa.

entonces lo es depende para que se quiere usar, el spinlock es mejor para secciones criticas cortas y el mutex cuando hay que hacer mucha espera.


## Semaforos 
Es un semaforo contador entero compartido para poder sincronizar procesos, ocupa solo dos operaciones atomicas que es el wait y el signal, el wait decrementa el contador y si es menor a 0 bloquea al thread, el signal incrementa el contador y abre el lock. 

Un semaforo binario funciona como un mutex tiene 0 y 1, el wait bloquea si es 0 y el signal desbloquea si es 1.

El lock en cierta parte se convierte en un cuello de botella dado que solo un thread puede entrar a la seccion critica a la vez, esto se puede optimizar.

- Lector escritor: si hay varios lectores no hay problema que lean varios procesos o threads el mismo dato dado que no se modifica, si se necesita escribir ahi si se necesita la mutua exclusion.

- hand over hand: lo que hace es que una lista cada nodo que vendria siendo el dato y el puntero tiene su propio lock y para pasar al siguente nodo priemro entra al lock de B y luego suelta el A, asi no se puede colar ni un thread entre A y B, pero si hay varios threads que quieren entrar a la lista pueden entrar a distintos nodos al mismo tiempo.

## Variables de condicion
Un Mutex solo asegura la exclusion mutua, pero hay veces que el thread necesita esperar a que algo mas pase como una clave o algo, hacer eso con wait desperdicia CPU y se tendria que soltar el lock en si, La variable de condicion CV es una cola donde los threads duermen hasta que se cumpla la condicion y les avisa cuando despertar.

Entonces lo que hace es que siempre va de la mano cin un mutex, lo que hace es que suelta el mutex y lo hace dormir para que otro thread pueda entrar a la seccion critica. Espera a que alguien le haga signal y al despertar sigue y toma al mutex nuevamente.

El signal lo que hace es que despierta a un thread que esta dormido en la variable de condicion.\
Broadcast despierta a todos los threads que estan dormidos en la variable de condicion.

El que tiene el mutex tomado es el que tiene que avisar que se modifico algo.


## Deadlock
Es un conjunto de procesos que queda bloqueado para siempre porque cada uno espera un recurso que tiene otro del mismo conjunto.
Al final es que si el thread 1 toma el lock a y el thread 2 toma el lock b, pero A necesita algo de B y B necesita algo de A se quedan esperando para toda la vida.

# Memoria Basica
La ram se particiona en pedazos fijos y cada programa compila para su pedazo, asi no se cruzan las direcciones de memoria, se tiene una base que es donde parte y luego un limite qu es el tamaño del pedazo.

Fragmentacion externa es cuando hay espacio total libre pero no hay pedazos suficientes para que un proceso quepa. La solucion es hacer una compactacion de memoria, mover los procesos para que queden juntos, el unico problema es que es costoso y se tiene que parar todo el sistema para hacerlo.

La fragmentacion interna es cuando sobra espacio dentro de un pedazo que te asignaron, por ejemplo te asignaron 100 por ende fisicamnets se ocupa 100 peri en verdad el proceso ocupa 80 por ende sobra 20 que en teoria esta ocupado pero no se ocupa y no lo puede tomar otro proceso.

## Paginacion
Es cuando la memoria se corta en fragmentos de tamaño fijo que se llaman marcos, cada programa se corta en fragmentos del mismo tamaño que se llaman paginas, cada pagina va en cualquier marco aunque no esten juntos, cada proceso tiene una tabla de paginas que anota donde quedo cada pedazo.

# Memoria Virtual y sistemas de archivos

## Paginacion de dos niveles
Lo que pasa es que cuando se tiene una sola tabla de paginas de puede que sean muchas entradas y eso es muy caro dado que pesa mucho y leerlo es lento.

La idea es partir esa tabla y que se ocupen lo pedazos que realmente se van a usar, entonces se hace un directorio de paginas que vendria siendo el indice de las tablas, esta siempre existe y la tablas de segundo nivel, que se van creando dependiendo si la el proceso lo necesita.

se hace mas o menos asi:\ 32 bits se parten en 10 de directorio, 10 de tabla de segundo nivel y 12 de offset 

## Bits de una entrada de una tabla PTE (page entry table)

|bit|nombre|significado|
|---|------|-----------|
|V|Valido/presente| 1 si la pagina esta en la RAM, 0 si esta en disco|
|A|Acceso| 1 si se uso hace poco|
|D|Dirty| 1 si la pagina se modifico

hay 3 casos posible de V = 0 que es qu ela region no pertenece al proceso, es valida pero no esta en la RAM, o hay que crealrla 

## TLB
Es un cache chico que se ocupa para guardar las traducciones recientes para no tener que acceder a la tabla completa que toma mas tiempo cuando es un hit toma un acceso, cuando es miss toma dos minimo si es de un solo nivel, si es de dos niveles toma 3 accesos, que es el TLB, dorectorio de paginas y tabla de segundo nivel y el dato

### EAT
Es la tasa de aciertos con respecto a un hit y un miss.
$EAT = (hit_ratio * tiempo_de_acceso_a_TLB) + (miss_ratio * tiempo_de_acceso_a_memoria)$

Lo que se puede sacar de esta formula es que el hit con mas niveles el miss es mas caro, pero la mayoria siempre es hit entonces vale la pena. 

Esto funciona dado que lo que se uso recien probablemente se va a usar denuevo por ejemplo un ciclo for.

## Falta de Página
Las paginas se traen del disco cuando se necesita, cuando la pagina no esta en ram el bit V es 0 y ocurre un swap.

### Cuando hay un marco libre en la RAM
1. La CPU accede, la PTE tiene el bit V = 0, se hace un trap.

2. Se verifica si la direccion es valida.

3. Se busca un marco libre dentro de la RAM

4. El programa hace una lectura de disco hacia ese marco, el proceso esta bloqueado pero se corre otro proceso mientras tanto.

5. Se termina la lectura y se pone el bit V = 1 y se actualiza la PTE

6. Se reintenta la instruccion.

### Cuando no hay un marco libre en la RAM
Es similar al anterior, la diferencia es que:
1. Tiene que elegir una victima a quien sacar con un algoritmo de reemplazo.
2. Si la victima esta sucia hay que escribirla en disco antes de reemplazarla, si no esta sucia solo se reemplaza.
3. Poner la victima en V = 0.
4. Invalidar la entrada de la TLB de la victima.
5. Pasos de la pagina nueva

$EAT = (1-p) * ma + p * pft$
- p = probabilidad de page fault
- ma = tiempo de acceso a memoria
- pft = tiempo de page fault (tiempo de atender una falta)

## Algoritmos de reemplazo 

| Algoritmo | Regla |
|-----------|-------|
| FIFO | Se saca la primera que llego |
| OPT | se saca la que no se usara en mas tiempo|
| LRU | se saca la que lleva mas tiempo sin usar, con el metodo del reloj|

## Thrashing 
Cuando el sistema pasa mas tiempo paginando que ejecutando, la causa es que los procesos no tienen suficientes marcos para el conjunto de trabajo, basicamente se entra en una cadena de faltas.

Lo que pasa es que hay muchas faltas esperando el disco, la CPU queda ociosa y el sistema operativo al ver esto dice que deberia quedar espacio libre y manda a agregar mas procesos, al final lo que pasa es que el disco esta al 100%, la CPU libre. La solucion es reducir la multiprogramacion, osea sacar procesos de la RAM y mandarlos a disco para que los que quedan tengan mas marcos y puedan ejecutar sin thrashing.

## FAT
Es como una lista anidad que apunta al bloque siguente pero esta guardada en un arreglo, la FAT se mantiene en la memoria por lo que es mas rapido de leer.

## i-nodos 
cada archivo tiene un i-nodo que contiene metadatos y punteros a sus bloques 
- Directo: directo al bloque de datos
- Indirecto simple: apunta a un bloque que contiene punteros a bloques de datos
- Indirecto doble: apunta a un bloque que contiene punteros a bloques que contienen pun


