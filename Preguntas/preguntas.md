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
