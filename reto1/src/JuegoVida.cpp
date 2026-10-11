#include "JuegoVida.h"

#include <algorithm>
#include <fstream>
#include <limits>
#include <stdexcept>

namespace {

void quitarRetornoDeCarro(std::string& linea) {
    if (!linea.empty() && linea.back() == '\r') {
        linea.pop_back();
    }
}

std::string leerLineaNecesaria(std::ifstream& entrada,
                              const std::string& descripcion) {
    std::string linea;
    if (!std::getline(entrada, linea)) {
        if (entrada.bad()) {
            throw std::runtime_error("No se pudo leer el fichero del patrón.");
        }
        throw std::invalid_argument("Falta " + descripcion + " en el fichero del patrón.");
    }
    quitarRetornoDeCarro(linea);
    return linea;
}

std::size_t leerDimension(const std::string& linea,
                         const std::string& descripcion) {
    const std::size_t principio = linea.find_first_not_of(" \t");
    if (principio == std::string::npos) {
        throw std::invalid_argument("La cabecera de " + descripcion + " está vacía.");
    }
    const std::size_t fin = linea.find_last_not_of(" \t");
    std::size_t valor = 0;
    constexpr std::size_t maximo = std::numeric_limits<std::size_t>::max();
    for (std::size_t i = principio; i <= fin; ++i) {
        if (linea[i] < '0' || linea[i] > '9') {
            throw std::invalid_argument("La cabecera de " + descripcion +
                                        " debe contener un entero decimal no negativo.");
        }
        const std::size_t digito = static_cast<std::size_t>(linea[i] - '0');
        if (valor > (maximo - digito) / 10) {
            throw std::overflow_error("El número de " + descripcion +
                                      " excede el rango permitido.");
        }
        valor = valor * 10 + digito;
    }
    return valor;
}

std::size_t comprobarArea(std::size_t filas, std::size_t columnas,
                          std::size_t capacidad) {
    if (columnas != 0 && filas > std::numeric_limits<std::size_t>::max() / columnas) {
        throw std::overflow_error("El tamaño de la matriz excede el rango permitido.");
    }
    const std::size_t area = filas * columnas;
    if (area > capacidad) {
        throw std::length_error("La matriz supera la capacidad máxima del vector.");
    }
    return area;
}

}  // namespace

JuegoVida::JuegoVida(JuegoVida&& otro) noexcept
    : celdas_(std::move(otro.celdas_)),
      filas_(otro.filas_),
      columnas_(otro.columnas_),
      vivas_(otro.vivas_),
      iteraciones_(otro.iteraciones_) {
    otro.celdas_.clear();
    otro.filas_ = 0;
    otro.columnas_ = 0;
    otro.vivas_ = 0;
    otro.iteraciones_ = 0;
}

JuegoVida& JuegoVida::operator=(JuegoVida&& otro) noexcept {
    if (this != &otro) {
        celdas_ = std::move(otro.celdas_);
        filas_ = otro.filas_;
        columnas_ = otro.columnas_;
        vivas_ = otro.vivas_;
        iteraciones_ = otro.iteraciones_;
        otro.celdas_.clear();
        otro.filas_ = 0;
        otro.columnas_ = 0;
        otro.vivas_ = 0;
        otro.iteraciones_ = 0;
    }
    return *this;
}

void JuegoVida::cargarPatron(const std::string& nombre) {
    if (nombre.empty()) {
        throw std::invalid_argument("El nombre del fichero de entrada está vacío.");
    }
    std::ifstream entrada(nombre, std::ios::binary);
    if (!entrada.is_open()) {
        throw std::runtime_error("No se pudo abrir el fichero de entrada: " + nombre);
    }

    const std::size_t nuevasFilas =
        leerDimension(leerLineaNecesaria(entrada, "la cabecera de filas"), "filas");
    const std::size_t nuevasColumnas =
        leerDimension(leerLineaNecesaria(entrada, "la cabecera de columnas"), "columnas");
    if ((nuevasFilas == 0) != (nuevasColumnas == 0)) {
        throw std::invalid_argument("Un patrón vacío debe tener dimensiones 0 x 0.");
    }

    const std::size_t area = comprobarArea(nuevasFilas, nuevasColumnas, celdas_.max_size());
    std::vector<unsigned char> nuevasCeldas(area, 0);
    std::size_t nuevasVivas = 0;
    bool bordeSuperiorVivo = false;
    bool bordeInferiorVivo = false;
    bool bordeIzquierdoVivo = false;
    bool bordeDerechoVivo = false;

    for (std::size_t fila = 0; fila < nuevasFilas; ++fila) {
        const std::string linea = leerLineaNecesaria(entrada, "una fila del patrón");
        if (linea.size() != nuevasColumnas) {
            throw std::invalid_argument("La fila " + std::to_string(fila + 1) +
                                        " no tiene el número de columnas indicado.");
        }
        for (std::size_t columna = 0; columna < nuevasColumnas; ++columna) {
            if (linea[columna] != '.' && linea[columna] != 'X') {
                throw std::invalid_argument("El patrón solo admite los caracteres '.' y 'X'.");
            }
            if (linea[columna] == 'X') {
                nuevasCeldas[fila * nuevasColumnas + columna] = 1;
                ++nuevasVivas;
                bordeSuperiorVivo = bordeSuperiorVivo || fila == 0;
                bordeInferiorVivo = bordeInferiorVivo || fila == nuevasFilas - 1;
                bordeIzquierdoVivo = bordeIzquierdoVivo || columna == 0;
                bordeDerechoVivo = bordeDerechoVivo || columna == nuevasColumnas - 1;
            }
        }
    }

    std::string lineaAdicional;
    if (std::getline(entrada, lineaAdicional)) {
        throw std::invalid_argument("El fichero contiene datos adicionales después del patrón.");
    }
    if (entrada.bad()) {
        throw std::runtime_error("No se pudo completar la lectura del fichero del patrón.");
    }
    if (nuevasFilas != 0 &&
        !(bordeSuperiorVivo && bordeInferiorVivo && bordeIzquierdoVivo && bordeDerechoVivo)) {
        throw std::invalid_argument("El patrón debe ser el rectángulo mínimo de sus celdas vivas.");
    }

    // El estado solo se sustituye una vez que todo el fichero es válido.
    celdas_.swap(nuevasCeldas);
    filas_ = nuevasFilas;
    columnas_ = nuevasColumnas;
    vivas_ = nuevasVivas;
    iteraciones_ = 0;
}

bool JuegoVida::guardarPatron(const std::string& nombre) const {
    if (nombre.empty()) {
        return false;
    }
    // Preparar el búfer antes de abrir evita truncar el fichero si falta memoria.
    std::string linea(columnas_, '.');
    std::ofstream salida(nombre, std::ios::binary | std::ios::trunc);
    if (!salida.is_open()) {
        throw std::runtime_error("No se pudo abrir el fichero de salida: " + nombre);
    }

    salida << filas_ << '\n' << columnas_ << '\n';
    for (std::size_t fila = 0; fila < filas_; ++fila) {
        for (std::size_t columna = 0; columna < columnas_; ++columna) {
            linea[columna] = celdas_[fila * columnas_ + columna] ? 'X' : '.';
        }
        salida << linea << '\n';
        if (!salida) {
            throw std::runtime_error("No se pudo escribir el patrón en el fichero de salida.");
        }
    }
    salida.close();
    if (!salida) {
        throw std::runtime_error("No se pudo completar la escritura del fichero de salida.");
    }
    return true;
}

void JuegoVida::avanzarGeneracion() {
    constexpr std::size_t maximo = std::numeric_limits<std::size_t>::max();
    if (iteraciones_ == maximo) {
        throw std::overflow_error("El contador de iteraciones excede el rango permitido.");
    }
    if (vivas_ == 0) {
        ++iteraciones_;
        return;
    }
    if (filas_ > maximo - 2 || columnas_ > maximo - 2) {
        throw std::overflow_error("No se puede ampliar la matriz sin exceder sus dimensiones.");
    }

    // Una franja de una celda muerta por cada lado contiene todos los nacimientos
    // posibles. Las consultas usan exclusivamente la matriz de la generación vieja.
    const std::size_t filasAmpliadas = filas_ + 2;
    const std::size_t columnasAmpliadas = columnas_ + 2;
    const std::size_t area = comprobarArea(filasAmpliadas, columnasAmpliadas,
                                          celdas_.max_size());
    std::vector<unsigned char> siguiente(area, 0);
    std::size_t nuevasVivas = 0;
    std::size_t filaMinima = filasAmpliadas;
    std::size_t filaMaxima = 0;
    std::size_t columnaMinima = columnasAmpliadas;
    std::size_t columnaMaxima = 0;

    for (std::size_t fila = 0; fila < filasAmpliadas; ++fila) {
        const std::size_t primeraFila = fila > 1 ? fila - 2 : 0;
        const std::size_t finFilas = fila < filas_ ? fila + 1 : filas_;
        for (std::size_t columna = 0; columna < columnasAmpliadas; ++columna) {
            const std::size_t primeraColumna = columna > 1 ? columna - 2 : 0;
            const std::size_t finColumnas = columna < columnas_ ? columna + 1 : columnas_;
            unsigned int vecinas = 0;
            for (std::size_t f = primeraFila; f < finFilas; ++f) {
                for (std::size_t c = primeraColumna; c < finColumnas; ++c) {
                    vecinas += celdas_[f * columnas_ + c];
                }
            }
            const bool viva = fila > 0 && fila <= filas_ &&
                              columna > 0 && columna <= columnas_ &&
                              celdas_[(fila - 1) * columnas_ + columna - 1] != 0;
            if (viva) {
                --vecinas;  // La celda central no es su propia vecina.
            }
            if (vecinas == 3 || (viva && vecinas == 2)) {
                siguiente[fila * columnasAmpliadas + columna] = 1;
                ++nuevasVivas;
                filaMinima = std::min(filaMinima, fila);
                filaMaxima = std::max(filaMaxima, fila);
                columnaMinima = std::min(columnaMinima, columna);
                columnaMaxima = std::max(columnaMaxima, columna);
            }
        }
    }

    std::size_t nuevasFilas = 0;
    std::size_t nuevasColumnas = 0;
    if (nuevasVivas != 0) {
        nuevasFilas = filaMaxima - filaMinima + 1;
        nuevasColumnas = columnaMaxima - columnaMinima + 1;
        if (nuevasFilas != filasAmpliadas || nuevasColumnas != columnasAmpliadas) {
            // Compactar de izquierda a derecha y de arriba abajo es seguro:
            // cada destino precede a su origen, y los orígenes son crecientes.
            for (std::size_t fila = 0; fila < nuevasFilas; ++fila) {
                for (std::size_t columna = 0; columna < nuevasColumnas; ++columna) {
                    siguiente[fila * nuevasColumnas + columna] =
                        siguiente[(fila + filaMinima) * columnasAmpliadas +
                                  columna + columnaMinima];
                }
            }
            siguiente.resize(nuevasFilas * nuevasColumnas);
        }
    } else {
        // Al extinguirse el patrón, también se libera la memoria de la matriz.
        std::vector<unsigned char>().swap(siguiente);
    }

    // Todos los pasos que pueden fallar han terminado: confirmar la generación.
    celdas_.swap(siguiente);
    filas_ = nuevasFilas;
    columnas_ = nuevasColumnas;
    vivas_ = nuevasVivas;
    ++iteraciones_;
}

std::size_t JuegoVida::simular(std::size_t n) {
    if (n > std::numeric_limits<std::size_t>::max() - iteraciones_) {
        throw std::overflow_error("El contador de iteraciones excede el rango permitido.");
    }
    if (vivas_ == 0) {
        iteraciones_ += n;
        return n;
    }
    for (std::size_t realizadas = 0; realizadas < n; ++realizadas) {
        avanzarGeneracion();
        if (vivas_ == 0) {
            iteraciones_ += n - realizadas - 1;
            break;
        }
    }
    return n;
}

std::pair<std::size_t, std::size_t> JuegoVida::obtenerDimensiones() const noexcept {
    return {filas_, columnas_};
}

std::size_t JuegoVida::obtenerNumeroCeldasVivas() const noexcept {
    return vivas_;
}

std::size_t JuegoVida::obtenerIteraciones() const noexcept {
    return iteraciones_;
}

bool JuegoVida::obtenerCelda(std::size_t fila, std::size_t columna) const {
    if (fila >= filas_ || columna >= columnas_) {
        throw std::out_of_range("La celda solicitada está fuera del rectángulo del patrón.");
    }
    return celdas_[fila * columnas_ + columna] != 0;
}
