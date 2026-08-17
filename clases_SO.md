# Clase 1 

estructura de vonnoimann, ver la entrada y salidas y ver los componentes, cpu, memoria y mas o menos como funciona 


# Clase 2

## Que es un proceso?

Un proceso es un programa en ejecucion, el programa esta en la memoria, la cpu ejecuta las instrucciones y los datos tambien deberian estar en la memoria, la imagen es el objeto ejecutable despues de la compilacion guatdada en el disco, para que el proceso pueda ejecutarse debe estar guardado en la memoria primero para poder ejecutarse. Cuando el programa esta en ejecucion tiene un layout.

![layout](imagenes/layout.png)

- texto: codigo de programa
- datos: variables globales
- stack: datos temporales
- heap: memoria asignada dinamicamente

Lo que pasa la cpu cuando de ejecuta un proceso es:
- fetch de la instruccion
- decodificacion
- ejecucion
- escritura de resultados en los registros 
- siguente instruccion y repetir

cuando un proceso esta cargado en memoria tiene un espacio de direcciones para el proceso, el tamaño del espacio depende el tamaño del bus de direcciones, osea de la arquitectura de la CPU, cuando el programa escribe en memoria queda actualizada la direccion de memoria en, puede ser operaciones de I/O. constantemente estamos creando datos en el stack y pedimos datos al heap.

### Estados de un proceso
- new: el proceso se esta creando 
- ready: el proceso esta esperando la cpu
- running: las instrucciones estan siendo ejecutadas por la CPU
- waiting: el proceso esta esperando qeu un evento ocurra, por ejemplo un I/O que se termine
- terminated: el proceso termino 

para un procesador un solo proceso puede estar ejecutandose a la vez, varios procesos pueden estar en estado ready o en waiting

![arquitectura_de_proceso](imagenes/ejecuciones.png)

### Process control block (PCB)

![PCB](imagenes/PCB.png)

cada proceso esta representado en el sistema por un PCB, es el ente que permite mantener el estado de ejecucion del proceso, contiene varios datos que describen el estado de un proceso:
- process  state: new, ready, waiting ...
- program counter: indica la direccion de la siguente instruccion
- registers: dependen de la arquitectura de la CPU
- informacion de planificacion y administracion de la memoria, prioridad del proceso e informacionde la tabla de paginas 
- estado de I/O: lista de dispositivos de I/O asignados al proceso.

### Como se puede hacer que varios procesos funcionen con una sola CPU

#### Multiplexacion de CPU
Con multiprogramacion el SO mantiene varios orocesos ejecutandose en memoria, esto quiere decir que varios procesos pueden estar en estado ready esperando a estar running, solo un proceso puede estar en running por cpu, el quantum de tiempo puede ser super corto a la percepcion del usuario humano, mayor quantum a mayor prioridad, basicamente la rotacion rapida de la CPU causa la ilusion de simultaneadad.

#### Cambio de CPU entre procesos
se conoce como cambio de contexto, los PCB guardan el estado del los procesos de la informacon en los cambios de contextos, los cambios de contextos se pueden gatillar como resultado de una interrupcion por ejemplo un timer. 

![contexto](imagenes/contexto.png)


## Planificaciond de un proceso 

la idea es mantener la cpu de forma ocupada de la manera mas eficiente, un planificador de procesos se encarga de elegir alguno de los procesos disponibles para que la cpu lo ejecute, la justicia de procesos es alfinal la probailidad que los procesos se ejecuten, estos procesos entran a una cola la cual contiene todos los procesos que se van a ejecutar. los procesos que residen en la memoroa principal y que estan listos entan en una cola y un queue de listo, cuando un proceso espera un I/O enytra a una cola de dispositivos, cuando sale de la cola de se dice que esta dispatch

implementacion de las colas:
![colas](imagenes/colas.png)

la imagen muestra un tipo de colas anclando pcb's

cuando un proceso es despachado podria requerir un I/O o puede esperar un subproceso y espera la la terminacion de este por ende el proceso padre queda encolado esperando que termine, cuando termina un proceso salen de la rotacion de colas, siempre al final terminan volviendo a la cola de los listos hasta que realmente termine el proceso, cuando esta waiting o algo siempe queda encolado en algun lado

![plan_colas](imagenes/plan_colas.png)

los pcb se mueven de cola en cola conforme cambian los estados del proceso 

un proceso migra entre colas en su ciclo que dependen del sistema operativo y los criterios de que proceso elige para ejecutar, hay un planificador especifico para eso 

- planificador de corto plazo (dispatcher)
- planificador de medio plazo
- planificador de largo plazo (job shceduler)

largo y corto plazo difieren en la frecuencia con la que se ejecutan, corto con mucha frecuencia y el largo con poca.

El planificador de largo plazo control a el grado de multiprogramacion en el sistema.

comportamiento de procesos, acotado por cpu el mayor tiempo es en operaciones de computo y por I/O el mayor tiempo es utilizado en operaciones I/O, si es I/O el dispatcher esta casi vacio el otro caso la cola de dispositivos permanece casi vacia.

mediano plazo: sacar procesos de la memoria para reducir la multiprogramacion

### operaciones e procesos 
#### Creacion de procesos 
un proceso puede crear nuevos procesos mediante una system call durante la ejecucion, el creador se llama padre y crea un hijo, el hijo tambien puede crear nuevos hijos y asi se crea un arbol genealogico de procesos, cada proceso tiene un id 

![arbol](imagenes/arbol.png)

construir un proceso cuesta, el pcb cuesta poco recurso, copiar datos del proceso padre al hijo se llama fork() y copia exactamente el I/O y los datos y esto es costoso, fork es crear un hijo en semejanza, retorna al hijo y al padre devuelven cosas distintas osea al proceso hijo retorna un int que es el indice del proceso.


## Terminacion de un proceso 

cuando un proceso termina se le puede solicitar al SO que elimine sis doatos para liberar recursos, se usa exit()
el wait() es qeu el proceso padre espera a que el hijo termine

los threads son procesos livianos, un hilo puede crear procesos liviando, se requiere una funcion de donde partir esto, no comparten espacios de memoria, necesitan stacks distintos, lo que si sirve es que tiene paralelismo entre procesos, osea los threads pueden ejecutar procesos, se comparten datos en el heap, una lista ligada por ejemplo, por ejemplo en lectura no hay mucho problema pero en escritura si hay, ahi se va a ver 


En resumen un procesos es una abrstacciond del sistema operativo utilizada para representar todo lo que se requiere para ejecutar un programa, un programa puede tener varios o solo un hilo.

![threds](imagenes/multithreads.png)

no se puede tener infinitos stacks por ende no se puede tener infinitos threads, es limitante por la memoria, un ejemplo un cliente crea una peticion y esto se crea un nuevo thread y ese thread procesa la peticion

las ventajas son:
- mejor respuesta del sistema al usuario
- facilidad de recursos compartidos
- escalabilidad: se puede aprovechar multiprocesadores

hay threads en espacio de usuario y de kernel, habia que restringir la cantidad de threads que habian, los user son threads son administrados por bibliotecas y los kernel, los user threads tienen relaciones con los user, uno a uno, uno a muchos mucho a muchos 

- muchos a uno: muchos user a uno kernel un thread accede a la vez al kernel
- uno a uno: mejor concurrencia que muchos a uno, permite paralelismo mayor costo
- mucho a mucho: se puede hacer paralelismo y ejecutar y ver que thread toma que cosa