
#include <gtest/gtest.h>
#include <HausdorffK3-tree/Point.hpp>
#include <HausdorffK3-tree/BaseLine.hpp>
#include <set>
#include <cmath>

// Prueba para la distancia de Hausdorff entre dos puntos específicos en 3D.
TEST(SpecificHausdorffTest, SinglePoints3D) {
    // Conjunto A con un solo punto (1, 0, 0)
    std::set<Point<3>> setA = {
        Point<3>({{1, 0, 0}})
    };

    // Conjunto B con un solo punto (4, 4, 2)
    std::set<Point<3>> setB = {
        Point<3>({{4, 4, 2}})
    };

    // La distancia de Hausdorff entre dos conjuntos de un solo punto
    // es simplemente la distancia euclidiana entre esos dos puntos.
    // Distancia = sqrt((4-1)^2 + (4-0)^2 + (2-0)^2)
    //           = sqrt(3^2 + 4^2 + 2^2)
    //           = sqrt(9 + 16 + 4)
    //           = sqrt(29)
    double expected_distance = std::sqrt(29.0);

    // Calcular la distancia usando la implementación
    double actual_distance = SYM_HDD_ND_Taha<3>(setA, setB);

    // Verificar que la distancia calculada es cercana a la esperada
    ASSERT_NEAR(actual_distance, expected_distance, 1e-9);
}
