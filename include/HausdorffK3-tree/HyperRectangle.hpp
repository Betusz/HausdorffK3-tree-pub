/**
 * @file HyperRectangle.hpp
 * @brief Define la clase HyperRectangle para hiperrectángulos N-dimensionales alineados a los ejes.
 */

#ifndef HAUSDORFFK3_TREE_HYPERRECTANGLE_HPP
#define HAUSDORFFK3_TREE_HYPERRECTANGLE_HPP


#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <algorithm>
#include <stdexcept>
#include "Point.hpp"

/**
 * @brief Representa un hiperrectángulo N-dimensional alineado a los ejes en un espacio discreto.
 *
 * @tparam D La dimensionalidad del hiperrectángulo.
 */
template <std::size_t D>
class HyperRectangle {
public:
    Point<D> minCorner; /**< El punto de la esquina mínima del hiperrectángulo. */
    Point<D> maxCorner; /**< El punto de la esquina máxima del hiperrectángulo. */

    /**
     * @brief Constructor por defecto. Inicializa minCorner y maxCorner a cero.
     */
    HyperRectangle() : minCorner(), maxCorner() {}

    /**
     * @brief Constructor parametrizado.
     * @param minC El punto de la esquina mínima.
     * @param maxC El punto de la esquina máxima.
     */
    explicit HyperRectangle(const Point<D>& minC, const Point<D>& maxC)
        : minCorner(minC), maxCorner(maxC) {}

    /**
     * @brief Constructor de copia.
     * @param other El HyperRectangle a copiar.
     */
    HyperRectangle(const HyperRectangle<D>& other)
        : minCorner(other.minCorner), maxCorner(other.maxCorner) {}

    /**
     * @brief Operador de asignación.
     * @param other El HyperRectangle del cual se asignarán los valores.
     * @return Una referencia al HyperRectangle asignado.
     */
    HyperRectangle<D>& operator=(const HyperRectangle<D>& other) {
        if (this != &other) {
            minCorner = other.minCorner;
            maxCorner = other.maxCorner;
        }
        return *this;
    }

    /**
     * @brief Destructor.
     */
    ~HyperRectangle() = default;

    /**
     * @brief Calcula la unión de este hiperrectángulo con otro.
     * @param other El otro hiperrectángulo.
     * @return Un nuevo HyperRectangle que representa la unión.
     */
    HyperRectangle<D> Union(const HyperRectangle<D>& other) const {
        Point<D> newMin, newMax;
        for (std::size_t i = 0; i < D; ++i) {
            newMin[i] = static_cast<uint64_t>(std::min(static_cast<int64_t>(minCorner[i]), static_cast<int64_t>(other.minCorner[i])));
            newMax[i] = static_cast<uint64_t>(std::max(static_cast<int64_t>(maxCorner[i]), static_cast<int64_t>(other.maxCorner[i])));
        }
        return HyperRectangle<D>(newMin, newMax);
    }

    // --- Métodos geométricos ---
    /**
     * @brief Calcula el punto central del hiperrectángulo.
     * @return El punto central.
     */
    Point<D> center() const {
        Point<D> c;
        for (std::size_t i = 0; i < D; ++i)
            c[i] = minCorner[i] + (maxCorner[i] - minCorner[i]) / 2;
        return c;
    }

    /**
     * @brief Calcula la distancia mínima al cuadrado entre este hiperrectángulo y otro.
     * @param other El otro hiperrectángulo.
     * @return La distancia mínima al cuadrado.
     */
    uint64_t minMinDistSq(const HyperRectangle<D>& other) const {
        uint64_t sumSq = 0;
        for (std::size_t i = 0; i < D; ++i) {
            uint64_t dist = 0;
            if (maxCorner[i] < other.minCorner[i])
                dist = other.minCorner[i] - maxCorner[i];
            else if (other.maxCorner[i] < minCorner[i])
                dist = minCorner[i] - other.maxCorner[i];
            sumSq += dist * dist;
        }
        return sumSq;
    }

    /**
     * @brief Calcula la distancia máxima al cuadrado entre este hiperrectángulo y otro.
     * @param other El otro hiperrectángulo.
     * @return La distancia máxima al cuadrado.
     */
    uint64_t maxMaxDistSq(const HyperRectangle<D>& other) const {
        uint64_t sumSq = 0;
        for (std::size_t i = 0; i < D; ++i) {
            uint64_t d1 = std::llabs(static_cast<int64_t>(other.minCorner[i]) -
                                     static_cast<int64_t>(maxCorner[i]));
            uint64_t d2 = std::llabs(static_cast<int64_t>(other.maxCorner[i]) -
                                     static_cast<int64_t>(minCorner[i]));
            uint64_t maxd = std::max(d1, d2);
            sumSq += maxd * maxd;
        }
        return sumSq;
    }

    /**
     * @brief Calcula la distancia mínima al cuadrado desde un punto a este hiperrectángulo.
     * @param p El punto.
     * @return La distancia mínima al cuadrado.
     */
    uint64_t minDistSq(const Point<D>& p) const {
        uint64_t sumSq = 0;
        for (std::size_t i = 0; i < D; ++i) {
            int64_t di = 0;
            int64_t p_coord = static_cast<int64_t>(p[i]);
            int64_t min_coord = static_cast<int64_t>(minCorner[i]);
            int64_t max_coord = static_cast<int64_t>(maxCorner[i]);

            if (p_coord < min_coord) {
                di = min_coord - p_coord;
            } else if (p_coord > max_coord) {
                di = p_coord - max_coord;
            }
            sumSq += static_cast<uint64_t>(di * di);
        }
        return sumSq;
    }

    /**
     * @brief Calcula la distancia máxima al cuadrado desde un punto a este hiperrectángulo.
     * @param p El punto.
     * @return La distancia máxima al cuadrado.
     */
    uint64_t maxDistSq(const Point<D>& p) const {
        uint64_t sumSq = 0;
        for (std::size_t i = 0; i < D; ++i) {
            uint64_t di = std::max(
                std::llabs(static_cast<int64_t>(p[i]) - static_cast<int64_t>(minCorner[i])),
                std::llabs(static_cast<int64_t>(p[i]) - static_cast<int64_t>(maxCorner[i])));
            sumSq += di * di;
        }
        return sumSq;
    }

    /**
     * @brief Calcula la distancia mínima entre este hiperrectángulo y otro.
     * @param other El otro hiperrectángulo.
     * @return La distancia mínima.
     */
    double minMinDist(const HyperRectangle<D>& other) const {
        return std::sqrt(static_cast<double>(minMinDistSq(other)));
    }

    /**
     * @brief Calcula la distancia máxima entre este hiperrectángulo y otro.
     * @param other El otro hiperrectángulo.
     * @return La distancia máxima.
     */
    double maxMaxDist(const HyperRectangle<D>& other) const {
        return std::sqrt(static_cast<double>(maxMaxDistSq(other)));
    }

    /**
     * @brief Calcula la distancia mínima desde un punto a este hiperrectángulo.
     * @param p El punto.
     * @return La distancia mínima.
     */
    double minDist(const Point<D>& p) const {
        return std::sqrt(static_cast<double>(minDistSq(p)));
    }

    /**
     * @brief Calcula la distancia máxima desde un punto a este hiperrectángulo.
     * @param p El punto.
     * @return La distancia máxima.
     */
    double maxDist(const Point<D>& p) const {
        return std::sqrt(static_cast<double>(maxDistSq(p)));
    }

    /**
     * @brief Sobrecarga del operador de inserción en flujo para HyperRectangle.
     * @param os El flujo de salida.
     * @param c El HyperRectangle a imprimir.
     * @return El flujo de salida.
     */
    friend std::ostream& operator<<(std::ostream& os, const HyperRectangle<D>& c) {
        os << "HyperRectangle(min=" << c.minCorner << ", max=" << c.maxCorner << ")";
        return os;
    }
};



#endif //HAUSDORFFK3_TREE_HYPERRECTANGLE_HPP