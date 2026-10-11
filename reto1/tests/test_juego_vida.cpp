#include "JuegoVida.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

namespace {

namespace fs = std::filesystem;
using Patron = std::vector<std::string>;

void comprobar(bool condicion, const std::string& mensaje) {
    if (!condicion) {
        throw std::runtime_error(mensaje);
    }
}

template <typename Excepcion, typename Accion>
void comprobarExcepcion(Accion accion, const std::string& mensaje) {
    try {
        accion();
    } catch (const Excepcion&) {
        return;
    } catch (...) {
        throw std::runtime_error(mensaje + ": tipo de excepcion inesperado");
    }
    throw std::runtime_error(mensaje + ": no se lanzo una excepcion");
}

class DirectorioTemporal {
public:
    DirectorioTemporal() {
        const auto marca = std::chrono::high_resolution_clock::now()
                               .time_since_epoch().count();
        for (unsigned intento = 0; intento < 100; ++intento) {
            ruta_ = fs::temp_directory_path() /
                    ("reto1-pruebas-" + std::to_string(marca) + "-" +
                     std::to_string(intento));
            if (fs::create_directory(ruta_)) {
                return;
            }
        }
        throw std::runtime_error("No se pudo crear el directorio temporal");
    }

    ~DirectorioTemporal() {
        std::error_code error;
        fs::remove_all(ruta_, error);
    }

    DirectorioTemporal(const DirectorioTemporal&) = delete;
    DirectorioTemporal& operator=(const DirectorioTemporal&) = delete;

    fs::path fichero(const std::string& nombre) const {
        return ruta_ / nombre;
    }

private:
    fs::path ruta_;
};

void escribir(const fs::path& ruta, const std::string& contenido) {
    std::ofstream salida(ruta, std::ios::binary);
    salida << contenido;
    salida.close();
    comprobar(static_cast<bool>(salida), "No se pudo escribir el dato de prueba");
}

std::string leer(const fs::path& ruta) {
    std::ifstream entrada(ruta, std::ios::binary);
    comprobar(static_cast<bool>(entrada), "No se pudo abrir el resultado guardado");
    return std::string(std::istreambuf_iterator<char>(entrada),
                       std::istreambuf_iterator<char>());
}

std::string textoPatron(const Patron& filas) {
    std::string resultado = std::to_string(filas.size()) + "\n" +
                            std::to_string(filas.empty() ? 0 : filas.front().size()) +
                            "\n";
    for (const auto& fila : filas) {
        resultado += fila + "\n";
    }
    return resultado;
}

void cargar(JuegoVida& juego, const DirectorioTemporal& temporal,
            const Patron& patron) {
    const auto ruta = temporal.fichero("entrada.txt");
    escribir(ruta, textoPatron(patron));
    juego.cargarPatron(ruta.string());
}

void comprobarEstado(const JuegoVida& juego, const Patron& esperado,
                     std::size_t vivas, std::size_t generaciones) {
    const auto dimensiones = std::make_pair(
        esperado.size(), esperado.empty() ? std::size_t{0} : esperado.front().size());
    comprobar(juego.obtenerDimensiones() == dimensiones, "Dimensiones incorrectas");
    comprobar(juego.obtenerNumeroCeldasVivas() == vivas, "Numero de vivas incorrecto");
    comprobar(juego.obtenerIteraciones() == generaciones, "Contador incorrecto");
    for (std::size_t fila = 0; fila < esperado.size(); ++fila) {
        for (std::size_t columna = 0; columna < esperado[fila].size(); ++columna) {
            comprobar(juego.obtenerCelda(fila, columna) ==
                          (esperado[fila][columna] == 'X'),
                      "Celda incorrecta en " + std::to_string(fila) + "," +
                          std::to_string(columna));
        }
    }
}

void probarVacio() {
    JuegoVida juego;
    comprobarEstado(juego, {}, 0, 0);
    comprobarExcepcion<std::out_of_range>(
        [&] { juego.obtenerCelda(0, 0); }, "Acceso al patron vacio");
    comprobar(juego.simular(0) == 0, "simular(0) devuelve un valor incorrecto");
    comprobarEstado(juego, {}, 0, 0);
    juego.avanzarGeneracion();
    comprobar(juego.simular(5) == 5, "simular debe devolver las iteraciones realizadas");
    comprobarEstado(juego, {}, 0, 6);
    comprobar(!juego.guardarPatron(""), "Un nombre vacio debe omitir el guardado");
}

void probarReglas() {
    DirectorioTemporal temporal;
    JuegoVida juego;

    // Soledad: se extingue y su rectangulo minimo pasa a ser 0 x 0.
    cargar(juego, temporal, {"X"});
    juego.avanzarGeneracion();
    comprobarEstado(juego, {}, 0, 1);
    juego.simular(7);
    comprobarEstado(juego, {}, 0, 8);

    cargar(juego, temporal, {"XX"});
    juego.avanzarGeneracion();
    comprobarEstado(juego, {}, 0, 1);

    // Una celda muerta con tres vecinas nace; las otras sobreviven con dos.
    cargar(juego, temporal, {"XX", "X."});
    juego.avanzarGeneracion();
    comprobarEstado(juego, {"XX", "XX"}, 4, 1);

    // Las vecinas diagonales cuentan y todas las transiciones son simultaneas.
    // El centro (ocho vecinas) y los centros de los lados (cinco) mueren;
    // aparecen celdas en cada uno de los cuatro lados exteriores.
    cargar(juego, temporal, {"XXX", "XXX", "XXX"});
    juego.avanzarGeneracion();
    comprobarEstado(juego,
                    {"..X..", ".X.X.", "X...X", ".X.X.", "..X.."}, 8, 1);
}

void probarBloqueYContador() {
    DirectorioTemporal temporal;
    JuegoVida juego;
    cargar(juego, temporal, {"XX", "XX"});
    comprobar(juego.simular(0) == 0, "Simulacion nula incorrecta");
    comprobarEstado(juego, {"XX", "XX"}, 4, 0);
    comprobar(juego.simular(100) == 100, "Simulacion del bloque incorrecta");
    comprobarEstado(juego, {"XX", "XX"}, 4, 100);
    juego.avanzarGeneracion();
    comprobarEstado(juego, {"XX", "XX"}, 4, 101);

    cargar(juego, temporal, {"XXX"});
    comprobarEstado(juego, {"XXX"}, 3, 0);
    cargar(juego, temporal, {});
    comprobarEstado(juego, {}, 0, 0);
}

void probarOscilador() {
    DirectorioTemporal temporal;
    JuegoVida juego;
    cargar(juego, temporal, {"XXX"});
    juego.avanzarGeneracion();
    comprobarEstado(juego, {"X", "X", "X"}, 3, 1);
    juego.avanzarGeneracion();
    comprobarEstado(juego, {"XXX"}, 3, 2);
    juego.simular(99);
    comprobarEstado(juego, {"X", "X", "X"}, 3, 101);

    comprobarExcepcion<std::out_of_range>(
        [&] { juego.obtenerCelda(3, 0); }, "Fila fuera del patron");
    comprobarExcepcion<std::out_of_range>(
        [&] { juego.obtenerCelda(0, 1); }, "Columna fuera del patron");
    comprobarExcepcion<std::out_of_range>(
        [&] { juego.obtenerCelda(std::numeric_limits<std::size_t>::max(), 0); },
        "Indice de fila extremo");
}

void probarDeslizador() {
    DirectorioTemporal temporal;
    JuegoVida juego;
    cargar(juego, temporal, {".X.", "..X", "XXX"});
    juego.avanzarGeneracion();
    comprobarEstado(juego, {"X.X", ".XX", ".X."}, 5, 1);
    juego.simular(3);
    comprobarEstado(juego, {".X.", "..X", "XXX"}, 5, 4);
    juego.simular(36);
    comprobarEstado(juego, {".X.", "..X", "XXX"}, 5, 40);

    // El bloque inmovil permite observar la traslacion del deslizador y el
    // crecimiento simultaneo de filas y columnas del rectangulo minimo.
    cargar(juego, temporal,
           {"XX.......", "XX.......", ".........", ".........", ".........",
            ".........", ".......X.", "........X", "......XXX"});
    juego.simular(4);
    comprobarEstado(juego,
                    {"XX........", "XX........", "..........", "..........",
                     "..........", "..........", "..........", "........X.",
                     ".........X", ".......XXX"}, 9, 4);
}

void probarExploder() {
    DirectorioTemporal temporal;
    JuegoVida juego;
    cargar(juego, temporal, {"X.X.X", "X...X", "X...X", "X...X", "X.X.X"});
    comprobar(juego.simular(100) == 100, "Iteraciones del exploder incorrectas");
    // Resultado publicado en el enunciado: 72 vivas en un rectangulo 13 x 13.
    comprobarEstado(juego,
                    {"..XX.....XX..", "...XX...XX...", "X..X.X.X.X..X",
                     "XXX.XX.XX.XXX", ".X.X.X.X.X.X.", "..XXX...XXX..",
                     ".............", "..XXX...XXX..", ".X.X.X.X.X.X.",
                     "XXX.XX.XX.XXX", "X..X.X.X.X..X", "...XX...XX...",
                     "..XX.....XX.."}, 72, 100);
}

void probarEntradaYSalida() {
    DirectorioTemporal temporal;
    JuegoVida juego;
    const auto entrada = temporal.fichero("formato.txt");
    escribir(entrada, "2\r\n2\r\nXX\r\nXX\r\n");
    juego.cargarPatron(entrada.string());
    comprobarEstado(juego, {"XX", "XX"}, 4, 0);

    escribir(entrada, "1\n3\nXXX");
    juego.cargarPatron(entrada.string());
    comprobarEstado(juego, {"XXX"}, 3, 0);
    juego.avanzarGeneracion();
    const auto salida = temporal.fichero("resultado.txt");
    comprobar(juego.guardarPatron(salida.string()), "No se guardo el patron");
    comprobar(leer(salida) == "3\n1\nX\nX\nX\n", "Formato de salida incorrecto");
    JuegoVida recargado;
    recargado.cargarPatron(salida.string());
    comprobarEstado(recargado, {"X", "X", "X"}, 3, 0);
    comprobarEstado(juego, {"X", "X", "X"}, 3, 1);
    comprobar(!juego.guardarPatron(""), "No se omitio el nombre vacio");
    comprobarEstado(juego, {"X", "X", "X"}, 3, 1);

    comprobarExcepcion<std::exception>(
        [&] { juego.guardarPatron(temporal.fichero("inexistente/salida.txt").string()); },
        "Error al guardar en un directorio inexistente");
    comprobarEstado(juego, {"X", "X", "X"}, 3, 1);

    cargar(juego, temporal, {"X"});
    juego.avanzarGeneracion();
    comprobar(juego.guardarPatron(salida.string()), "No se guardo el patron vacio");
    comprobar(leer(salida) == "0\n0\n", "Formato del patron vacio incorrecto");
    recargado.cargarPatron(salida.string());
    comprobarEstado(recargado, {}, 0, 0);
}

void probarEntradaInvalida() {
    DirectorioTemporal temporal;
    JuegoVida juego;
    cargar(juego, temporal, {"XX", "XX"});
    juego.simular(5);
    const auto ruta = temporal.fichero("invalido.txt");
    const std::vector<std::string> invalidos = {
        "", "1\n", "filas\n1\nX\n", "-1\n1\nX\n", "1\n-1\nX\n",
        "1x\n1\nX\n", "1\n1x\nX\n", "1\n1\nx\n", "1\n1\n?\n",
        "1\n2\nX\n", "1\n1\nXX\n", "2\n1\nX\n", "1\n1\nX\nX\n",
        "1\n1\nX\n\n", "0\n1\n", "1\n0\n\n", "0\n0\nX\n",
        "1\n1\n.\n", "2\n2\n..\nXX\n", "2\n2\nXX\n..\n",
        "2\n2\n.X\n.X\n", "2\n2\nX.\nX.\n",
        "184467440737095516160\n1\nX\n"
    };
    for (std::size_t i = 0; i < invalidos.size(); ++i) {
        escribir(ruta, invalidos[i]);
        comprobarExcepcion<std::exception>(
            [&] { juego.cargarPatron(ruta.string()); },
            "Entrada malformada numero " + std::to_string(i));
        comprobarEstado(juego, {"XX", "XX"}, 4, 5);
    }
    comprobarExcepcion<std::exception>(
        [&] { juego.cargarPatron(temporal.fichero("no-existe.txt").string()); },
        "Fichero de entrada inexistente");
    comprobarEstado(juego, {"XX", "XX"}, 4, 5);
    comprobarExcepcion<std::exception>(
        [&] { juego.cargarPatron(""); }, "Nombre de entrada vacio");
    comprobarEstado(juego, {"XX", "XX"}, 4, 5);
}

void probarDesbordamientoContador() {
    JuegoVida juego;
    const auto maximo = std::numeric_limits<std::size_t>::max();
    // En el patron vacio no hay celdas que actualizar; esta operacion es O(1).
    comprobar(juego.simular(maximo) == maximo, "No se alcanzo el contador maximo");
    comprobarEstado(juego, {}, 0, maximo);
    comprobar(juego.simular(0) == 0, "simular(0) con contador maximo fallo");
    comprobarExcepcion<std::overflow_error>(
        [&] { juego.avanzarGeneracion(); }, "Desbordamiento al avanzar");
    comprobarEstado(juego, {}, 0, maximo);
    comprobarExcepcion<std::overflow_error>(
        [&] { juego.simular(1); }, "Desbordamiento al simular");
    comprobarEstado(juego, {}, 0, maximo);
}

void probarCopiaYMovimiento() {
    DirectorioTemporal temporal;
    JuegoVida original;
    cargar(original, temporal, {"XXX"});
    original.simular(7);
    JuegoVida copia(original);
    cargar(original, temporal, {"X"});
    comprobarEstado(original, {"X"}, 1, 0);
    comprobarEstado(copia, {"X", "X", "X"}, 3, 7);

    JuegoVida asignado;
    asignado = copia;
    copia.avanzarGeneracion();
    comprobarEstado(asignado, {"X", "X", "X"}, 3, 7);
    comprobarEstado(copia, {"XXX"}, 3, 8);

    JuegoVida movido(std::move(asignado));
    comprobarEstado(movido, {"X", "X", "X"}, 3, 7);
    comprobarEstado(asignado, {}, 0, 0);
    original = std::move(movido);
    comprobarEstado(original, {"X", "X", "X"}, 3, 7);
    comprobarEstado(movido, {}, 0, 0);
    JuegoVida& mismaInstancia = original;
    original = std::move(mismaInstancia);
    comprobarEstado(original, {"X", "X", "X"}, 3, 7);
}

} // namespace

int main() {
    const std::vector<std::pair<std::string, std::function<void()>>> pruebas = {
        {"Patron vacio", probarVacio},
        {"Reglas y actualizacion simultanea", probarReglas},
        {"Bloque estable y contador", probarBloqueYContador},
        {"Oscilador y bordes abiertos", probarOscilador},
        {"Deslizador y crecimiento", probarDeslizador},
        {"Exploder del enunciado", probarExploder},
        {"Carga y guardado", probarEntradaYSalida},
        {"Entradas invalidas y conservacion del estado", probarEntradaInvalida},
        {"Desbordamiento del contador", probarDesbordamientoContador},
        {"Copia independiente y movimiento", probarCopiaYMovimiento}
    };
    std::size_t fallos = 0;
    for (const auto& prueba : pruebas) {
        try {
            prueba.second();
            std::cout << "[OK] " << prueba.first << '\n';
        } catch (const std::exception& error) {
            ++fallos;
            std::cerr << "[FALLO] " << prueba.first << ": " << error.what() << '\n';
        }
    }
    if (fallos != 0) {
        std::cerr << fallos << " pruebas fallidas\n";
        return 1;
    }
    std::cout << "Todas las pruebas superadas (" << pruebas.size() << ")\n";
    return 0;
}
