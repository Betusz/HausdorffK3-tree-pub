#include <gtest/gtest.h>
#include <HausdorffK3-tree/hausdorff.hpp>
#include <HausdorffK3-tree/k3tree.hpp>
#include <random>
#include <vector>
#include <algorithm>
#include <cmath>
#include <limits>
#include <array>

using namespace k3tree;

// Implementación ingenua (Fuerza Bruta) para Ground Truth
double naive_hausdorff(const std::vector<Point<3>>& A, const std::vector<Point<3>>& B) {
    if (A.empty() || B.empty()) return 0.0;

    // h(A, B)
    double max_dist_sq_AB = 0;
    for (const auto& a : A) {
        uint64_t min_dist_sq = std::numeric_limits<uint64_t>::max();
        for (const auto& b : B) {
            uint64_t d = a.distSq(b);
            if (d < min_dist_sq) min_dist_sq = d;
        }
        if (min_dist_sq > max_dist_sq_AB) max_dist_sq_AB = min_dist_sq;
    }

    // h(B, A)
    double max_dist_sq_BA = 0;
    for (const auto& b : B) {
        uint64_t min_dist_sq = std::numeric_limits<uint64_t>::max();
        for (const auto& a : A) {
            uint64_t d = b.distSq(a);
            if (d < min_dist_sq) min_dist_sq = d;
        }
        if (min_dist_sq > max_dist_sq_BA) max_dist_sq_BA = min_dist_sq;
    }

    return std::sqrt(std::max(max_dist_sq_AB, max_dist_sq_BA));
}

TEST(OctreeHausdorff, SmallSetCheck) {
    // Caso de prueba simple manual
    // A: (0,0,0), (10,0,0)
    // B: (2,0,0), (8,0,0)
    
    // Distancias A->B:
    // (0,0,0) -> (2,0,0) es dist 2.
    // (10,0,0) -> (8,0,0) es dist 2.
    // max(2, 2) = 2.
    
    // Distancias B->A:
    // (2,0,0) -> (0,0,0) es dist 2.
    // (8,0,0) -> (10,0,0) es dist 2.
    // max(2, 2) = 2.
    
    // Hausdorff = 2.
    
    std::vector<Point<3>> setA;
    setA.push_back(Point<3>(std::array<uint64_t, 3>{0,0,0}));
    setA.push_back(Point<3>(std::array<uint64_t, 3>{10,0,0}));
    
    std::vector<Point<3>> setB;
    setB.push_back(Point<3>(std::array<uint64_t, 3>{2,0,0}));
    setB.push_back(Point<3>(std::array<uint64_t, 3>{8,0,0}));
    
    double result = hausdorff_distance_k3(setA, setB);
    EXPECT_NEAR(result, 2.0, 1e-9);
}

TEST(OctreeHausdorff, RandomPointsComparision) {
    std::mt19937_64 rng(42); // Semilla fija para reproducibilidad
    std::uniform_int_distribution<uint64_t> dist(0, 5000);

    size_t N = 200; // Número de puntos
    std::vector<Point<3>> setA, setB;

    for (size_t i = 0; i < N; ++i) {
        setA.push_back(Point<3>(std::array<uint64_t, 3>{dist(rng), dist(rng), dist(rng)}));
        setB.push_back(Point<3>(std::array<uint64_t, 3>{dist(rng), dist(rng), dist(rng)}));
    }

    double expected = naive_hausdorff(setA, setB);
    double actual = hausdorff_distance_k3(setA, setB);

    // Permitimos un margen de error pequeño por operaciones de punto flotante
    EXPECT_NEAR(actual, expected, 1e-5) 
        << "Naive result: " << expected << ", Octree result: " << actual;
}
