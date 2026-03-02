#include <iostream>
#include <vector>
#include <chrono>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include "HausdorffK3-tree/Point.hpp"
#include "HausdorffK3-tree/ryu_kamata_3d.hpp"

// Función para cargar puntos (copiada de benchmark.cpp)
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
    std::string archivoA = "data/SINTETICOS1/DG_100M_262144_3d.txt"; // Default path
    if (argc > 1) archivoA = argv[1];
    
    int lambda = 100; // Valor por defecto
    if (argc > 2) lambda = std::stoi(argv[2]);

    std::cout << "--- Benchmark Ryu-Kamata 3D ---" << std::endl;
    std::cout << "Dataset: " << archivoA << std::endl;
    std::cout << "Lambda: " << lambda << std::endl;
    
    // Cargar puntos
    std::vector<Point<3>> todos_puntos = cargarPuntos(archivoA, 0); 
    if (todos_puntos.empty()) return 1;

    // Dividir en dos conjuntos A y B para simular
    size_t mid = todos_puntos.size() / 2;
    std::vector<Point<3>> vecA(todos_puntos.begin(), todos_puntos.begin() + mid);
    std::vector<Point<3>> vecB(todos_puntos.begin() + mid, todos_puntos.end());

    std::cout << "Set A size: " << vecA.size() << std::endl;
    std::cout << "Set B size: " << vecB.size() << std::endl;

    // Ejecutar Benchmark
    auto start = std::chrono::high_resolution_clock::now();
    double dist = hausKamata3D(vecA, vecB, lambda);
    auto end = std::chrono::high_resolution_clock::now();
    
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Distancia Ryu-Kamata 3D: " << dist << std::endl;
    std::cout << "Tiempo:    " << elapsed.count() << " s" << std::endl;

    return 0;
}
