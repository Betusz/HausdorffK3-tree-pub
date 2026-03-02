/**
 * @file BaseLine.hpp
 * @brief Implementación de referencia (fuerza bruta) para el cálculo de la distancia de Hausdorff.
 *
 * Este archivo proporciona funciones para calcular la distancia de Hausdorff de manera
 * directa, sin optimizaciones espaciales como los k-d trees. Sirve como una línea base
 * para comparar el rendimiento de algoritmos más avanzados.
 */

#ifndef HAUSDORFFK3_TREE_BASELINE_HPP
#define HAUSDORFFK3_TREE_BASELINE_HPP
#include <cstdint>
#include <limits>
#include <vector>
#include <set>
#include "Point.hpp"

//*************************TAHA ALGORITHM******************************************************************

/**
 * @brief Calcula la distancia de Hausdorff dirigida (h(A, B)) entre dos conjuntos de puntos.
 *
 * Implementa el algoritmo de Taha con una optimización de "early break". Para cada punto en A,
 * busca el punto más cercano en B. La distancia de Hausdorff dirigida es el máximo de estas
 * distancias mínimas. Los cálculos se realizan con distancias al cuadrado para mayor eficiencia.
 *
 * @tparam D La dimensionalidad de los puntos.
 * @param A El primer conjunto de puntos (desde donde se mide la distancia).
 * @param B El segundo conjunto de puntos (hacia donde se mide la distancia).
 * @return La distancia de Hausdorff dirigida h(A, B).
 * @note Asume que los puntos de entrada están aleatorizados si provienen de un escaneo,
 *       según el comentario original.
 */
template <std::size_t D>
double HDD_ND_Taha(const std::set<Point<D>> &A, const std::set<Point<D>>  &B) {
    uint64_t cmax = 0.0, cmin;

    for (auto& a: A) {
        cmin = std::numeric_limits<uint64_t>::max();
        for (auto& b: B) {
            const uint64_t dist = a.distSq(b);
            if (dist < cmin) cmin = dist;
            if (dist <= cmax) break; // Early break
        }
        if (cmin > cmax) cmax = cmin;
    }
    return sqrt(static_cast<double>(cmax));
}

/**
 * @brief Calcula la distancia de Hausdorff simétrica (H(A, B)) entre dos conjuntos de puntos.
 *
 * La distancia de Hausdorff simétrica se define como H(A, B) = max(h(A, B), h(B, A)).
 * Esta función calcula ambas distancias dirigidas (h(A,B) y h(B,A)) y devuelve el máximo
 * de las dos, utilizando el algoritmo de Taha con optimización.
 *
 * @tparam D La dimensionalidad de los puntos.
 * @param A El primer conjunto de puntos.
 * @param B El segundo conjunto de puntos.
 * @return La distancia de Hausdorff simétrica H(A, B).
 */
template <std::size_t D>
double SYM_HDD_ND_Taha(const std::set<Point<D>> &A, const std::set<Point<D>>  &B) {
    uint64_t cmax = 0.0, cmin;
    // todos los calculos de distancia internos son usando la distancia^2.
    // distancia de hausdorff de A a B
    for (auto& a: A) {
        cmin = std::numeric_limits<uint64_t>::max();
        for (auto& b: B) {
            const uint64_t dist = a.distSq(b);
            if (dist < cmin) cmin = dist;
            if (dist <= cmax) break; // Early break
        }
        if (cmin > cmax) cmax = cmin;
    }
    // Mantenemos cmax en el valor que quedó
    // distancia de hausdorff B a A, sin borrar cmax
    for (auto& b: B) {
        cmin = std::numeric_limits<uint64_t>::max();
        for (auto& a: A) {
            const uint64_t dist = b.distSq(a);
            if (dist < cmin) cmin = dist;
            if (dist <= cmax) break; // Early break
        }
        if (cmin > cmax) cmax = cmin;
    }
    // reporto el valor final calculando la raiz cuadrada al resultado.
    return sqrt(static_cast<double>(cmax));
}

#endif //HAUSDORFFK3_TREE_BASELINE_HPP
