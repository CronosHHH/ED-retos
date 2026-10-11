#include "JuegoVida.h"

#include <charconv>
#include <chrono>
#include <exception>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <system_error>

namespace {

void mostrarUso(const char* programa) {
    std::cout << "Uso: " << programa << " <fichero_inicial> <iteraciones>\n"
              << "Sin argumentos, los datos se solicitan por consola.\n"
              << "Las iteraciones deben ser un entero no negativo.\n";
}

std::size_t leerIteraciones(const std::string& texto) {
    const auto primero = texto.find_first_not_of(" \t\r\n");
    if (primero == std::string::npos) {
        throw std::invalid_argument("El número de iteraciones está vacío.");
    }
    const auto ultimo = texto.find_last_not_of(" \t\r\n");
    const char* inicio = texto.data() + primero;
    const char* fin = texto.data() + ultimo + 1;
    if (*inicio == '+') {
        ++inicio;
    }
    std::size_t valor = 0;
    const auto resultado = std::from_chars(inicio, fin, valor);
    if (resultado.ec == std::errc::result_out_of_range) {
        throw std::out_of_range("El número de iteraciones es demasiado grande.");
    }
    if (resultado.ec != std::errc{} || resultado.ptr != fin) {
        throw std::invalid_argument("Las iteraciones deben ser un entero no negativo.");
    }
    return valor;
}

std::string pedirDato(const char* mensaje) {
    std::cout << mensaje << std::flush;
    std::string dato;
    if (!std::getline(std::cin, dato)) {
        throw std::runtime_error("No se ha podido leer el dato de entrada.");
    }
    return dato;
}

} // namespace

int main(int argc, char* argv[]) {
    try {
        if (argc == 2 && std::string(argv[1]) == "--help") {
            mostrarUso(argv[0]);
            return 0;
        }
        if (argc != 1 && argc != 3) {
            mostrarUso(argv[0]);
            return 1;
        }

        std::string entrada;
        std::size_t iteraciones = 0;
        if (argc == 3) {
            entrada = argv[1];
            iteraciones = leerIteraciones(argv[2]);
        } else {
            entrada = pedirDato("Fichero de entrada: ");
            iteraciones = leerIteraciones(pedirDato("Número de iteraciones: "));
        }

        JuegoVida juego;
        const auto inicio = std::chrono::steady_clock::now();
        juego.cargarPatron(entrada);
        juego.simular(iteraciones);
        const auto fin = std::chrono::steady_clock::now();
        const std::chrono::duration<double> tiempo = fin - inicio;
        const auto dimensiones = juego.obtenerDimensiones();

        std::cout << juego.obtenerIteraciones() << " iteraciones\n"
                  << juego.obtenerNumeroCeldasVivas() << " celdas vivas\n"
                  << "Dimensiones: " << dimensiones.first << " x " << dimensiones.second << '\n'
                  << std::fixed << std::setprecision(6) << tiempo.count() << " segundos\n"
                  << "Fichero de salida: " << std::flush;

        std::string salida;
        if (!std::getline(std::cin, salida)) {
            if (std::cin.eof()) {
                return 0;
            }
            throw std::runtime_error("No se ha podido leer el nombre del fichero de salida.");
        }
        juego.guardarPatron(salida);
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }
}
