#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <string>
#include <sstream>
#include <iomanip>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <filesystem>
#include <array>

#include <HausdorffK3-tree/Point.hpp>
#include <HausdorffK3-tree/chausdorff.hpp>

// --- Helper to execute shell command and capture output ---
std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen((std::string(cmd) + " 2>&1").c_str(), "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

// --- Data Loading (K3 style) ---
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
        // Try reading 3 coordinates
        if (ss >> coords[0] >> coords[1] >> coords[2]) {
            puntos.emplace_back(coords);
            count++;
        } 
        // Fallback for 2D files (fill Z with 0)
        else {
             std::stringstream ss2(linea);
             if (ss2 >> coords[0] >> coords[1]) {
                 coords[2] = 0;
                 puntos.emplace_back(coords);
                 count++;
             }
        }
    }
    return puntos;
}

// --- Write Temp File for K2 (CSV format: x,y) ---
void writeTempFileK2(const std::string& filename, const std::vector<Point<3>>& points) {
    std::ofstream out(filename);
    for (const auto& p : points) {
        // K2 expects "x,y" CSV format and handles 2D.
        // We output the first 2 dimensions.
        out << p[0] << "," << p[1] << "\n";
    }
    out.close();
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Uso: " << argv[0] << " <archivo_puntos> [path_to_k2_executable]" << std::endl;
        return 1;
    }

    std::string archivoA = argv[1];
    std::string k2_exec_path = "./build/Hausdorffk2tree/hausdorff"; // Default relative path in build dir
    if (argc > 2) k2_exec_path = argv[2];

    std::cout << "--- Comparativa Hausdorff: K3-Tree (3D) vs K2-Tree (2D Proyección) ---" << std::endl;
    std::cout << "Leyendo archivo: " << archivoA << std::endl;

    auto todos_puntos = cargarPuntos(archivoA);
    if (todos_puntos.empty()) return 1;

    // Split dataset
    size_t mid = todos_puntos.size() / 2;
    std::vector<Point<3>> vecA(todos_puntos.begin(), todos_puntos.begin() + mid);
    std::vector<Point<3>> vecB(todos_puntos.begin() + mid, todos_puntos.end());

    std::cout << "Puntos Totales: " << todos_puntos.size() << " -> A: " << vecA.size() << ", B: " << vecB.size() << std::endl;

    // --- 1. Run K3 Algorithm ---
    std::cout << "\n[1] Ejecutando K3-Tree Compacto..." << std::endl;
    auto start_k3 = std::chrono::high_resolution_clock::now();
    double dist_k3 = hausdorff_distance_compact(vecA, vecB);
    auto end_k3 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> time_k3 = end_k3 - start_k3;

    std::cout << ">> K3 Distancia: " << dist_k3 << std::endl;
    std::cout << ">> K3 Tiempo:    " << time_k3.count() << " s" << std::endl;

    // --- 2. Run K2 Algorithm ---
    std::cout << "\n[2] Ejecutando K2-Tree (External)..." << std::endl;
    
    // Prepare temp files using absolute paths
    std::filesystem::path current_dir = std::filesystem::current_path();
    std::filesystem::path tmp1_path = current_dir / "temp_k2_A.txt";
    std::filesystem::path tmp2_path = current_dir / "temp_k2_B.txt";

    std::string tmp1 = tmp1_path.string();
    std::string tmp2 = tmp2_path.string();

    writeTempFileK2(tmp1, vecA);
    writeTempFileK2(tmp2, vecB);

    // Run K2 Algorithm in Direct Mode (In-Memory)
    std::cout << "   -> Ejecutando K2 (Modo Directo)..." << std::endl;
    // Args: <set1> <set2> <experimento> <construir>
    // Experimento 0 = HDK2, Construir 2 = Directo
    std::string cmd_direct = k2_exec_path + " " + tmp1 + " " + tmp2 + " 0 2";
    
    auto start_k2 = std::chrono::high_resolution_clock::now();
    std::string out_run = exec((cmd_direct + " 2>&1").c_str());
    auto end_k2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> time_k2_wall = end_k2 - start_k2;

    // Parse K2 output
    std::cout << ">> Output K2:\n" << out_run << std::endl;
    std::cout << ">> K2 Wall Time (App Wrapper): " << time_k2_wall.count() << " s" << std::endl;

    // Clean up temp text files
    remove(tmp1.c_str());
    remove(tmp2.c_str());

    return 0;
}
