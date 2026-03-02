#include <gtest/gtest.h>
#include <HausdorffK3-tree/Point.hpp>
#include <HausdorffK3-tree/BaseLine.hpp>
#include <HausdorffK3-tree/chausdorff.hpp> // <--- Usar la nueva implementación
#include <set>
#include <vector>
#include <cmath>
#include <random>

// ================================================================
// Genera N puntos D-dimensionales aleatorios con coordenadas enteras
// ================================================================
template<std::size_t D>
std::set<Point<D> > generarPuntos(const std::size_t n, const uint64_t maxCoord = 1000) {
    std::set<Point<D> > puntos;
    std::mt19937_64 rng(std::random_device{}());
    std::uniform_int_distribution<uint64_t> dist(0, maxCoord);

    for (std::size_t i = 0; i < n; ++i) {
        std::array<uint64_t, D> valores;
        for (std::size_t j = 0; j < D; ++j)
            valores[j] = dist(rng);

        puntos.insert(Point<D>(valores));
    }

    return puntos;
}


TEST(CK3HausdorffTest, CompareWithBaseline) { // <--- Nuevo nombre de suite de pruebas
    // --- CONJUNTOS DE PUNTOS ---
    std::set<Point<3>> setA_set = {
        Point<3>({{1, 0, 0}}),
        Point<3>({{5, 0, 0}})
    };

    std::set<Point<3>> setB_set = {
        Point<3>({{0, 0, 0}})
    };

    // Convertir a vector para la implementación de ck3-tree
    std::vector<Point<3>> setA_vec(setA_set.begin(), setA_set.end());
    std::vector<Point<3>> setB_vec(setB_set.begin(), setB_set.end());

    // --- CÁLCULO CON LA IMPLEMENTACIÓN DE LÍNEA BASE ---
    double baseline_distance = SYM_HDD_ND_Taha<3>(setA_set, setB_set);

    // --- CÁLCULO CON LA IMPLEMENTACIÓN DE CK3-TREE ---
    double ck3_distance = hausdorff_distance_compact(setA_vec, setB_vec); // <-- Usar la nueva función

    // --- VERIFICACIÓN ---
    ASSERT_NEAR(ck3_distance, baseline_distance, 1e-9);
}

TEST(CK3HausdorffTest, RandomHausdorffTest) { // <--- Nuevo nombre de suite de pruebas
    // --- CONJUNTOS DE PUNTOS ---
    std::set<Point<3>> setA_set = generarPuntos<3>(1000);
    std::set<Point<3>> setB_set = generarPuntos<3>(1000);

    // Convertir a vector para la implementación de ck3-tree
    std::vector<Point<3>> setA_vec(setA_set.begin(), setA_set.end());
    std::vector<Point<3>> setB_vec(setB_set.begin(), setB_set.end());

    // --- CÁLCULO CON LA IMPLEMENTACIÓN DE LÍNEA BASE ---
    double baseline_distance = SYM_HDD_ND_Taha<3>(setA_set, setB_set);

    // --- CÁLCULO CON LA IMPLEMENTACIÓN DE CK3-TREE ---
    double ck3_distance = hausdorff_distance_compact(setA_vec, setB_vec); // <-- Usar la nueva función

    // --- VERIFICACIÓN ---
    ASSERT_NEAR(ck3_distance, baseline_distance, 1e-9);
}
