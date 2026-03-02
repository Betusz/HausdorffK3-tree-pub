#include <gtest/gtest.h>
#include <HausdorffK3-tree/Point.hpp>
#include <HausdorffK3-tree/BaseLine.hpp>
#include <HausdorffK3-tree/hausdorff.hpp>
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


TEST(K3HausdorffTest, CompareWithBaseline) {
    // --- CONJUNTOS DE PUNTOS ---
    std::set<Point<3>> setA_set = {
        Point<3>({{1, 0, 0}}),
        Point<3>({{5, 0, 0}})
    };

    std::set<Point<3>> setB_set = {
        Point<3>({{0, 0, 0}})
    };

    // Convert to vector for k3-tree implementation
    std::vector<Point<3>> setA_vec(setA_set.begin(), setA_set.end());
    std::vector<Point<3>> setB_vec(setB_set.begin(), setB_set.end());

    // --- CÁLCULO CON LA IMPLEMENTACIÓN DE LÍNEA BASE ---
    double baseline_distance = SYM_HDD_ND_Taha<3>(setA_set, setB_set);

    // --- CÁLCULO CON LA IMPLEMENTACIÓN DE K3-TREE ---
    double k3_distance = hausdorff_distance_k3(setA_vec, setB_vec);

    // --- VERIFICACIÓN ---
    ASSERT_NEAR(k3_distance, baseline_distance, 1e-9);
}

TEST(K3HausdorffTest, RandomHausdorffTest) {
    // --- CONJUNTOS DE PUNTOS ---
    std::set<Point<3>> setA_set = generarPuntos<3>(1000);
    std::set<Point<3>> setB_set = generarPuntos<3>(1000);

    // Convert to vector for k3-tree implementation
    std::vector<Point<3>> setA_vec(setA_set.begin(), setA_set.end());
    std::vector<Point<3>> setB_vec(setB_set.begin(), setB_set.end());

    // --- CÁLCULO CON LA IMPLEMENTACIÓN DE LÍNEA BASE ---
    double baseline_distance = SYM_HDD_ND_Taha<3>(setA_set, setB_set);

    // --- CÁLCULO CON LA IMPLEMENTACIÓN DE K3-TREE ---
    double k3_distance = hausdorff_distance_k3(setA_vec, setB_vec);

    // --- VERIFICACIÓN ---
    ASSERT_NEAR(k3_distance, baseline_distance, 1e-9);
}
