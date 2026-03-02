
#include <gtest/gtest.h>
#include <HausdorffK3-tree/Point.hpp>
#include <HausdorffK3-tree/BaseLine.hpp>
#include <set>
#include <cmath>

// Test para verificar el cálculo de la distancia de Hausdorff con un caso simple.
TEST(SimpleHausdorffTest, ManualVerification) {
    // --- CONJUNTOS DE PUNTOS ---
    // Conjunto A: Dos puntos en el eje X
    std::set<Point<3>> setA = {
        Point<3>({{1, 0, 0}}),
        Point<3>({{5, 0, 0}})
    };

    // Conjunto B: Un solo punto en el origen
    std::set<Point<3>> setB = {
        Point<3>({{0, 0, 0}})
    };

    // --- CÁLCULO MANUAL ---
    // La distancia de Hausdorff es max(h(A, B), h(B, A))
    //
    // 1. h(A, B): Distancia dirigida de A a B
    //    - Para cada punto en A, encontrar la distancia al punto más cercano en B.
    //    - Distancia de (1,0,0) a (0,0,0) es 1.
    //    - Distancia de (5,0,0) a (0,0,0) es 5.
    //    - El máximo de estas distancias es 5.
    //    - Por lo tanto, h(A, B) = 5.
    //
    // 2. h(B, A): Distancia dirigida de B a A
    //    - Para cada punto en B, encontrar la distancia al punto más cercano en A.
    //    - El único punto en B es (0,0,0).
    //    - El punto más cercano en A a (0,0,0) es (1,0,0).
    //    - La distancia entre (0,0,0) y (1,0,0) es 1.
    //    - Por lo tanto, h(B, A) = 1.
    //
    // 3. Distancia de Hausdorff
    //    - H(A, B) = max(h(A, B), h(B, A)) = max(5, 1) = 5.

    double expected_distance = 5.0;

    // --- CÁLCULO CON LA IMPLEMENTACIÓN ---
    double actual_distance = SYM_HDD_ND_Taha<3>(setA, setB);

    // --- VERIFICACIÓN ---
    // Usamos ASSERT_NEAR para comparar valores de punto flotante.
    ASSERT_NEAR(actual_distance, expected_distance, 1e-9);
}
