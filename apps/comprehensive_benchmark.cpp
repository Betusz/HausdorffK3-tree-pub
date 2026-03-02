#include <iostream>
#include <vector>
#include <chrono>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cstring>
#include <array>

#include "HausdorffK3-tree/Point.hpp"
#include "HausdorffK3-tree/chausdorff_v2.hpp"
#include "HausdorffK3-tree/ryu_kamata_3d.hpp"

// Función para cargar puntos
std::vector<Point<3>> cargarPuntos(const std::string& ruta) {
    std::vector<Point<3>> puntos;
    std::ifstream archivo(ruta);
    if (!archivo.is_open()) return puntos;
    std::string linea;
    while (std::getline(archivo, linea)) {
        std::stringstream ss(linea);
        std::array<uint64_t, 3> coords;
        if (ss >> coords[0] >> coords[1] >> coords[2]) {
            puntos.emplace_back(coords);
        }
    }
    return puntos;
}

// Función Worker: Ejecuta un algoritmo específico
int run_worker(const std::string& algo, const std::string& fileA, int lambda) {
    auto points = cargarPuntos(fileA);
    if (points.empty()) return 1;

    size_t mid = points.size() / 2;
    std::vector<Point<3>> A(points.begin(), points.begin() + mid);
    std::vector<Point<3>> B(points.begin() + mid, points.end());

    double dist = -1.0;
    if (algo == "ryu") {
        dist = hausKamata3D(A, B, lambda);
    } else if (algo == "ck3") {
        dist = hausdorff_distance_compact_v2(A, B);
    }
    
    std::cout << "RESULT_DIST=" << dist << std::endl;
    return 0;
}

// Función Manager: Lanza el worker y mide recursos
void measure_process(const std::string& title, const std::string& exe_path, const std::string& algo, const std::string& fileA, int lambda) {
    std::cout << "\n[" << title << "] Iniciando..." << std::endl;
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        std::string lambda_str = std::to_string(lambda);
        //execl(path, arg0, arg1, ..., NULL)
        execl(exe_path.c_str(), exe_path.c_str(), "--worker", algo.c_str(), fileA.c_str(), lambda_str.c_str(), NULL);
        perror("execl failed");
        exit(1);
    } else if (pid > 0) {
        // Parent process
        int status;
        struct rusage usage;
        // Esperar al hijo y obtener uso de recursos
        wait4(pid, &status, 0, &usage);
        
        auto end_time = std::chrono::high_resolution_clock::now();
        double wall_seconds = std::chrono::duration<double>(end_time - start_time).count();

        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            double cpu_user = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec / 1e6;
            double cpu_sys = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec / 1e6;
            long max_rss_kb = usage.ru_maxrss; // KB on Linux

            std::cout << ">> Wall Time: " << wall_seconds << " s" << std::endl;
            std::cout << ">> CPU User:  " << cpu_user << " s" << std::endl;
            std::cout << ">> CPU Sys:   " << cpu_sys << " s" << std::endl;
            std::cout << ">> Max RSS:   " << max_rss_kb / 1024.0 << " MB" << std::endl;
        } else {
            std::cerr << "Error: El proceso hijo falló." << std::endl;
        }
    } else {
        perror("fork failed");
    }
}

int main(int argc, char* argv[]) {
    // Modo Worker
    if (argc >= 5 && std::string(argv[1]) == "--worker") {
        std::string algo = argv[2];
        std::string file = argv[3];
        int lambda = std::stoi(argv[4]);
        return run_worker(algo, file, lambda);
    }

    // Modo Manager
    std::string archivoA = "data/SINTETICOS1/DG_1M_262144_3d.txt";
    if (argc > 1) archivoA = argv[1];
    int lambda = 100;

    std::cout << "=== Comprehensive Benchmark (Isolated Processes) ===" << std::endl;
    std::cout << "Dataset: " << archivoA << std::endl;

    // Obtener ruta del propio ejecutable para llamarse a sí mismo
    std::string self_path = argv[0];
    
    // Si la ruta no es absoluta o relativa explícita, agregar ./
    if (self_path.find('/') == std::string::npos) {
        self_path = "./" + self_path;
    }

    measure_process("Ryu-Kamata 3D", self_path, "ryu", archivoA, lambda);
    measure_process("CK3-Tree V2", self_path, "ck3", archivoA, lambda);

    return 0;
}
