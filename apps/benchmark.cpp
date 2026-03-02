
#include <iostream>
#include <fstream>
#include <set>
#include <vector>
#include <chrono>
#include <sstream>
#include <string>
#include <algorithm>
#include <HausdorffK3-tree/Point.hpp>
#include <HausdorffK3-tree/BaseLine.hpp> // Incluido por si acaso, pero no usado para Taha
#include <HausdorffK3-tree/chausdorff.hpp>

// Función para cargar puntos desde un archivo de texto (x y z)
std::vector<Point<3>> cargarPuntos(const std::string& ruta, size_t max_puntos = 0) {
    std::vector<Point<3>> puntos;
    std::ifstream archivo(ruta);
    if (!archivo.is_open()) {
        std::cerr << "Error al abrir el archivo: " << ruta << std::endl;
        return puntos;
    }

    std::string linea;
    size_t count = 0;
    while (std::getline(archivo, linea)) {
        if (max_puntos > 0 && count >= max_puntos) break;
        
        std::stringstream ss(linea);
        std::array<uint64_t, 3> coords;
        if (ss >> coords[0] >> coords[1] >> coords[2]) {
            puntos.emplace_back(coords);
            count++;
        }
    }
    std::cout << "Cargados " << puntos.size() << " puntos de " << ruta << std::endl;
    return puntos;
}

int main(int argc, char* argv[]) {
    std::string archivoA = "data/SINTETICOS1/DG_100M_262144_3d.txt"; // Archivo grande por defecto
    
    if (argc > 1) archivoA = argv[1];
    
    std::cout << "--- Benchmark Hausdorff K3-Tree Compacto con Datos Grandes ---" << std::endl;
    std::cout << "Leyendo archivo: " << archivoA << std::endl;

    // Cargar todos los puntos disponibles en el archivo, sin límite
    std::vector<Point<3>> todos_puntos = cargarPuntos(archivoA, 0); 

    if (todos_puntos.empty()) {
        std::cerr << "No se pudieron cargar puntos." << std::endl;
        return 1;
    }

    // Dividir en dos conjuntos A y B
    size_t mid = todos_puntos.size() / 2;
    std::vector<Point<3>> vecA(todos_puntos.begin(), todos_puntos.begin() + mid);
    std::vector<Point<3>> vecB(todos_puntos.begin() + mid, todos_puntos.end());
    
    std::cout << "Conjunto A: " << vecA.size() << " puntos." << std::endl;
    std::cout << "Conjunto B: " << vecB.size() << " puntos." << std::endl;

    // --- Benchmark Compacto (CK3Tree) ---
    std::cout << "\nEjecutando Compact K3-Tree..." << std::endl;
    auto start_comp = std::chrono::high_resolution_clock::now();
    double dist_comp = hausdorff_distance_compact(vecA, vecB);
    auto end_comp = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> time_comp = end_comp - start_comp;
    std::cout << "Distancia Compacta: " << dist_comp << std::endl;
    std::cout << "Tiempo Compacto:    " << time_comp.count() << " s" << std::endl;

    std::cout << "\n--- Resumen ---" << std::endl;
    std::cout << "VERIFICACIÓN: La ejecución se completó exitosamente." << std::endl;

    return 0;
}
