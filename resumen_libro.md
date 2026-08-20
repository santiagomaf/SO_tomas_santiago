# Resumen Prueba 1

Un sistema operativo es una pieza de software que hace facil usar el sistema, y lo logra mediante la virtualización, el cual toma recursos fisicos y lo transforma en una version mas general, el sistema operativo ademas tiene interfaces (API's) que son system calls para que los programas puedan pedirle cosas al sistema 

## Virtualizing the CPU

En la virtualizacion de la cpu crea la ilusion de que hay mas de lo que en verdad hay, por ejemplo en la funcion de cpu.c llama repetida la función spin() y llama a una cadena de texto, un ejemplo es si llamamos a ./cpu "A" va a llamar repetidamente "A", en cambio si llamamos ./cpu "A" & ./cpu "B" va a llamar intercalado A y B

```c

 #include<stdio.h>
#include<stdlib.h>
#include<sys/time.h>
#include<assert.h>
#include"common.h"

int
main(intargc,char *argv[])
{
if (argc!= 2){
fprintf(stderr,"usage:cpu<string>\n");
exit(1);
}
char *str= argv[1];
while(1) { 
    Spin(1);
printf("%s\n",str);
}
return0;
}
```

esto es clave porque muestra la ilusion de que realmente hay muchas cpu cuando en verdad hay una nada más

### Virtualizacion de la memoria 

La memoria física de la maquina es un simple arreglo de bytes, en el cual para leer o escribir se tiene que especificar el espacio de la memoria, cuando un código accede a la memoria, primero pasa por una direccion virtual que con ayuda del sistema operativo se convierte a direccion física, al final lo que importa es que caa prceso tiene su propia espacio de direcciones privado, que el SO hace parecer grande y exclusivo pero en verdad todos los procesos comparten el mismo espacio

### Concurrency (concurrencia)

el problema cuando un proceso tiene multipes hilos, el problema que se crea es el de condiciones de carrera, por ejemplo si dos hilos modifican la misma variable mas o menos al mismo tiempo el resultado puede ser impredecible, basicamente se requiere una buena sincronizacion como locks o waits 

### Persistence (persistencia)

como logra que los datos de SO duren incluso cuando se apaga la maquina, a diferencia de la RAM que es volatil el SO tiene el file system  que es donde se guardan todos los datos del sistema operativo, esto aca decide como se organiza y se almacenan los archivos, los SO pueden estar estructurados desde listas simples a arboles complejos y se ocupan drivers para acceder a cosas del sistema operativo

### Design goals 

cosas que busca el sistema operativo.

- Abraccion: para que el sistema sea conveniente y facil de usar, permite escribir programas grandes dividiendolos en piezas comprensibles 

- alto rendimiento / minimizar overhead: la virtualización debe lograrse sin agregar demasiada sobrecarga

- proteccion y aislamiento: se debe evitar que un programa dañe a otro sea por malicia o por descuido

- reliability: debe funcionar sin parar, ya que si falla, fallara todas las aplicaciones dentro de ella

### Arquitectura de von neumann

- unidad central de procsamiento (CPU)
- memoria unica compartida
- Dispositivos I/O

lo central es que los programas y datos se almacenan en una misma memoria, por lo que los datos se pueden modificar dinamicamente, la CPU se conecta con un bus a la memoria y los dispositivos I/O y la CPU compiten por la memoria 

### Memoria 
es un arreglo unidimencional con celdas numeradas, cada celda almacena una unidad de informacion qeu puede contener una instruccion o un dato, la CPU genera direcciones numericas, estas son interpretadas por la unidad de control para leer o escribir la celda correspondiente 

### CPU
porvee un conjunto de instrucciones ejecutables, tiene una memoria interna volatil con un conjunto de registros:
- program counter
- istrucction register

la CPU hace fetch, el cual la unidad de control busca la proxima instruccion en la memoria despues el decode hace que el IR interpreta para determinar la proxima operacion y por ultimo se ejecuta (ALU, memoria, saltos o control de flujo)

Interacciones entre al CPU y I/O
- Programada: la cpu controla directamente la transferencia mediante instrucciones(polling), sistemas simples.
- interrupciones: la CPU delega la operacion y sigue con otras tarea, el dispositivo interrumpe cuando termina o necesita atencion
- directo a la memoria: se transfiere datos entre la memoria y un dispoitivo sin intervencion directa de la cpu, DMA es el que hace esto que actua como una mini CPU

entonces el SO es un dispositivo que administra recursos computacionales

### arquitecturas de SO
- kernel monolitico: unico programa ejecutable principal, si falla el kernel falla todo lo demas, ejemplo UNIX puede ser en modo usuario o kernel
- microkernel: muchas funciones tradicionales del nucleo se trasladan a servidores en espacio de usuario, si algo falla no compromete todo el sistema, modularidad
- arquitectura modular: kernel se extiende mediante moduos cargables, controladores de sistema de archivos
- hibrido: combina todos los otros

### Privilegios
- modo kernel: modo supervisor
- modo usuario: donde se ejecutan los programas del usuarios

### conclusiones 
- abstraccion de maquina virtual 
- abstraen la complejodad de hardware
- administra la asignacion de recursos 
- simplifica el desarrollo 
- mecanimos para contener tolerar y recuperar fallas de aplicaciones y datos

## Procesos

las intrucciones que un proceso lee o escribe residen en la memoria, los registros son que las instrucciones leen o modifican estos, hay especificos para cada tarea, el Program counter (que instruccion se esta ejecutando), stack counter (administra la pila).

### Operaciones de un SO

- Create: creacion de nuevos procesos (por ejemplo un comando en la shell)
- Destroy: forzar la terminacion de un proceso
- Wait: esperar a que un proceso termine
- Miscellaneous: controles de suspenciones y reanudacion de proceso
- Status: saber el estado de un proceso

### De programa a proceso 
El sistema operativo lee el ejecutable y lo coloca en espacio de direcciones del proceso, antes se cargaba todo de a una, ahora solo se va haciendo por partes del código.

El SO reserva espacio en la pila de run-time usando las variables locales, parametros de la funcion y direcciones de retorno, la asignacion del heap es para datos asignados dinamicamente (malloc() y free()), el heap empieza pequeño y crece conforme el código o el programa lo solicita. En el sistema UNIX el proceso recibe 3 descriptores por defecto que es un ticket que tiene un valor numerico, por ejemplo entrada estandar (standard input) que es para leer datos que entran al programa, standard output donde el programa imprime si output normal como un printf pro ejemplo, standard error, domde el programa imprime el mensaje de error.

### Estados de un proceso 
- Running: el proceso esta ejecutandose actualmente en el procesador
-  Ready: esta listo pero el SO no lo esta ejecutando todavia
- Blocked: el proceso realizo alguna operacion que lo deja no listo hasta que ocurra algun evento.

el sistema operativo necesita una esrtructura de datos para llevar el registro de los procesos

- Process List: registro de todos los procesos en el sistema y la informacion sobre la cual esta corriendo
- PCB: estructura individual en C que almacena toda la informacion de un proceso individual.

## system calls
### Fork()

se usa ára crear un proceso nuevo, el proceso eso si es una copia del proceso padre, el tema es que el valor de el proceso hijo retorna algo distinto al proceso padre, el hijo retorna 0 y el padre el PID del hijo

### Wait()

el proceso padre necesita, a veces, necesita esperar que el proceso hijo termine para poder continuar, para eso usamos wait(), hace que un proceso se bloquee hasta que otro proceso termina 

### Exec()

se usa para que un proceso ejecute un programa distinto al que lo creo, es decir transforma el proceso actual en otro

