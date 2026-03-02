/**
 * @file hausdorff.hpp
 * @brief Declara funciones para calcular la distancia de Hausdorff.
 */

#ifndef HAUSDORFF_HPP
#define HAUSDORFF_HPP

#include <vector>
#include "Point.hpp"
#include "k3tree.hpp"

/**
 * @brief Calcula la distancia de Hausdorff entre dos conjuntos de puntos 3D usando un k3-tree.
 *
 * Esta función calcula la distancia de Hausdorff simétrica H(A, B) = max(h(A, B), h(B, A)),
 * donde h(A, B) es la distancia de Hausdorff dirigida del conjunto A al conjunto B.
 * Utiliza un k3-tree para encontrar eficientemente los vecinos más cercanos,
 * acelerando significativamente el cálculo en comparación con un enfoque de fuerza bruta.
 *
 * @param setA Una referencia constante a un vector of puntos 3D que representa el primer conjunto.
 * @param setB Una referencia constante a un vector of puntos 3D que representa el segundo conjunto.
 * @return La distancia de Hausdorff entre setA y setB.
 */
double hausdorff_distance_k3(const std::vector<Point<3>>& setA, const std::vector<Point<3>>& setB);

#endif // HAUSDORFF_HPP
