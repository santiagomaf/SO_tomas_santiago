# 03. Sockets en Python (Prueba 4)

> Peso en el examen: 1 de los 4 problemas de código. Formato típico: receptor "minishare" con `recv_line`, `receive_file` y ciclo `accept` con threads, más explicaciones.

---

## Parte 1. Explicación

### 1.1 Servidor y cliente TCP

```python
import socket

# SERVIDOR
srv = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
srv.bind(("", 5000))          # "" = todas las interfaces
srv.listen()
while True:
    sock, addr = srv.accept() # socket NUEVO por cada conexión
    data = sock.recv(1024)
    sock.sendall(b"OK\n")
    sock.close()

# CLIENTE
c = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
c.connect(("127.0.0.1", 5000))
c.sendall("hola\n".encode("utf-8"))
resp = c.recv(1024).decode("utf-8")
c.close()
```

Reglas:

- El socket de escucha (`srv`) **nunca** transporta datos; sólo produce sockets conectados con `accept`.
- `sendall(data)` envía **todo** (reintenta internamente). `send(data)` puede enviar **menos** y retorna cuántos bytes envió.
- `recv(n)` retorna **entre 1 y n bytes**, o `b""` si el otro extremo **cerró** la conexión.
- Los sockets trabajan con `bytes`, no con `str`: `"txt".encode("utf-8")` y `b.decode("utf-8")`. `b"\n"` no es igual a `"\n"`.

**Preguntas:**

1. ¿Qué pasa en el cliente si nadie escucha en el puerto? Explica en términos del handshake de 3 vías.
2. ¿Para qué sirve `SO_REUSEADDR`? ¿Qué error ves sin él al reiniciar el servidor rápido?
3. Con 3 clientes conectados, ¿cuántos sockets tiene abiertos el servidor? ¿Cómo distingue el SO a qué socket va cada segmento (cuádrupla)?

### 1.2 Framing: delimitar mensajes

TCP es un **flujo de bytes sin fronteras**. Un `sendall(b"OFFER 10 a.txt\n")` del emisor puede llegar como `recv` → `b"OFF"` y otro `recv` → `b"ER 10 a.txt\n0123"` (ya mezclado con el archivo que viene después).

Dos opciones:

- **Delimitador**: cada mensaje termina en `\n`.
- **Prefijo de largo**: primero un largo fijo (p. ej., 4 bytes big-endian) y después exactamente ese número de bytes.

```python
def recv_line(sock, max_len=1024):
    # tú la escribes en la Parte 2
    ...

line = recv_line(sock)                 # "OFFER 1234 mi archivo.txt"
parts = line.split(" ", 2)             # ["OFFER", "1234", "mi archivo.txt"]
size = int(parts[1])                   # ValueError si no es número
name = os.path.basename(parts[2])      # quita "../../" y rutas
```

**Preguntas:**

4. ¿Por qué `recv_line` lee **de a 1 byte** y no con `recv(1024)`? ¿Qué bytes se perderían?
5. ¿Por qué `split(" ", 2)` y no `split()`?
6. ¿Qué ataque evita `os.path.basename`? Da un nombre de archivo malicioso.
7. ¿Por qué se limita el largo de la línea?

### 1.3 Recibir exactamente N bytes

```python
remaining = size
while remaining > 0:
    chunk = sock.recv(min(BLOCK, remaining))
    if chunk == b"":
        raise ConnectionError("cerrado antes de tiempo")
    f.write(chunk)
    remaining -= len(chunk)
```

**Preguntas:**

8. ¿Por qué `sock.recv(size)` en una sola llamada no basta?
9. ¿Qué pasa si quitas el `if chunk == b""`?
10. ¿Por qué `min(BLOCK, remaining)` y no sólo `BLOCK`?
11. ¿Por qué se escribe bloque a bloque y no se junta todo en un `bytes` y se escribe al final?

### 1.4 Concurrencia y limpieza

```python
import threading

while True:
    sock, addr = srv.accept()
    t = threading.Thread(target=handle_conn, args=(sock, addr), daemon=True)
    t.start()
```

```python
def handle_conn(sock, addr):
    try:
        ...
    except (ConnectionError, ValueError) as e:
        ...           # borrar archivo incompleto
    finally:
        sock.close()  # siempre
```

**Preguntas:**

12. Sin threads, ¿qué le pasa al segundo cliente mientras el primero envía un archivo grande? ¿Dónde queda (en qué cola)?
13. `args=(sock, addr)` y no `args=(sock)`: ¿por qué la coma importa?
14. ¿Qué significa `daemon=True`?
15. ¿Por qué el `close` va en `finally` y no al final del `try`?

### 1.5 UDP

```python
# SERVIDOR
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.bind(("", 6000))
data, addr = s.recvfrom(1024)   # addr = (ip, puerto) del emisor
s.sendto(b"PONG", addr)

# CLIENTE
c = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
c.sendto(b"PING", ("127.0.0.1", 6000))
data, addr = c.recvfrom(1024)

# BROADCAST
c.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
c.sendto(b"HOLA", ("<broadcast>", 6000))
```

**Preguntas:**

16. ¿Por qué un cliente UDP "funciona" (no lanza excepción en `sendto`) aunque no haya servidor?
17. ¿Por qué UDP conserva las fronteras de mensaje y TCP no?
18. ¿Por qué broadcast sólo existe en UDP y no sale de la subred?
19. ¿Qué pasa si el cliente UDP llama `recvfrom` y el datagrama se perdió? ¿Cómo lo evitas? (`settimeout`)

---

## Parte 2. Ejercicios segmentados

Protocolo **minishare** (receptor = servidor):

```
Emisor  → Receptor : "OFFER <size> <name>\n"
Receptor → Emisor  : "ACCEPT\n"   o   "REJECT <motivo>\n"
Emisor  → Receptor : <size bytes del archivo>        (sólo si ACCEPT)
Receptor → Emisor  : "DONE\n"
```

Constantes y API disponibles:

```python
import os, socket, threading
PORT = 5000
BLOCK = 4096
MAX_LINE = 1024
MAX_SIZE = 100 * 1024 * 1024
DEST_DIR = "recibidos"

socket.socket(family, type)
sock.setsockopt(level, optname, value)
sock.bind(address); sock.listen(); sock.accept() -> (sock, addr)
sock.connect(address)
sock.recv(n) -> bytes          # b"" si el otro extremo cerró
sock.sendall(data: bytes)
sock.close()
os.path.basename(path); os.path.join(a, b); os.remove(path)
threading.Thread(target=..., args=(...), daemon=True).start()
```

### 2.1 `recv_line`

Lee hasta `\n` (sin incluirlo) y retorna un `str`. Lanza `ConnectionError` si la conexión se cierra antes del `\n`, y `ValueError` si la línea supera `MAX_LINE` bytes.

```python
def recv_line(sock) -> str:
```

```python
# Tu respuesta:
```

### 2.2 `parse_offer`

Recibe la línea (sin `\n`) y retorna `(size, name)`. Lanza `ValueError` si: no empieza con `OFFER`, faltan campos, `size` no es entero, `size < 0` o `size > MAX_SIZE`, o el nombre queda vacío después de `basename`.

```python
def parse_offer(line: str) -> tuple[int, str]:
```

```python
# Tu respuesta:
```

### 2.3 `receive_file`

Recibe **exactamente** `size` bytes del socket y los escribe en `path` por bloques de `BLOCK`. Lanza `ConnectionError` si se cierra antes. El archivo debe quedar cerrado aunque haya excepción.

```python
def receive_file(sock, path: str, size: int) -> None:
```

```python
# Tu respuesta:
```

### 2.4 Ciclo de `main`

Crea el socket de escucha y atiende cada conexión en un thread que ejecuta `handle_conn(sock, addr)`. Crea `DEST_DIR` si no existe.

```python
def main():
```

```python
# Tu respuesta:
```

### 2.5 `handle_conn` completo

Lee el OFFER, valida (si es inválido responde `REJECT` y cierra), responde `ACCEPT`, recibe el archivo, responde `DONE`. Si la transferencia falla a la mitad, **borra el archivo incompleto**. El socket se cierra siempre.

Pregunta extra: si el archivo ya existe en `DEST_DIR`, ¿qué harías y en qué punto del protocolo?

```python
def handle_conn(sock, addr) -> None:
```

```python
# Tu respuesta:
```

### 2.6 UDP PING / PONG

Servidor UDP en el puerto 6000 que responde `b"PONG"` a cada `b"PING"` usando la dirección del emisor; ignora cualquier otro mensaje. Cliente que envía `PING`, espera la respuesta con un timeout de 1 s y reintenta hasta 3 veces.

```python
def udp_server(): ...
def udp_client(host: str) -> bool: ...   # True si recibió PONG
```

```python
# Tu respuesta:
```

### 2.7 Framing con prefijo de largo

Cambia el framing: cada mensaje de control va precedido de **4 bytes big-endian** con su largo. Escribe `recv_exact(sock, n)` y `recv_msg(sock)`, y `send_msg(sock, text)`.

```python
int.from_bytes(b, "big")        # bytes -> int
n.to_bytes(4, "big")            # int -> 4 bytes

def recv_exact(sock, n: int) -> bytes: ...
def recv_msg(sock) -> str: ...
def send_msg(sock, text: str) -> None: ...
```

Pregunta: ¿qué ventaja y qué riesgo tiene frente al `\n`? (Pista: ¿qué pasa si el emisor manda un largo de 4 GB?)

```python
# Tu respuesta:
```

---

## Parte 3. Ejercicios tipo examen

### 3.1 Prueba 4 original (reconstruida): receptor minishare

> Tiempo sugerido: 30 min.

Con el protocolo y la API de la Parte 2, implementa el receptor completo:

- **(a)** `recv_line(sock)`. Explica por qué lee de a un byte.
- **(b)** `receive_file(sock, path, size)`. Explica por qué una sola llamada `recv(size)` no basta.
- **(c)** `main()` con el ciclo `accept` y un thread por conexión, y `handle_conn`. Explica qué pasaría con un segundo emisor si no usaras threads.

```python
# Tu respuesta:
```

### 3.2 Variante A: el emisor de minishare

Escribe el **cliente** que envía un archivo:

```python
def send_file(host: str, port: int, path: str) -> bool:
    """Retorna True si el receptor respondió DONE."""
```

- Conecta, envía `OFFER <size> <basename>\n` (`os.path.getsize(path)`).
- Espera la respuesta con `recv_line`. Si es `REJECT ...`, imprime el motivo y retorna `False`.
- Si es `ACCEPT`, abre el archivo en `"rb"` y lo envía **por bloques** con `sendall`.
- Espera `DONE`. Cualquier otra cosa (o cierre) → `False`.
- El socket se cierra siempre; maneja `ConnectionRefusedError`.

Pregunta: ¿por qué el emisor **no** puede mandar el archivo inmediatamente después del OFFER, sin esperar el ACCEPT?

```python
# Tu respuesta:
```

### 3.3 Variante B: servidor de comandos con varias líneas por conexión

Un servidor TCP mantiene un diccionario compartido `store = {}` entre todas las conexiones. Cada cliente envía **varias** líneas por la misma conexión:

```
SET <clave> <valor>   → "OK\n"
GET <clave>           → "VALUE <valor>\n"  o  "NOTFOUND\n"
DEL <clave>           → "OK\n"  o  "NOTFOUND\n"
QUIT                  → "BYE\n" y se cierra la conexión
otra cosa             → "ERR\n"
```

- Un thread por conexión; el ciclo de cada thread termina con `QUIT` o cuando el cliente cierra (`ConnectionError` de `recv_line`).
- `store` es **compartido**: protégelo con `threading.Lock()` (`with lock:`). Agrega además `INCR <clave>` (suma 1 a un valor entero; si no existe, parte en 0). Muestra con dos threads la intercalación que **pierde un incremento** si `INCR` no tiene lock (lectura → suma → escritura), y explica por qué un `SET` aislado es menos peligroso que un `INCR`.
- `<valor>` puede tener espacios.

```python
# Tu respuesta:
```
