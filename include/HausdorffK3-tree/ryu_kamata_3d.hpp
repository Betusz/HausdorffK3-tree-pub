#ifndef RYU_KAMATA_3D_HPP
#define RYU_KAMATA_3D_HPP

#include <vector>
#include "Point.hpp"

// Calcula la distancia de Hausdorff usando el algoritmo de Ryu-Kamata adaptado a 3D
double hausKamata3D(const std::vector<Point<3>> &A, const std::vector<Point<3>> &B, int lambda);

#endif // RYU_KAMATA_3D_HPP
