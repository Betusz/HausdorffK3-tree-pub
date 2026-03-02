/**
 * @file experiment.cpp
 * @brief Realiza una serie de experimentos para validar y analizar el comportamiento
 *        del cálculo de la distancia de Hausdorff con k3-trees.
 *
 * Los experimentos incluyen:
 * 1.  **Sanity Check**: Compara dos nubes de puntos idénticas.
 * 2.  **Sensibilidad a la Translación**: Mide la distancia entre una nube y su versión trasladada.
 * 3.  **Sensibilidad a Outliers**: Evalúa el impacto de un punto atípico en la distancia.
 * 4.  **Escalabilidad y Rendimiento**: Mide el tiempo de ejecución con diferentes tamaños de entrada.
 *
 * Los resultados se imprimen en la consola y, en algunos casos, se guardan en archivos
 * (ej. `performance.csv`, `set_a_no_outlier.xyz`).
 */

#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <fstream>
#include <cmath>
#include <set>

#include "HausdorffK3-tree/Point.hpp"
#include "HausdorffK3-tree/hausdorff.hpp"
#include "HausdorffK3-tree/BaseLine.hpp"
#include "HausdorffK3-tree/chausdorff.hpp"
#include "HausdorffK3-tree/chausdorff_v2.hpp"

// --- Funciones de Ayuda para la Generación de Datos ---

/**
 * @brief Genera una nube de puntos 3D aleatorios dentro de un cubo.
 * @param num_points El número de puntos a generar.
 * @param min_coord La coordenada mínima para cada eje.
 * @param max_coord La coordenada máxima para cada eje.
 * @return Un vector de puntos 3D.
 */
std::vector<Point<3>> generate_random_cloud(size_t num_points, int min_coord = 0, int max_coord = 1000) {
    std::vector<Point<3>> cloud;
    cloud.reserve(num_points);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(min_coord, max_coord);

    for (size_t i = 0; i < num_points; ++i) {
        std::array<uint64_t, 3> coords = {
            static_cast<uint64_t>(distrib(gen)),
            static_cast<uint64_t>(distrib(gen)),
            static_cast<uint64_t>(distrib(gen))
        };
        cloud.emplace_back(coords);
    }
    return cloud;
}

/**
 * @brief Guarda una nube de puntos en un archivo en formato XYZ.
 * @param filename El nombre del archivo a crear.
 * @param cloud La nube de puntos a guardar.
 */
void save_cloud_to_xyz(const std::string& filename, const std::vector<Point<3>>& cloud) {
    std::ofstream outfile(filename);
    if (!outfile.is_open()) {
        std::cerr << "Error: No se pudo abrir el archivo " << filename << std::endl;
        return;
    }
    for (const auto& p : cloud) {
        outfile << p[0] << " " << p[1] << " " << p[2] << std::endl;
    }
}


// ... (código anterior)

/**
 * @brief Carga una nube de puntos desde un archivo de texto con formato X Y Z.
 */
std::vector<Point<3>> load_points_from_file(const std::string& filepath) {
    std::vector<Point<3>> cloud;
    std::ifstream infile(filepath);
    if (!infile.is_open()) {
        std::cerr << "Error: No se pudo abrir el archivo " << filepath << std::endl;
        return cloud;
    }
    
    uint64_t x, y, z;
    while (infile >> x >> y >> z) {
        cloud.emplace_back(std::array<uint64_t, 3>{x, y, z});
    }
    return cloud;
}

/**
 * @brief Punto de entrada principal para la ejecución de los experimentos.
 *
 * Este programa ejecuta varios escenarios de prueba para la función `hausdorff_distance_k3`,
 * evaluando su corrección y rendimiento. Los resultados se muestran en la consola y
 * se guardan archivos de datos para su posterior análisis.
 *
 * @return 0 si la ejecución es exitosa.
 */
int main() {
    std::cout << "Iniciando experimentos para la distancia de Hausdorff..." << std::endl;

    // ... (Escenarios 1, 2, 3 se mantienen igual) ...

    // =================================================================
    // Escenario 1: Sanity Check (Nubes idénticas)
    // =================================================================
    std::cout << "\n--- Escenario 1: Sanity Check ---" << std::endl;
    {
        auto cloud_a = generate_random_cloud(1000);
        auto cloud_b = cloud_a; // Copia exacta
        double distance = hausdorff_distance_k3(cloud_a, cloud_b);
        std::cout << "Distancia entre nubes idénticas: " << distance << " (Esperado: 0)" << std::endl;
    }

    // =================================================================
    // Escenario 1.5: Verificación de Precisión (Correctness)
    // =================================================================
    std::cout << "\n--- Escenario 1.5: Verificación de Precisión ---" << std::endl;
    {
        std::cout << "Generando nubes aleatorias distintas (N=1000)..." << std::endl;
        auto cloud_a = generate_random_cloud(1000);
        auto cloud_b = generate_random_cloud(1000);

        std::cout.precision(15); // Alta precisión para ver decimales

        double d_k3 = hausdorff_distance_k3(cloud_a, cloud_b);
        double d_cpt = hausdorff_distance_compact(cloud_a, cloud_b);
        double d_cpt2 = hausdorff_distance_compact_v2(cloud_a, cloud_b);

        std::cout << "Resultados:" << std::endl;
        std::cout << "  K3 (Punteros): " << d_k3 << std::endl;
        std::cout << "  Compact (V1):  " << d_cpt << std::endl;
        std::cout << "  Compact (V2):  " << d_cpt2 << std::endl;

        double diff = std::abs(d_k3 - d_cpt2);
        std::cout << "Diferencia |K3 - V2|: " << diff << std::endl;

        if (diff < 1e-9) {
            std::cout << "VALIDACIÓN: ÉXITO. Los resultados son idénticos." << std::endl;
        } else {
            std::cout << "VALIDACIÓN: FALLO. Hay discrepancia significativa." << std::endl;
        }
        std::cout.precision(6); // Restaurar precisión normal
    }

    // =================================================================
    // Escenario 2: Sensibilidad a la Translación
    // =================================================================
    std::cout << "\n--- Escenario 2: Sensibilidad a la Translación ---" << std::endl;
    {
        const std::array<int64_t, 3> translation = {100, 200, -50};
        auto cloud_a = generate_random_cloud(1000, 0, 500);
        std::vector<Point<3>> cloud_b;
        cloud_b.reserve(cloud_a.size());

        for (const auto& p_a : cloud_a) {
            std::array<uint64_t, 3> translated_coords = {
                static_cast<uint64_t>(p_a[0] + translation[0]),
                static_cast<uint64_t>(p_a[1] + translation[1]),
                static_cast<uint64_t>(p_a[2] + translation[2])
            };
            cloud_b.emplace_back(translated_coords);
        }

        double distance = hausdorff_distance_k3(cloud_a, cloud_b);
        double expected_distance = std::sqrt(
            translation[0] * translation[0] +
            translation[1] * translation[1] +
            translation[2] * translation[2]
        );

        std::cout << "Distancia entre nube original y transladada: " << distance << std::endl;
        std::cout << "Distancia esperada (magnitud de la translación): " << expected_distance << std::endl;
    }


    // =================================================================
    // Escenario 3: Sensibilidad a los Outliers
    // =================================================================
    std::cout << "\n--- Escenario 3: Sensibilidad a los Outliers ---" << std::endl;
    {
        auto cloud_a = generate_random_cloud(1000, 0, 500);
        auto cloud_b = cloud_a;

        const Point<3> outlier({5000, 5000, 5000});
        cloud_b.push_back(outlier);

        double distance = hausdorff_distance_k3(cloud_a, cloud_b);

        std::cout << "Distancia con un outlier añadido: " << distance << std::endl;
        std::cout << "Se esperaba una distancia grande, dominada por el outlier." << std::endl;

        save_cloud_to_xyz("set_a_no_outlier.xyz", cloud_a);
        save_cloud_to_xyz("set_b_with_outlier.xyz", cloud_b);
        std::cout << "Nubes de puntos guardadas en 'set_a_no_outlier.xyz' y 'set_b_with_outlier.xyz' para visualización." << std::endl;
    }


    // =================================================================
    // Escenario 4: Escalabilidad y Rendimiento (Sintético Aleatorio)
    // =================================================================
    std::cout << "\n--- Escenario 4: Escalabilidad y Rendimiento (Sintético Aleatorio) ---" << std::endl;
    {
        std::ofstream perf_file("performance.csv");
        if (!perf_file.is_open()) {
            std::cerr << "Error: No se pudo abrir performance.csv" << std::endl;
        } else {
            // Header del CSV
            perf_file << "N_Points,Time_K3_ms,Time_Compact_ms,Time_CompactV2_ms,Time_BruteForce_ms\n";
            std::cout << "Generando datos para performance.csv (K3 vs Compact vs CompactV2 vs BruteForce)..." << std::endl;

            // Tamaños de prueba. Limitamos Brute Force para N grandes.
            const std::vector<size_t> sizes = {50, 100, 250, 500, 750, 1000, 2500, 5000, 7500, 10000, 25000, 50000};
            // Límite para ejecutar Brute Force (se vuelve muy lento O(N^2))
            const size_t bf_limit = 10000;

            for (const auto n_points : sizes) {
                std::cout << "Procesando N = " << n_points << "..." << std::endl;
                auto cloud_a = generate_random_cloud(n_points);
                auto cloud_b = generate_random_cloud(n_points);

                // --- 1. K3-Tree Normal ---
                long long time_k3 = 0;
                {
                    auto start = std::chrono::high_resolution_clock::now();
                    hausdorff_distance_k3(cloud_a, cloud_b);
                    auto end = std::chrono::high_resolution_clock::now();
                    time_k3 = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
                }

                // --- 2. K3-Tree Compacto (V1) ---
                long long time_compact = 0;
                {
                    auto start = std::chrono::high_resolution_clock::now();
                    hausdorff_distance_compact(cloud_a, cloud_b);
                    auto end = std::chrono::high_resolution_clock::now();
                    time_compact = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
                }

                // --- 3. K3-Tree Compacto (V2 - Optimized) ---
                long long time_compact_v2 = 0;
                {
                    auto start = std::chrono::high_resolution_clock::now();
                    hausdorff_distance_compact_v2(cloud_a, cloud_b);
                    auto end = std::chrono::high_resolution_clock::now();
                    time_compact_v2 = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
                }

                // --- 4. Brute Force (Taha) ---
                long long time_bf = -1; // -1 indica que no se ejecutó
                if (n_points <= bf_limit) {
                    // Convertir a sets para la implementación BaseLine
                    std::set<Point<3>> set_a(cloud_a.begin(), cloud_a.end());
                    std::set<Point<3>> set_b(cloud_b.begin(), cloud_b.end());

                    auto start = std::chrono::high_resolution_clock::now();
                    SYM_HDD_ND_Taha<3>(set_a, set_b);
                    auto end = std::chrono::high_resolution_clock::now();
                    time_bf = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
                }

                // Salida a consola
                std::cout << "  -> K3: " << time_k3 << " ms, Cpt: " << time_compact << " ms, CptV2: " << time_compact_v2 << " ms";
                if (time_bf != -1) {
                    std::cout << ", BF: " << time_bf << " ms";
                } else {
                    std::cout << ", BF: SKIPPED";
                }
                std::cout << std::endl;

                // Escribir a archivo
                perf_file << n_points << "," << time_k3 << "," << time_compact << "," << time_compact_v2 << ",";
                if (time_bf != -1) perf_file << time_bf;
                perf_file << "\n";
            }
            std::cout << "Archivo 'performance.csv' generado correctamente." << std::endl;
        }
    }

    // =================================================================
    // Escenario 5: Benchmark con Datasets Reales (data/SINTETICOS1)
    // =================================================================
    std::cout << "\n--- Escenario 5: Benchmark con Datasets Reales ---" << std::endl;
    {
        std::ofstream real_perf_file("performance_real.csv");
        if (!real_perf_file.is_open()) {
            std::cerr << "Error: No se pudo abrir performance_real.csv" << std::endl;
        } else {
            real_perf_file << "Dataset,Total_Points,Points_A,Points_B,Time_K3_ms,Time_Compact_ms,Time_CompactV2_ms\n";
            
            // Lista de archivos a probar
            // Lista de archivos a probar
            std::vector<std::string> datasets = {
                "../data/SINTETICOS1/DG_1M_262144_3d.txt",
                "../data/SINTETICOS1/DH_1M_262144_3d.txt",
                "../data/SINTETICOS1/DG_10M_262144_3d.txt",
                "../data/SINTETICOS1/DH_10M_262144_3d.txt"
            };

            for (const auto& ds_path : datasets) {
                std::cout << "Cargando dataset: " << ds_path << "..." << std::endl;
                auto all_points = load_points_from_file(ds_path);
                
                if (all_points.empty()) {
                    std::cout << "  -> Saltando dataset vacío o no encontrado." << std::endl;
                    continue;
                }

                size_t total = all_points.size();
                size_t half = total / 2;
                
                // Dividir en dos nubes
                std::vector<Point<3>> cloud_a(all_points.begin(), all_points.begin() + half);
                std::vector<Point<3>> cloud_b(all_points.begin() + half, all_points.end());

                std::cout << "  -> Total: " << total << ", A: " << cloud_a.size() << ", B: " << cloud_b.size() << std::endl;

                // 1. K3
                long long t_k3 = 0;
                {
                    auto start = std::chrono::high_resolution_clock::now();
                    hausdorff_distance_k3(cloud_a, cloud_b);
                    auto end = std::chrono::high_resolution_clock::now();
                    t_k3 = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
                }

                // 2. Compact
                long long t_cpt = 0;
                {
                    auto start = std::chrono::high_resolution_clock::now();
                    hausdorff_distance_compact(cloud_a, cloud_b);
                    auto end = std::chrono::high_resolution_clock::now();
                    t_cpt = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
                }

                // 3. Compact V2
                long long t_cpt2 = 0;
                {
                    auto start = std::chrono::high_resolution_clock::now();
                    hausdorff_distance_compact_v2(cloud_a, cloud_b);
                    auto end = std::chrono::high_resolution_clock::now();
                    t_cpt2 = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
                }

                std::cout << "  -> K3: " << t_k3 << " ms, Cpt: " << t_cpt << " ms, CptV2: " << t_cpt2 << " ms" << std::endl;
                
                // Nombre archivo corto
                std::string ds_name = ds_path.substr(ds_path.find_last_of("/\\") + 1);
                
                real_perf_file << ds_name << "," << total << "," << cloud_a.size() << "," << cloud_b.size() << ","
                               << t_k3 << "," << t_cpt << "," << t_cpt2 << "\n";
            }
        }
    }

    std::cout << "\nExperimentos finalizados." << std::endl;

    return 0;
}
