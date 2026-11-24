# 04. Anexo: paginación, TLB, EAT e i-nodos (Prueba 3)

> La Prueba 3 **no tiene código**: son cálculos. Aquí se practican como **procedimientos** (pasos fijos, como un algoritmo). Escribe siempre el paso intermedio (el binario, la fórmula con los números), porque la pauta corrige el procedimiento.

---

## Parte 1. Explicación

### 1.1 Descomponer una dirección (10 | 10 | 12)

Con direcciones de 32 bits, páginas de 4 KB (2^12 → **12 bits de offset**) y tabla de 2 niveles:

```
| 31 ........ 22 | 21 ........ 12 | 11 ............ 0 |
|  dir (10 bits) | tabla (10 bits)|  offset (12 bits) |
```

Procedimiento:

1. Pasar el hex a binario (4 bits por dígito).
2. Cortar 10 | 10 | 12 desde la izquierda.
3. Pasar cada parte a decimal (o hex).

Ejemplo: `0x00403ABC`

```
0    0    4    0    3    A    B    C
0000 0000 0100 0000 0011 1010 1011 1100

dir    = 0000000001          = 1
tabla  = 0000000011          = 3
offset = 1010 1011 1100      = 0xABC
```

Atajo útil: el offset son los **3 últimos dígitos hex** (12 bits = 3 dígitos). Los 5 primeros dígitos (20 bits) son el número de página virtual (VPN).

**Preguntas:**

1. ¿Por qué con páginas de 4 KB el offset tiene 12 bits?
2. ¿Por qué se usa una tabla de 2 niveles en vez de una de 1 nivel con 2^20 entradas? Calcula el tamaño de la tabla de 1 nivel con entradas de 4 bytes.
3. Explica la diferencia entre **TLB** y **tabla de páginas**: qué guarda cada una, dónde vive, quién la consulta primero.
4. ¿Por qué con paginación **no existe** fragmentación externa? ¿Qué fragmentación sí existe?

### 1.2 EAT (tiempo efectivo de acceso)

Notación: `α` = tasa de acierto en TLB, `t_tlb` = tiempo de TLB, `t_mem` = tiempo de un acceso a memoria.

- **Tabla de 1 nivel con TLB:**
  `EAT = α·(t_tlb + t_mem) + (1 − α)·(t_tlb + 2·t_mem)`
- **Tabla de 2 niveles con TLB:** en un fallo de TLB hay que leer **2** entradas de tabla + el dato:
  `EAT = α·(t_tlb + t_mem) + (1 − α)·(t_tlb + 3·t_mem)`
- **Con faltas de página** (`p` = tasa de falta, `t_fault` = tiempo de atender la falta):
  `EAT = (1 − p)·t_acceso + p·t_fault`

**Preguntas:**

5. ¿Por qué en el fallo de TLB con 2 niveles son 3 accesos a memoria y no 2?
6. ¿Por qué incluso una tasa de faltas de página de 1/1000 dispara el EAT? (Compara órdenes de magnitud: ns vs ms.)

### 1.3 i-nodos

Un i-nodo típico tiene `D` punteros directos, 1 indirecto simple, 1 doble y 1 triple. Con bloques de `B` bytes y punteros de `p` bytes, cada bloque de punteros guarda `k = B / p` punteros.

```
Tamaño máximo = (D + k + k² + k³) · B
```

Accesos a disco para leer un byte (con el i-nodo ya en memoria):

- Zona directa: 1 (el bloque de datos).
- Indirecta simple: 2.
- Doble: 3.
- Triple: 4.

**Preguntas:**

7. ¿Por qué el i-nodo no guarda punteros directos para todos los bloques?
8. ¿Qué cambia en los accesos si el i-nodo **no** está en memoria?

---

## Parte 2. Ejercicios segmentados

### 2.1 Descomponer

Descompón en 10 | 10 | 12 (mostrando el binario): `0x0080F123`, `0xC0001FFF`, `0x12345678`.

```
Tu respuesta:
```

### 2.2 Traducir

Directorio de páginas (nivel 1) y tablas (nivel 2) de un proceso. Los números de marco están en hex.

```
Directorio:           Tabla en marco 0x0A1:      Tabla en marco 0x0B7:
idx 0 → inválida      idx 0 → marco 0x3F2        idx 0 → inválida
idx 1 → marco 0x0A1   idx 3 → marco 0x111        idx 1 → marco 0x500
idx 2 → marco 0x0B7   idx 4 → inválida           idx 15 → marco 0x2C4
```

Traduce a dirección física (o indica falta de página y en qué nivel):

- (a) `0x00403ABC`
- (b) `0x00404010`
- (c) `0x0080F123`
- (d) `0x00001000`

```
Tu respuesta:
```

### 2.3 EAT

- (a) `t_tlb = 10 ns`, `t_mem = 100 ns`, `α = 0,9`, tabla de 1 nivel.
- (b) Lo mismo con tabla de 2 niveles.
- (c) Sobre el resultado de (b), con `p = 0,0001` y `t_fault = 8 ms`. ¿Cuántas veces más lento que sin faltas?
- (d) Con 2 niveles, ¿qué `α` mínimo necesitas para que el EAT sea ≤ 130 ns?

```
Tu respuesta:
```

### 2.4 i-nodo

Bloques de 4 KB, punteros de 4 bytes, 12 directos + 1 simple + 1 doble + 1 triple.

- (a) `k` = ?
- (b) Tamaño máximo de archivo (exprésalo en potencias de 2 y en GB/TB aproximados).
- (c) ¿Cuántos accesos a disco para leer el byte en la posición `40 000`? ¿Y en la `5 000 000`? ¿Y en la `2 000 000 000`? Indica en qué zona cae cada uno.

```
Tu respuesta:
```

---

## Parte 3. Ejercicio tipo examen (reconstruido)

> Tiempo sugerido: 30 min.

Un sistema usa direcciones virtuales de 32 bits, páginas de 4 KB, tabla de 2 niveles (10 | 10 | 12) con entradas de 4 bytes, y una TLB con tasa de acierto del 95 %. `t_tlb = 5 ns`, `t_mem = 80 ns`.

- **(a)** Para la dirección `0x00C02F0A`, indica los índices de directorio y de tabla y el offset (con el binario).
- **(b)** Si el directorio en el índice obtenido apunta al marco `0x1F0` y la tabla en el índice obtenido apunta al marco `0x7A3`, ¿cuál es la dirección física?
- **(c)** Calcula el EAT sin faltas de página.
- **(d)** Si un proceso usa sólo 3 regiones de 4 MB cada una, ¿cuánta memoria ocupan sus tablas de páginas con 2 niveles? Compara con una tabla de 1 nivel.
- **(e)** El sistema de archivos usa bloques de 4 KB, punteros de 4 bytes y un i-nodo con 10 directos + simple + doble. ¿Tamaño máximo de archivo? ¿Cuántos accesos para leer el byte `1 000 000` si el i-nodo ya está en memoria?

```
Tu respuesta:
```
