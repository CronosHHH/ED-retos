#ifndef JUEGO_VIDA_H
#define JUEGO_VIDA_H

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

/**
 * @brief TDA para un patrón del juego de la vida de Conway.
 *
 * El universo es un plano ilimitado: toda celda fuera del patrón está muerta.
 * El patrón se representa siempre mediante el rectángulo mínimo que contiene
 * sus celdas vivas. Las coordenadas de consulta son locales a ese rectángulo y
 * pueden cambiar cuando el patrón crece, se desplaza o se recorta.
 *
 * El patrón se almacena en una matriz rectangular densa dentro de un vector STL.
 * Las reglas se aplican simultáneamente a partir de la generación anterior.
 */
class JuegoVida {
public:
    /**
     * @brief Construye un patrón vacío.
     * @post Las dimensiones son 0 x 0 y ambos contadores valen cero.
     */
    JuegoVida() noexcept = default;

    /** @brief Copia el patrón y sus contadores de forma independiente. */
    JuegoVida(const JuegoVida&) = default;

    /** @brief Sustituye el patrón y sus contadores por una copia independiente. */
    JuegoVida& operator=(const JuegoVida&) = default;

    /**
     * @brief Transfiere el patrón y sus contadores.
     * @post El origen queda vacío con dimensiones 0 x 0 y contadores a cero.
     */
    JuegoVida(JuegoVida&& otro) noexcept;

    /**
     * @brief Sustituye el patrón y sus contadores transfiriendo otro objeto.
     * @post El origen queda vacío con contadores a cero, salvo autoasignación,
     *       en cuyo caso el objeto mantiene su estado.
     */
    JuegoVida& operator=(JuegoVida&& otro) noexcept;

    /**
     * @brief Carga un patrón desde un fichero de texto.
     * @param nombre Ruta del fichero que se desea leer.
     * @pre El objeto mantiene su invariante de representación.
     * @post En caso de éxito, sustituye el patrón y reinicia a cero el número
     *       de iteraciones. En caso de error, conserva todo el estado anterior.
     * @throws std::invalid_argument Si el nombre o el formato no son válidos.
     * @throws std::runtime_error Si no se puede abrir o leer el fichero.
     * @throws std::overflow_error Si las dimensiones no caben en size_t.
     * @throws std::length_error Si la matriz supera la capacidad del vector.
     * @throws std::bad_alloc Si no se dispone de memoria suficiente.
     *
     * El formato de entrada consta de dos cabeceras decimales no negativas F
     * y C, seguidas de F líneas de C caracteres, exclusivamente '.' o 'X'.
     * Un patrón no vacío debe tener alguna celda viva en cada uno de sus
     * cuatro bordes. El vacío se escribe con las dimensiones 0 y 0.
     * Se aceptan LF, CRLF y una última línea sin salto final. Las cabeceras
     * pueden incluir espacios o tabuladores al principio o al final. No se
     * admiten líneas ni caracteres adicionales después del patrón.
     */
    void cargarPatron(const std::string& nombre);

    /**
     * @brief Guarda el patrón actual con el mismo formato que cargarPatron().
     * @param nombre Ruta del fichero de salida; una cadena vacía cancela.
     * @return false si el nombre está vacío; true si se ha guardado con éxito.
     * @post El estado del TDA no cambia. El fichero, si se escribe, contiene
     *       el rectángulo mínimo del patrón; para el vacío contiene 0 y 0.
     * @throws std::runtime_error Si no se puede abrir, escribir o cerrar el
     *         fichero. En tal caso, el fichero puede haber quedado incompleto.
     * @throws std::bad_alloc Si no se dispone de memoria para la fila de salida.
     */
    bool guardarPatron(const std::string& nombre) const;

    /**
     * @brief Aplica una generación completa de las reglas de Conway.
     * @post Cada celda viva sobrevive con dos o tres vecinas vivas y cada
     *       celda muerta nace con tres. Incrementa el contador de iteraciones
     *       en uno y recorta el resultado a su rectángulo mínimo.
     * @post Un patrón vacío permanece vacío y también suma una iteración.
     * @post Si se produce una excepción, todo el estado permanece intacto.
     * @throws std::overflow_error Si las dimensiones o el contador desbordan.
     * @throws std::length_error Si la matriz supera la capacidad del vector.
     * @throws std::bad_alloc Si no se dispone de memoria suficiente.
     */
    void avanzarGeneracion();

    /**
     * @brief Ejecuta n generaciones consecutivas.
     * @param n Número de generaciones adicionales que se desean ejecutar.
     * @return n cuando todas las generaciones se han completado.
     * @post Con n igual a cero, no cambia el estado.
     * @post En caso de éxito, equivale a llamar n veces a avanzarGeneracion().
     * @post Ante un fallo de una generación, conserva la última generación
     *       completada. Si la suma del contador desborda, no inicia la simulación.
     * @throws std::overflow_error Si el contador o las dimensiones desbordan.
     * @throws std::length_error Si alguna matriz supera la capacidad del vector.
     * @throws std::bad_alloc Si no se dispone de memoria suficiente.
     */
    std::size_t simular(std::size_t n);

    /**
     * @brief Consulta las dimensiones del rectángulo mínimo del patrón.
     * @return Par (número de filas, número de columnas); (0, 0) para el vacío.
     * @post El estado no cambia.
     */
    std::pair<std::size_t, std::size_t> obtenerDimensiones() const noexcept;

    /**
     * @brief Consulta el número de celdas vivas del patrón actual.
     * @return Número exacto de celdas vivas.
     * @post El estado no cambia.
     */
    std::size_t obtenerNumeroCeldasVivas() const noexcept;

    /**
     * @brief Consulta las generaciones completadas desde la última carga.
     * @return Número de iteraciones realizadas.
     * @post El estado no cambia.
     */
    std::size_t obtenerIteraciones() const noexcept;

    /**
     * @brief Consulta una celda del rectángulo actual usando índices desde cero.
     * @param fila Índice de la fila local.
     * @param columna Índice de la columna local.
     * @pre fila < número de filas y columna < número de columnas.
     * @return true si está viva; false si está muerta.
     * @post El estado no cambia.
     * @throws std::out_of_range Si algún índice está fuera del rectángulo.
     */
    bool obtenerCelda(std::size_t fila, std::size_t columna) const;

private:
    /**
     * @brief Representación densa en orden de filas.
     *
     * Invariante: celdas_.size() == filas_ * columnas_; cada elemento es 0 o 1;
     * vivas_ cuenta los elementos iguales a 1. Si vivas_ == 0, las dimensiones
     * son 0 x 0 y el vector está vacío. En cualquier otro caso, filas_ y
     * columnas_ son positivas y cada borde contiene al menos una celda viva.
     * La celda (f, c) ocupa celdas_[f * columnas_ + c]. No se conservan
     * coordenadas absolutas, compresión ni estructuras auxiliares avanzadas.
     */
    std::vector<unsigned char> celdas_;
    std::size_t filas_ = 0;
    std::size_t columnas_ = 0;
    std::size_t vivas_ = 0;
    std::size_t iteraciones_ = 0;
};

#endif
