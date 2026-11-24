### Los procesos cooperativos 
Son los procesos que comparten espacios como los threads, el problema esta cuando los threads comparten el mismo espacio de memoria y entran a modificar un dato de manera concurrente, eso se llama condicion de carrera, el cual depende del scheduler y no es determinista. 

##### Problema del productor y consumidor
Lo que hace es que el productor y el consumidor actuan sobre dos cosas compartidas, el buffer doinde almacenan las variables, y el contador donde revisa la cantidad de elementos que estan dentro del buffer. El problema esta en que como necesitan acceder al contador y puede ser mas o menos al mismo tiempo, puede que el productor y el consumidor entren a modificar el contador al mismo tiempo, lo cual genera una condicion de carrera.

El tema es que la atomicidad de la instruccion del counter++ y counter-- puede cortarse en la mitad del assembly. entonces al acceder al contador al mismo tiempo la instruccion de assembly queda a la mitad 


### Condicion de carrera
Cuando varios procesos manupulan un mismo dato compartido concurrentemente, el tema es que puede pasar lo que definimos antes por eso hay que manipular bien sincronizado los datos.

### La seccion critica
Es la parte del codigo donde se accede a los datos compartidos, y es importante que solo un proceso pueda acceder a esa seccion critica a la vez, para evitar la condicion de carrera, para eso se diseña una seccion de entrada otra que es la parte critica y finalmente la de salida


#### Requisitos de solucion de la condicion de carrera
1. Exclusion mutua: Solo un proceso puede estar en la seccion critica a la vez.
2. Progreso: Si ningun proceso esta en la seccion critica, y hay procesos que quieren entrar, entonces uno de ellos debe ser elegido para entrar a la seccion critica.
3. Espera limitada: Un proceso no puede esperar indefinidamente para entrar a la seccion critica, debe haber un limite de tiempo para que pueda entrar.

#### Propiedades deseables de una solucion (no obligatorias pero valoradas)

1. Justicia: Todos los procesos deben tener la misma oportunidad de entrar a la seccion critica, no puede haber favoritismo.
2. Escalabilidad: La solucion debe poder adaptarse a un numero mayor de procesos sin perder eficiencia.

### Semaforos
Es un algoritmo de sincronizacion que permite a los procesos no caerse en las secciones criticas, se controla mediante una variable S con wait() y signal(), el tema es que coordina los procesos para que no entren a la seccion critica al mismo tiempo, y si un proceso entra a la seccion critica, los demas procesos deben esperar hasta que el proceso que esta en la seccion critica salga de ella. no necesariamente el valor de S vale 0 o 1, pude vales menos que cero y asi coordina la entrada de los procesos.

Estan los spinlocks que es lo que acabamos de describir, los mutez son los que pueden tomas valores como 0 y 1

hay semaforos que hacen dormir el proceso que los valores negativos dicen cuantos procesos esperando para entrar a la seccion critica y los deja durmiendo para no ocupar CPU.

#### Locks

Es un proceso que especifica el modo, multiples pueden tener el modo lectura y entrar a la seccion critica, pero si un proceso tiene el modo escritura, los demas procesos no pueden entrar a la seccion critica.

Un proceso debe adquirir el lock antes de entrar a la seccion critica y liberarlo al salir. Para implementar una exclusion mutua el hardware puede deshabilitar las interrupciones que es desde el kernel, pero no funciona con multiprocesadores, dado que un procesador no puede deshabilitar las interrupciones 

#### Test and Set
Es una sola instruccion atomica que lee lo que hay en el target y escribe un true y retorna lo que habia antes y la interpretacion del retorno es lo que da el lock, si retorna false estaba libre y queda en true para los demas, si estaba en true es que esta ocupado y sigue girando



### variable de condicion
La variable de condicion es una cola que permite a un thread avisar a otros thread qeu el estado del sistema cambio, y los procesos duermen en esa cola sin gastar la CPU, hay un mutex que protege a la cola de procesos dormidos. Si es Hoare de un proceso a otro se le entrega en la mano el lock, T2 se suspende de inmediato y entrega el mutex a T1, corre inmediatamente como delo el estado el T2, cuando T1 termina T2 toma el control nuevamente y continua.

La condicion Mesa es que te aviso pero sigo con lo mio.
cuando T2 hace signal(), T1 pasa a la cola de CV a la de listos, T2 sigue ejecutando con el mutex hasta que lo libere cuando le de la gana, cuando el scheduler hace el cambio de contexto T2 suelta el mutex y los procesos que estan en la cola de los ready se pelean por el mutex. 
el wait siempre esta dentro de un while nunca de un if, pthread_cond_wait(&c, &m) hace dos cosas atómicamente: libera m y duerme. Y al despertar, readquiere m antes de retornar. Si esto no fuera atómico, el señalizador podría hacer signal entre "libero el mutex" y "me duermo", la señal se perdería (nadie estaba en la cola) y el thread dormiría para siempre.
