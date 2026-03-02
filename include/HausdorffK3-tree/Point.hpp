//
// Created by migue on 14-10-2025.
//

#ifndef HAUSDORFFK3_TREE_POINT_H
#define HAUSDORFFK3_TREE_POINT_H
#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <stdexcept>

/**
 * @class Point
 * @brief Representa un punto en un espacio D-dimensional.
 *
 * @tparam D El número de dimensiones del punto.
 */
template <std::size_t D>
class Point {
private:
    std::array<uint64_t, D> coords;

public:
    // --- Constructores ---

    /**
     * @brief Constructor por defecto. Inicializa todas las coordenadas a 0.
     */
    Point() {
        coords.fill(0);
    }

    /**
     * @brief Constructor explícito a partir de un array de valores.
     * @param values Un array de `uint64_t` con los valores de las coordenadas.
     */
    explicit Point(const std::array<uint64_t, D>& values)
        : coords(values) {}

    /**
     * @brief Constructor de copia.
     * @param other El punto a copiar.
     */
    Point(const Point<D>& other) {
        coords = other.coords;
    }

    /**
     * @brief Operador de asignación.
     * @param other El punto del que se asignarán los valores.
     * @return Una referencia a este punto.
     */
    Point<D>& operator=(const Point<D>& other) {
        if (this != &other)
            coords = other.coords;
        return *this;
    }

    // --- Acceso a coordenadas ---

    /**
     * @brief Accede a una coordenada específica (const).
     * @param index El índice de la coordenada.
     * @return El valor de la coordenada.
     * @throw std::out_of_range si el índice está fuera del rango de dimensiones.
     */
    uint64_t operator[](std::size_t index) const {
        if (index >= D) throw std::out_of_range("Índice fuera de rango");
        return coords[index];
    }

    /**
     * @brief Accede a una coordenada específica para modificarla.
     * @param index El índice de la coordenada.
     * @return Una referencia a la coordenada.
     * @throw std::out_of_range si el índice está fuera del rango de dimensiones.
     */
    uint64_t& operator[](std::size_t index) {
        if (index >= D) throw std::out_of_range("Índice fuera de rango");
        return coords[index];
    }

    // --- Igualdad ---

    /**
     * @brief Compara si este punto es igual a otro.
     * @param other El otro punto para comparar.
     * @return `true` si son iguales, `false` en caso contrario.
     */
    bool equals(const Point<D>& other) const {
        for (std::size_t i = 0; i < D; ++i)
            if (coords[i] != other.coords[i])
                return false;
        return true;
    }

    /**
     * @brief Operador de igualdad.
     */
    bool operator==(const Point<D>& other) const { return equals(other); }

    /**
     * @brief Operador de desigualdad.
     */
    bool operator!=(const Point<D>& other) const { return !equals(other); }

    // --- Distancia euclidiana ---

    /**
     * @brief Calcula la distancia euclidiana a otro punto.
     * @param other El otro punto.
     * @return La distancia euclidiana.
     */
    double distanciaA(const Point<D>& other) const {
        double suma = 0.0;
        double diff;
        for (std::size_t i = 0; i < D; ++i) {
            diff = static_cast<double>(coords[i]) - static_cast<double>(other.coords[i]);
            suma += diff * diff;
        }
        return std::sqrt(suma);
    }

    /**
     * @brief Calcula el cuadrado de la distancia euclidiana a otro punto.
     *
     * Útil para comparaciones de distancia sin la necesidad de calcular la raíz cuadrada.
     *
     * @param other El otro punto.
     * @return El cuadrado de la distancia euclidiana.
     */
    uint64_t distSq(const Point<D>& other) const {
        uint64_t suma = 0;
        for (std::size_t i = 0; i < D; ++i) {
            const int64_t diff = static_cast<int64_t>(coords[i]) - static_cast<int64_t>(other.coords[i]);
            suma += diff * diff;
        }
        return suma;
    }

    // --- Morton Code (Z-order) ---

    /**
     * @brief Convierte las coordenadas del punto a un código Morton (Z-order).
     *
     * El código Morton intercala los bits de las coordenadas para generar
     * un valor 1D que preserva la localidad espacial.
     *
     * @return El código Morton de 64 bits.
     * @note Se asume que cada coordenada no excede los 21 bits.
     */
    uint64_t toMorton() const {
        // Suponemos hasta 21 bits por coordenada (ajustable según rango)
        uint64_t morton = 0;
        for (uint64_t bit = 0; bit < 21; ++bit) {
            for (std::size_t i = 0; i < D; ++i)
                morton |= ((coords[i] >> bit) & 1ULL) << (bit * D + i);
        }
        return morton;
    }

    /**
     * @brief Crea un punto a partir de un código Morton.
     *
     * @param morton El código Morton de 64 bits.
     * @return Un objeto Point correspondiente al código Morton.
     */
    static Point<D> fromMorton(uint64_t morton) {
        Point<D> p;
        for (std::size_t i = 0; i < D; ++i) {
            uint64_t coord = 0;
            for (uint64_t bit = 0; bit < 21; ++bit)
                coord |= ((morton >> (bit * D + i)) & 1ULL) << bit;
            p.coords[i] = coord;
        }
        return p;
    }

    // --- Comparadores geométricos (orden Z por Morton) ---

    /**
     * @brief Compara dos puntos basado en su orden Z (código Morton).
     */
    bool operator<(const Point<D>& other) const {
        return this->toMorton() < other.toMorton();
    }

    /**
     * @brief Compara dos puntos basado en su orden Z (código Morton).
     */
    bool operator>(const Point<D>& other) const {
        return this->toMorton() > other.toMorton();
    }

    /**
     * @brief Compara dos puntos basado en su orden Z (código Morton).
     */
    bool operator<=(const Point<D>& other) const {
        return this->toMorton() <= other.toMorton();
    }

    /**
     * @brief Compara dos puntos basado en su orden Z (código Morton).
     */
    bool operator>=(const Point<D>& other) const {
        return this->toMorton() >= other.toMorton();
    }

    // --- Representación en consola ---
    friend std::ostream& operator<<(std::ostream& os, const Point<D>& p) {
        os << "(";
        for (std::size_t i = 0; i < D; ++i) {
            os << p.coords[i];
            if (i < D - 1) os << ", ";
        }
        os << ")";
        return os;
    }
};

#endif //HAUSDORFFK3_TREE_POINT_H