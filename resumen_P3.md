# Resumen Prueba 3

la memoria fisica se corta en marcos, para simular esto la memoria virtual hace lo mismo mediante un proceso que se llama paginacion, existe una tabla de paginas que dice en que marco se encuentra cada pagina, esto elimina lo que es la fragmentacion extrna.
> El offset es la parte de la direccion virtual que indica la posicion exacta dentro de la pagina donde se encuantra el dato.

El espacio logico es de $2^{m}$, esto quiere decir si el espacio logico tiene 16 direcciones nececita $16 = 2^4$, por lo tanto $m = 4$, osea 4 bits para direccionar.

La pagina es parecido es $2^{n}$ es el tamaño de la pagina, si el tamaño de la pagina es 4, entonces $4 = 2^2$, por lo tanto $n = 2$, osea 2 bits para direccionar.


${direccion virtual} = (m-n) + n$

Esta es la cantidad de bits que se necesitan para direccionar la memoria virtual, $(m-n)$ son los bits que te dice cual es la pagina y $n$ son los bits que te dicen el offset.

${direccion fisica} = ({tamaño} * {Marco}) + {offset}$

---

# PARTE A: Cheat sheet para la guía de ejercicios

## 0. Potencias de 2 que hay que saberse

| Potencia | Valor | Potencia | Valor |
|---|---|---|---|
| $2^{10}$ | 1 KB | $2^{30}$ | 1 GB |
| $2^{12}$ | 4 KB | $2^{11}$ | 2 KB |
| $2^{20}$ | 1 MB | $2^{9}$ | 512 B |

- Tamaño en potencia de 2: `4 KB = 2^2 · 2^10 = 2^12`, `1 GB = 2^30`, `2 GB = 2^31`.
- Una página de 4 KB usa **12 bits** de offset.
- Hex: 1 dígito hex = 4 bits. 12 bits de offset = **3 dígitos hex**.
- Multiplicar por 4096 en hex = **correr 3 dígitos a la izquierda** (`0x1A2 · 4096 = 0x1A2000`).

---

## 1. PAGINACIÓN

### 1.1 Bits y tamaños
Dado: dirección virtual de $m$ bits, página de $2^n$ bytes, memoria física de $F$ bytes.

| Qué | Fórmula | Ejemplo (32 bits, 4 KB, 1 GB) |
|---|---|---|
| Bits de offset | $n = \log_2(\text{tamaño página})$ | 12 |
| Bits de nº de página virtual | $m - n$ | 20 |
| Nº de páginas virtuales | $2^{m-n}$ | $2^{20}$ |
| Nº de marcos | $F / 2^n$ | $2^{30}/2^{12} = 2^{18}$ |
| Bits para numerar marcos | $\log_2(\text{nº marcos})$ | 18 |
| Entradas de tabla lineal | $2^{m-n}$ (se usa el espacio **virtual**) | $2^{20}$ |
| Tamaño tabla lineal | entradas × bytes por entrada | $2^{20} · 4 = 4$ MB |

> La RAM física **no** cambia el nº de entradas de la tabla, solo cuántos bits tiene el nº de marco.
> Una tabla lineal ocupa lo mismo siempre, use poca o mucha memoria el proceso.

### 1.2 Traducir una dirección (un nivel)
1. Separar: $p$ = bits altos ($m-n$), $d$ = bits bajos ($n$).
   - Sin binario: $p = \lfloor dir / tam\_pag \rfloor$, $d = dir \bmod tam\_pag$.
   - En hex: los últimos 3 dígitos son $d$ (si la página es 4 KB), el resto es $p$.
2. Buscar en la tabla: $p \to$ marco $f$.
3. **Física** $= f \cdot tam\_pag + d$.

Ejemplo guía (ej. 7.3): `0x0003C7F0` → $d$ = `0x7F0`, $p$ = `0x0003C`. Marco `0x1A2` → física = `0x1A27F0`.

### 1.3 Dos niveles (32 bits, 4 KB): 10 | 10 | 12
```
[ índice directorio (10) | índice tabla (10) | offset (12) ]
```
- Directorio: $2^{10}$ = 1024 entradas × 4 B = **4 KB** (1 página).
- Cada tabla de 2º nivel: también 1024 entradas × 4 B = **4 KB**.
- Una tabla de 2º nivel cubre $1024 · 4\text{ KB} = $ **4 MB** de espacio virtual.
- Un directorio cubre $1024 · 4$ MB = 4 GB.

**Método para descomponer una dirección hex de 32 bits:**
1. Offset = últimos 3 dígitos hex.
2. Pasar los primeros 5 dígitos hex (20 bits) a binario.
3. Los 10 bits altos = directorio, los 10 siguientes = tabla.

Ejemplo: `0x00801234` → `0000000010 | 0000000001 | 001000110100` → dir `0x002`, tabla `0x001`, offset `0x234`.
Con marco `0x1C5`: física = `0x1C5234`.

`0xFFC00010` → dir `0x3FF`, tabla `0x000`, offset `0x010`.

Traducción: directorio[dir] → tabla de 2º nivel; tabla[idx] → marco; física = marco·4096 + offset.

**¿Por qué cada tabla ocupa exactamente 1 página?** Se asigna/libera como cualquier marco (sin fragmentación), se puede mandar a disco, y 10 bits recorren justo 1024 entradas.

### 1.4 Memoria que ocupan las tablas (ej. 9)
$$\text{Memoria tablas} = 4\text{ KB (directorio)} + (\text{nº regiones de 4 MB tocadas}) \cdot 4\text{ KB}$$

- Contar **regiones de 4 MB distintas** donde hay algo mapeado (aunque sea 1 byte).
- Ejemplo: 20 MB desde 0 → 5 tablas; 512 KB de pila arriba → 1 tabla. Total = 4 + 6·4 = **28 KB** (vs 4 MB lineal, ~146 veces más).
- Mapear 100 KB en otra región nueva → **+1 tabla (4 KB)**, sin importar lo chica que sea.
- Idea clave: el costo depende de la memoria usada, no del tamaño del espacio virtual (procesos usan el espacio disperso: código abajo, pila arriba, hueco enorme en medio). Las entradas de directorio sin uso quedan **inválidas** y sin tabla.

### 1.5 TLB y tiempo efectivo de acceso (EAT)
Notación: $h$ = hit ratio, $t_m$ = acceso a memoria, $t_{TLB}$ = consulta TLB.

**General:**
$$EAT = h\,(t_{TLB} + t_m) + (1-h)\,(t_{TLB} + k\cdot t_m + t_m)$$
donde $k$ = nº de niveles de tabla (accesos extra para recorrer tablas).

**Si el TLB es instantáneo ($t_{TLB}=0$)** (como en el ej. 10):
- Acierto: $1 \cdot t_m$
- Fallo con 1 nivel: $2 \cdot t_m$ (tabla + dato)
- Fallo con 2 niveles: $3 \cdot t_m$ (directorio + tabla + dato)

$$EAT_{2\,niveles} = h \cdot t_m + (1-h)\cdot 3t_m$$

Ejemplo ($t_m$ = 80 ns, $h$ = 0.98): 
- 2 niveles: $0.98·80 + 0.02·240 = 83.2$ ns
- 1 nivel: $0.98·80 + 0.02·160 = 81.6$ ns
- costo del nivel extra: 1.6 ns (2 %).

Ejemplo clase 07 ($t_{TLB}$=20, $t_m$=100): hit = 120 ns, miss = 220 ns. $h$=0.8 → 140 ns; $h$=0.98 → 122 ns.

### 1.6 EAT con faltas de página
$$EAT = (1-p)\cdot t_{acceso} + p \cdot t_{falta}$$
- $p$ = prob. de falta de página por acceso, $t_{falta}$ = tiempo de atenderla (ej. 8 ms = 8 000 000 ns).
- Para que $EAT \le L$: 
$$p \le \frac{L - t_{acceso}}{t_{falta} - t_{acceso}}$$
- Ej.: $L$=100, $t_{acceso}$=83.2, $t_{falta}$=8·10⁶ → $p \le 2.1·10^{-6}$ (≈ 1 falta cada 476 000 accesos).
- **Conclusión:** un fallo de TLB cuesta ns, una falta de página ms (6 órdenes de magnitud). El TLB puede fallar 2 % sin problema; las faltas de página deben ser rarísimas, por eso importan el reemplazo y el thrashing.

### 1.7 Bits de la PTE y manejo de una falta (ej. 11)
- **V** (válido/presente): la página está en un marco.
- **A** (accedida/referencia): el hardware lo pone en 1 al usarla. Sirve para LRU aproximado y working set.
- **D** (sucia/dirty): el hardware lo pone en 1 al **escribir**. 
- Bits de protección (R/W/X).

**Distinguir 3 casos al acceder:**
| Caso | Qué pasa |
|---|---|
| TLB hit | traducción directa, 1 acceso a memoria |
| TLB miss, PTE con V=1 | recorrer tablas (3 accesos en 2 niveles), cargar al TLB, **no hay falta de página** |
| PTE con V=0 pero la página **pertenece** al proceso (en swap / nunca cargada) | **falta de página**: el SO la carga desde disco |
| Página que **no pertenece** al proceso | **inválida**: el SO termina el proceso (SIGSEGV) |

**Pasos del SO ante una falta de página** (clase 08):
1. Verificar en el PCB que la referencia es válida; si no, terminar el proceso.
2. Buscar marco libre.
3. Si no hay: ejecutar algoritmo de reemplazo → elegir víctima.
   - Víctima con D=1 → hay que **escribirla a swap** (1 E/S extra). 
   - Víctima con D=0 (limpia) → se reutiliza el marco sin escribir.
   - Poner V=0 en la PTE de la víctima e **invalidar su entrada en el TLB**.
4. Programar lectura desde disco al marco (el proceso se bloquea, el planificador corre otro).
5. Al terminar la E/S: PTE con nº de marco, V=1 (A=0, D=0).
6. Reiniciar la instrucción: MMU traduce de nuevo, carga TLB; el hardware pone A=1 (y D=1 si fue escritura).

Por eso los algoritmos prefieren víctimas **limpias**. El dirty bit ahorra la escritura.

### 1.8 Algoritmos de reemplazo
Se cuentan solo las **faltas** (las referencias que no están en los marcos). Simular escribiendo los marcos tras cada referencia.

| Algoritmo | Víctima | Belady | Notas |
|---|---|---|---|
| FIFO | la que lleva más tiempo **cargada** | **Sí sufre** | simple; no mira el uso |
| OPT | la que **no se usará por más tiempo** | No | necesita futuro; es la referencia de comparación (mínimo de faltas) |
| LRU | la **menos recientemente usada** | No | aproxima OPT; implementación: contadores o stack |

- **Anomalía de Belady:** con FIFO, más marcos puede dar **más** faltas (ej. string `1,2,3,4,1,2,5,1,2,3,4,5`: 3 marcos → 9 faltas, 4 marcos → 10).
- OPT y LRU son **algoritmos de pila**: el conjunto con $n$ marcos ⊆ el de $n+1$ marcos, por eso no tienen Belady.
- Para FIFO con hit: **no** se reordena la cola. Para LRU con hit: la página pasa a ser la más reciente.
- Un string de referencia basta porque el reemplazo solo depende de **qué página** se toca y en qué orden, no del offset.

**Resultado del ej. 12 (3 marcos):** FIFO = 9, LRU = 10, OPT = 7. (LRU peor que FIFO acá porque el patrón cíclico sobre más páginas que marcos es su peor caso.)

**Cómo simular OPT:** ante una falta sin marcos libres, mirar hacia adelante y sacar la página cuyo próximo uso está más lejos (o que no se usa más).

### 1.9 Asignación de marcos
- Igualitaria: $m/n$ marcos por proceso (el resto va a un pool libre).
- Proporcional: $a_i = \dfrac{s_i}{S} \cdot m$, con $S=\sum s_i$.
- **Global:** el reemplazo puede sacar marcos de otros procesos (el proceso no controla su tasa de faltas; puede causar thrashing).
- **Local:** solo reemplaza dentro de sus marcos (puede desperdiciar marcos poco usados).

### 1.10 Thrashing y working set (ej. 13)
- **Thrashing:** el proceso pasa más tiempo paginando que ejecutando.
- **Criterio de riesgo:** $\sum WS_i > \text{marcos disponibles}$.
  - Ej.: 300+300+250+350 = 1200 > 1024 → hay riesgo.
- Síntomas: CPU baja, disco saturado. Se **realimenta**: CPU ociosa → el SO sube el grado de multiprogramación → más faltas.
- Soluciones:
  1. **Suspender un proceso completo** (bajar multiprogramación): ej. sacar el de 350 → quedan 850 ≤ 1024. Costo: latencia de ese proceso.
  2. **Control de admisión:** dar a cada proceso al menos su working set y admitir nuevos solo si sobran marcos. Previene en vez de curar.
  3. Reemplazo local o por prioridad (aminora el efecto).
- El working set se mide con los bits de referencia (A) de las PTE.

### 1.11 Otros conceptos de paginación
- **Fragmentación interna:** solo en la última página. Peor caso $2^n - 1$ bytes, promedio media página. Externa: no existe.
- **Protección:** bits R/W/X + bit V/I. Limitación: el límite es de granularidad de página (el proceso puede tocar el resto de su última página).
- **Páginas compartidas:** código **reentrante** (puro); datos/stack/heap privados.
- **PTBR:** registro que apunta a la tabla de páginas; un cambio de contexto solo cambia el PTBR. En Pentium es **CR3** (apunta al directorio).
- **Pentium:** lógica → segmentación → lineal → paginación → física. Páginas 4 KB (2 niveles 10|10|12) o 4 MB (10|22).
- **Paginación por demanda:** se carga solo lo que se necesita (lazy swapper). Funciona por la **localidad de referencias**.

---

## 2. SISTEMAS DE ARCHIVOS

### 2.1 Tiempo de acceso a disco (ej. 1)
$$T_{acceso} = T_{seek} + T_{rot} + T_{transf}$$
- $T_{rot}$ (latencia rotacional **media**) $= \dfrac{1}{2}\cdot\dfrac{60}{rpm}$ s. Ej. 7200 rpm → $0.5 · 8.33$ ms = **4.17 ms**.
- $T_{transf} = \dfrac{tam\_bloque}{tasa}$. Ej. 4096 B / 100 MB/s ≈ **0.04 ms**.
- Ej.: 4 + 4.17 + 0.04 ≈ **8.2 ms** (dominan las partes mecánicas).

**Leer archivo de $N$ bloques:**
- **Contiguo:** 1 seek + 1 rot + $N$ transferencias $= 8.17 + N\cdot0.04$ ms. Para 1000 bloques ≈ **49 ms**.
- **Disperso:** $N \cdot 8.2$ ms. Para 1000 ≈ **8.2 s** (~170 veces más).
- **Diseño:** el tiempo se va en mover el cabezal. Mantener bloques del archivo cerca (grupos de cilindros, extents, contigua) y hacer **read-ahead**.

### 2.2 Accesos a disco para leer el bloque $i$ de un archivo de $N$ bloques (ej. 2)
| Método | Accesos | Peor caso | Promedio ($i$ uniforme) |
|---|---|---|---|
| Contigua | 1 | 1 | 1 |
| Ligada en disco | $i+1$ | $N$ | $(N+1)/2$ |
| FAT en memoria | 1 (la cadena se recorre en RAM) | 1 | 1 |
| Indexada (1 bloque índice en disco) | 2 (1 si el índice ya está en memoria) | 2 | 2 |
| i-nodo en memoria, directos | 1 | 1 | 1 |

Valor esperado: si $i$ es uniforme en $0..N-1$, $E[i+1] = (N+1)/2$.

### 2.3 FAT (ej. 3)
- **Nº de entradas** $= \dfrac{\text{tamaño disco}}{\text{tamaño bloque}}$ (una por bloque del **disco**).
- **Tamaño FAT** $=$ entradas × bytes por entrada.
  - Ej.: 2 GB / 4 KB = $2^{31}/2^{12} = 2^{19}$ = 524 288 entradas × 4 B = $2^{21}$ = **2 MB**.
- **Lectura aleatoria, ligada en disco:** $\approx (N+1)/2$ accesos de disco por lectura. Ej. 1024 × 8 ms = 8.2 s por lectura; 50 lecturas ≈ **410 s**.
- **Lectura aleatoria, FAT:** recorrer cadena en memoria + 1 acceso a disco: $1024 \cdot 50\text{ ns} + 8\text{ ms} \approx 8.05$ ms; 50 lecturas ≈ **0.4 s** (~1000 veces menos).
- **Secuencial:** ambos esquemas ≈ igual: $N \cdot 8$ ms = 2048 · 8 ms ≈ **16.4 s**. El recorrido se amortiza porque cada bloque se visita una vez.
- **Conclusión:** la ventaja de FAT aparece en acceso **aleatorio**.

### 2.4 i-nodos (ej. 4 y 5)
Datos: bloque de $B$ bytes, puntero de $P$ bytes, 12 directos + simple + doble + triple.

$$\text{punteros por bloque } k = B / P$$
$$\text{bloques máx} = 12 + k + k^2 + k^3$$
$$\text{tamaño máx} = \text{bloques máx}\cdot B$$

Ej. (1 KB, 4 B): $k=256$. Bloques = 12 + 256 + 65 536 + 16 777 216 = **16 843 020** → ≈ **16.06 GB**.

**Ubicar el byte en el offset $X$:**
1. Bloque lógico $b = \lfloor X / B \rfloor$.
2. Zona según $b$:
   - Directos: $0 \dots 11$ → **1 acceso** (solo el dato)
   - Simple: $12 \dots 12+k-1$ ($12..267$) → **2 accesos**
   - Doble: $12+k \dots 12+k+k^2-1$ ($268..65\,803$) → **3 accesos**
   - Triple: el resto → **4 accesos**
3. (con i-nodo en memoria y sin caché; accesos = niveles de indirección + 1 del dato)

Ej.: $X=5\,000\,000$ → $b=\lfloor 5\,000\,000/1024 \rfloor = 4882$ → zona doble → **3 accesos**.

**Rediseño (ej. 5):** bloque 2 KB, i-nodo 48 B (16 B metadatos + 8 punteros × 4 B).
- Máx original: $8 \cdot 2$ KB = **16 KB**.
- Cambiar 1 directo por indirecto simple: $k = 2048/4 = 512$. Máx $= 7\cdot2\text{ KB} + 512\cdot2\text{ KB} = 14\text{ KB} + 1\text{ MB}$. Costo: **2 accesos** para bloques de la zona indirecta.
- Archivo de 3 KB: usa 2 directos, **no se entera** del cambio.
- Alternativa de subir el bloque a 128 KB: un archivo de 3 KB ocuparía 128 KB → >97 % de **fragmentación interna**. La indirección solo "cobra" a los archivos grandes.

### 2.5 Abrir un archivo por su ruta (ej. 6)
En Unix, un directorio es un archivo con pares (nombre, nº de i-nodo). Para cada componente intermedio se lee su **bloque de datos** (para buscar el siguiente nombre) y luego el **i-nodo** del siguiente.

Para `/home/ana/notas.txt` (i-nodo de `/` en memoria, cada directorio cabe en 1 bloque, sin caché):
1. bloque de datos de `/` (busca `home`)
2. i-nodo de `home`
3. bloque de datos de `home` (busca `ana`)
4. i-nodo de `ana`
5. bloque de datos de `ana` (busca `notas.txt`)
6. i-nodo de `notas.txt`
7. primer bloque de datos del archivo

Total = **7 accesos** (6 solo para resolver la ruta). Fórmula: $2k+1$ con $k$ = nº de componentes de la ruta.
Leer el **2º bloque** después: **1 acceso** (el i-nodo ya está en memoria). Por eso abrir es caro, leer es barato, y se usan cachés de i-nodos y de directorios.

---

## 3. Plantillas rápidas para la prueba

**"Calcula la tabla de páginas / entradas":** $2^{m-n}$ entradas × tamaño entrada.

**"Traduce la dirección":** separar $p$ y $d$ → buscar marco → $f \cdot tam + d$ (hex: pegar marco + offset de 3 dígitos).

**"Dos niveles, descompón":** 3 dígitos hex finales = offset; los 20 bits siguientes → 10 | 10.

**"Memoria de tablas con dos niveles":** 4 KB + 4 KB × nº de regiones de 4 MB usadas.

**"EAT":** ponderar por probabilidades; contar accesos a memoria en acierto y en fallo (1 nivel: 1 y 2; 2 niveles: 1 y 3).

**"Máx $p$":** despejar $p \le (L - t_a)/(t_{falta} - t_a)$.

**"Simular reemplazo":** tabla con marcos tras cada referencia, marcar F en cada falta. OPT mira hacia adelante, LRU hacia atrás, FIFO el orden de llegada.

**"¿Hay thrashing?":** $\sum WS_i$ vs. marcos disponibles; la solución es bajar el grado de multiprogramación o controlar la admisión.

**Archivos:** distinguir si el costo se paga en **disco** (ms) o en **memoria** (ns); FAT mueve los punteros a memoria.

---

## 4. Errores típicos a evitar
- Usar la RAM física para calcular las entradas de la tabla (se usa el espacio **virtual**).
- Olvidar que en 2 niveles un fallo de TLB cuesta **3** accesos (no 2).
- En FIFO, reordenar la cola en un acierto (no se hace).
- Confundir página **no presente** (se carga) con **inválida** (se aborta el proceso).
- Contar como falta de página un simple fallo de TLB cuando la PTE tiene V=1.
- Olvidar escribir a swap una víctima sucia (D=1).
- En i-nodos, olvidar sumar el acceso al bloque de datos tras los niveles de indirección.
- En FAT, calcular las entradas con el tamaño del archivo en vez del tamaño del **disco**.
