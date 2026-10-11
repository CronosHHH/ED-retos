# Reto 1: Juego de la Vida

Implementación en C++17 del TDA `JuegoVida`, con contratos Doxygen en
`include/JuegoVida.h`. El patrón se almacena en una matriz rectangular densa
mediante `std::vector<unsigned char>`: un valor por celda. Se utilizan vectores,
contadores y operaciones sencillas de la STL.

## Compilar y ejecutar

Se necesitan un compilador compatible con C++17 y Make. Desde esta carpeta:

```bash
make
./bin/reto1 ejemplos/exploder.txt 100
```

El programa muestra, en este orden, las iteraciones completadas, la población,
las dimensiones mínimas y el tiempo en segundos. El tiempo incluye la carga y
la simulación. Después pregunta por el fichero de salida; pulsa Intro para
omitir el guardado o introduce, por ejemplo, `resultado.txt`.

Para introducir también los datos iniciales por consola:

```bash
./bin/reto1
```

Las iteraciones pueden ser cero. Los errores se muestran en la salida de error
y el programa termina con código 1. La ayuda está disponible con `--help`.

## Organización

| Fichero o carpeta | Contenido |
| --- | --- |
| `include/JuegoVida.h` | Interfaz pública y contratos del TDA. |
| `src/JuegoVida.cpp` | Lectura, escritura, consultas y evolución. |
| `src/main.cpp` | Argumentos, interacción y cronómetro. |
| `ejemplos/` | Patrones listos para utilizar. |
| `tests/test_juego_vida.cpp` | Pruebas de reglas, evolución y ficheros. |
| `reto1.pdf` y `reto1.odt` | Especificación y su versión editable. |

## Formato de los patrones

El fichero contiene el número de filas, el de columnas y una línea por fila.
Cada fila tiene exactamente el número indicado de caracteres: `.` representa
una celda muerta y `X` una viva. Por ejemplo, `ejemplos/oscilador.txt`:

```text
1
3
XXX
```

Todo patrón no vacío ocupa su rectángulo mínimo: ninguno de sus cuatro bordes
puede estar completamente muerto. Se rechazan dimensiones negativas, filas de
longitud incorrecta, caracteres inválidos y datos adicionales. Se aceptan finales
de línea LF y CRLF, así como una última línea sin salto final.

La extinción se representa con dimensiones `0 × 0`: el fichero contiene dos
líneas con `0` y ninguna fila. Esta es la convención elegida para el caso vacío,
que el enunciado no concreta.

## Diseño del TDA

Todas las celdas de una generación se calculan desde el mismo estado anterior.
Se consideran también los posibles nacimientos a una celda de distancia del
rectángulo actual. Cada resultado se recorta al rectángulo mínimo. El plano no
conecta los bordes ni tiene dimensiones máximas prefijadas.

Durante cada paso se mantienen la matriz anterior y la siguiente. El recorte
reutiliza la segunda matriz, sin reservar otra para copiar el resultado.

Las consultas utilizan coordenadas locales del rectángulo actual. Una carga
correcta reinicia el contador de generaciones; `simular(n)` añade `n`, incluso
si el patrón está vacío. Una carga fallida conserva el objeto y un fallo durante
la simulación conserva la última generación completa.

Los errores de los contratos se comunican mediante excepciones. En una
simulación correcta, `simular` devuelve el número de generaciones solicitadas;
si falla, `obtenerIteraciones` permite conocer el total completado.
`guardarPatron` devuelve `false` si el destino está vacío y `true` si guarda
correctamente; una escritura fallida puede dejar incompleto el fichero de
destino, pero conserva el estado del TDA.

El tiempo de un paso es proporcional al área del rectángulo ampliado en una
celda por cada lado. El espacio depende del mayor patrón utilizado; no se
almacena el historial de generaciones. No se emplean tablas hash, árboles ni
compresión de celdas.

## Comprobar la implementación

```bash
make test
```

Las pruebas comprueban patrones estables, osciladores, deslizadores, extinción,
cero iteraciones, errores de entrada, conservación del estado, límites del
contador y escritura/lectura. También verifican el ejemplo del enunciado:
`exploder.txt` tras 100 generaciones tiene 72 celdas vivas en un rectángulo
de `13 × 13`.
