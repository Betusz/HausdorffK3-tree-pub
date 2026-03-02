#include <iostream>
#include <set>
#include <random>
#include <array>
#include <cstdint>
#include <vector> // Necesario para los vectores de puntos
#include <chrono> // Para medir el tiempo

// Incluir tus nuevas cabeceras
#include <HausdorffK3-tree/Point.hpp>
#include <HausdorffK3-tree/HyperRectangle.hpp>
#include <HausdorffK3-tree/BaseLine.hpp> // Para la versión Taha (línea base)


/**
 * @brief Genera un conjunto de puntos D-dimensionales aleatorios.
 *
 * @tparam D La dimensionalidad de los puntos a generar.
 * @param n El número de puntos a generar.
 * @param maxCoord El valor máximo para cada coordenada (por defecto es 1000).
 * @return Un `std::vector` que contiene los puntos generados.
 */
template<std::size_t D>
std::vector<Point<D> > generarPuntos(const std::size_t n, const uint64_t maxCoord = 1000) {
    std::vector<Point<D> > puntos;
    puntos.reserve(n); // Pre-reservar memoria
    std::mt19937_64 rng(std::random_device{}());
    std::uniform_int_distribution<uint64_t> dist(0, maxCoord);

    // Usar un set temporal para asegurar puntos únicos, luego convertir a vector
    std::set<Point<D> > unique_puntos;
    while(unique_puntos.size() < n) {
        std::array<uint64_t, D> valores;
        for (std::size_t j = 0; j < D; ++j)
            valores[j] = dist(rng);
        unique_puntos.insert(Point<D>(valores));
    }

    for(const auto& p : unique_puntos) {
        puntos.push_back(p);
    }
    return puntos;
}

/**
 * @brief Realiza pruebas básicas de las funcionalidades de la clase HyperRectangle.
 *
 * Crea dos hiperrectángulos y un punto, y luego calcula y muestra varias
 * distancias entre ellos (minMinDist, maxMaxDist, minDist, maxDist).
 */
void test_hyperRectangle() {
    Point<3> aMin({{0, 0, 0}});
    Point<3> aMax({{3, 3, 3}});
    Point<3> bMin({{5, 1, 0}});
    Point<3> bMax({{7, 3, 2}});
    HyperRectangle<3> A(aMin, aMax);
    HyperRectangle<3> B(bMin, bMax);
    Point<3> p({{8, 5, 1}});

    std::cout << A << "\n" << B << "\n\n";

    std::cout << "minMinDist(A,B) = " << A.minMinDist(B) << "\n";
    std::cout << "maxMaxDist(A,B) = " << A.maxMaxDist(B) << "\n";
    std::cout << "minDist(p,A) = " << A.minDist(p) << "\n";
    std::cout << "maxDist(p,A) = " << A.maxDist(p) << "\n";
}

/**
 * @brief Realiza pruebas de las funcionalidades de la clase Point y el cálculo de Hausdorff.
 *
 * Crea puntos, calcula distancias, códigos de Morton y compara puntos.
 * También genera dos grandes conjuntos de puntos aleatorios y calcula la distancia
 * de Hausdorff simétrica entre ellos utilizando la implementación de referencia (Taha)
 * y la nueva implementación compacta.
 */
void test_point_and_hausdorff() {
    //creación de dos puntos 3D
    Point<3> a({{1, 2, 3}});
    Point<3> b({{2, 1, 3}});

    std::cout << "a = " << a << ", b = " << b << "\n";
    std::cout << "Distancia: " << a.distanciaA(b) << "\n";
    std::cout << "Morton(a): " << a.toMorton() << "\n";
    std::cout << "Morton(b): " << b.toMorton() << "\n";

    std::cout << "a < b? " << (a < b) << "\n";

    std::set<Point<3> > puntos_set = {a, b}; // Usar un set aquí por cómo está diseñado el test original
    for (auto &p: puntos_set) std::cout << p << " ";
    std::cout << "\n";

    // Genera dos conjuntos de puntos y calcula la distancia de Hausdorff
    const size_t num_points = 10000;
    const uint64_t max_coord = 10000; // Aumentar rango para árboles más grandes

    std::cout << "Generando conjuntos A y B con " << num_points << " puntos cada uno (MaxCoord: " << max_coord << ")..." << std::endl;
    std::vector<Point<3> > A_vec = generarPuntos<3>(num_points, max_coord);
    std::vector<Point<3> > B_vec = generarPuntos<3>(num_points, max_coord);
    std::cout << "Puntos generados." << std::endl;

    // --- Cálculo con la línea base (Taha) ---
    std::cout << "Calculando la Distancia de Hausdorff (Taha - línea base)..." << std::endl;
    std::set<Point<3> > A_set(A_vec.begin(), A_vec.end()); // Convertir a set para Taha
    std::set<Point<3> > B_set(B_vec.begin(), B_vec.end());
    auto start_taha = std::chrono::high_resolution_clock::now();
    double h_taha = SYM_HDD_ND_Taha<3>(A_set, B_set);
    auto end_taha = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff_taha = end_taha - start_taha;
    std::cout << "Distancia simétrica (Taha) = " << h_taha << " (Tiempo: " << diff_taha.count() << "s)\n";

    // --- Cálculo con tu nueva implementación compacta ---
    std::cout << "Calculando la Distancia de Hausdorff (Compacta - CK3Tree)..." << std::endl;
    auto start_compact = std::chrono::high_resolution_clock::now();
    double h_compact = hausdorff_distance_compact(A_vec, B_vec);
    auto end_compact = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff_compact = end_compact - start_compact;
    std::cout << "Distancia simétrica (Compacta) = " << h_compact << " (Tiempo: " << diff_compact.count() << "s)\n";

    // Comparación simple de resultados (puede haber pequeñas diferencias por precisión flotante)
    double epsilon = 1e-6; // Tolerancia para comparar doubles
    if (std::abs(h_taha - h_compact) < epsilon) {
        std::cout << "Resultados de Hausdorff (Taha vs Compacta) son similares." << std::endl;
    } else {
        std::cout << "ADVERTENCIA: Resultados de Hausdorff (Taha vs Compacta) DIFIEREN SIGNIFICATIVAMENTE." << std::endl;
    }
}

/**
 * @brief Punto de entrada principal de la aplicación de demostración.
 */
int main() {
    std::cout << "Aplicacion principal iniciada." << std::endl;

    // Ejemplo de uso de la biblioteca:
    Point<3> a({{1, 2, 3}});
    std::cout << "Punto de ejemplo: " << a << std::endl;

    std::cout << "--- Test Point and Hausdorff Distances ---" << std::endl;
    test_point_and_hausdorff();
    std::cout << "--- Test HyperRectangle ---" << std::endl;
    test_hyperRectangle();
    
    std::cout << "Aplicacion principal finalizada." << std::endl;
    return 0;
}