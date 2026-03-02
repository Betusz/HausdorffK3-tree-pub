#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <fstream>
#include <cmath>
#include <set>
#include <string>
#include <iomanip>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cstring>
#include <array>

#include "HausdorffK3-tree/Point.hpp"
#include "HausdorffK3-tree/hausdorff.hpp"
#include "HausdorffK3-tree/BaseLine.hpp"
#include "HausdorffK3-tree/chausdorff.hpp"
#include "HausdorffK3-tree/chausdorff_v2.hpp"
#include "HausdorffK3-tree/ryu_kamata_3d.hpp"

// --- Helper Functions ---

std::vector<Point<3>> generate_random_cloud(size_t num_points, int seed = 42, int min_coord = 0, int max_coord = 1000) {
    std::vector<Point<3>> cloud;
    cloud.reserve(num_points);
    std::mt19937 gen(seed);
    std::uniform_int_distribution<> distrib(min_coord, max_coord);
    for (size_t i = 0; i < num_points; ++i) {
        cloud.emplace_back(std::array<uint64_t, 3>{
            static_cast<uint64_t>(distrib(gen)),
            static_cast<uint64_t>(distrib(gen)),
            static_cast<uint64_t>(distrib(gen))
        });
    }
    return cloud;
}

std::vector<Point<3>> load_points_from_file(const std::string& filepath) {
    std::vector<Point<3>> cloud;
    std::ifstream infile(filepath);
    if (!infile.is_open()) return cloud;
    uint64_t x, y, z;
    while (infile >> x >> y >> z) {
        cloud.emplace_back(std::array<uint64_t, 3>{x, y, z});
    }
    return cloud;
}

// --- WORKER LOGIC ---

int run_worker_synthetic(const std::string& algo, size_t n_points, int seed) {
    auto cloud_a = generate_random_cloud(n_points, seed);
    auto cloud_b = generate_random_cloud(n_points, seed + 1); // Diferente seed para B

    double dist = -1.0;
    if (algo == "k3") dist = hausdorff_distance_k3(cloud_a, cloud_b);
    else if (algo == "ck3v1") dist = hausdorff_distance_compact(cloud_a, cloud_b);
    else if (algo == "ck3v2") dist = hausdorff_distance_compact_v2(cloud_a, cloud_b);
    else if (algo == "ryu") dist = hausKamata3D(cloud_a, cloud_b, 100);
    else if (algo == "bf") {
        std::set<Point<3>> set_a(cloud_a.begin(), cloud_a.end());
        std::set<Point<3>> set_b(cloud_b.begin(), cloud_b.end());
        dist = SYM_HDD_ND_Taha<3>(set_a, set_b);
    }
    // std::cout << "DIST=" << dist << std::endl; // Opcional 
    return 0; // Success
}

int run_worker_file(const std::string& algo, const std::string& filepath) {
    auto all_points = load_points_from_file(filepath);
    if (all_points.empty()) return 1;
    size_t mid = all_points.size() / 2;
    std::vector<Point<3>> A(all_points.begin(), all_points.begin() + mid);
    std::vector<Point<3>> B(all_points.begin() + mid, all_points.end());

    double dist = -1.0;
    if (algo == "k3") dist = hausdorff_distance_k3(A, B);
    else if (algo == "ck3v1") dist = hausdorff_distance_compact(A, B);
    else if (algo == "ck3v2") dist = hausdorff_distance_compact_v2(A, B);
    else if (algo == "ryu") dist = hausKamata3D(A, B, 100);
    
    // std::cout << "DIST=" << dist << std::endl;
    return 0;
}

// --- MANAGER LOGIC ---

struct RunResult {
    double wall_time_ms;
    long max_rss_kb;
    bool success;
};

RunResult measure_execution(const std::string& exe_path, const std::vector<std::string>& args) {
    auto start_time = std::chrono::high_resolution_clock::now();
    pid_t pid = fork();
    if (pid == 0) {
        // Child
        std::vector<char*> c_args;
        c_args.push_back(strdup(exe_path.c_str()));
        for (const auto& arg : args) c_args.push_back(strdup(arg.c_str()));
        c_args.push_back(NULL);
        execv(exe_path.c_str(), c_args.data());
        exit(1);
    } else if (pid > 0) {
        // Parent
        int status;
        struct rusage usage;
        wait4(pid, &status, 0, &usage);
        auto end_time = std::chrono::high_resolution_clock::now();
        
        RunResult res;
        res.wall_time_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();
        res.max_rss_kb = usage.ru_maxrss;
        res.success = WIFEXITED(status) && WEXITSTATUS(status) == 0;
        return res;
    }
    return {0, 0, false};
}

int main(int argc, char* argv[]) {
    // --- WORKER DISPATCH ---
    if (argc > 1 && std::string(argv[1]) == "--worker") {
        std::string mode = argv[2]; // "synth" or "file"
        std::string algo = argv[3];
        if (mode == "synth") {
            size_t n = std::stoul(argv[4]);
            int seed = std::stoi(argv[5]);
            return run_worker_synthetic(algo, n, seed);
        } else if (mode == "file") {
            std::string path = argv[4];
            return run_worker_file(algo, path);
        }
        return 1;
    }

    // --- MANAGER (MAIN) ---
    std::string self_path = argv[0];
    // Asegurar ruta absoluta o relativa válida para execv
    if (self_path.find('/') == std::string::npos) self_path = "./" + self_path;

    std::cout << "=== EXPERIMENT SUITE (Isolated Process & Resource Monitor) ===" << std::endl;

    // 1. Escenario Sintético
    std::cout << "\n--- [Escenario Sintético] Escalabilidad ---" << std::endl;
    std::ofstream csv_synth("performance_isolated.csv");
    csv_synth << "N,Algo,Time_ms,RAM_KB\n";
    
    std::vector<size_t> sizes = {50, 500, 1000, 5000, 10000, 50000}; 
    std::vector<std::string> algos = {"k3", "ck3v1", "ck3v2", "ryu"};
    // Añadimos BF solo para N pequeños
    
    for (size_t n : sizes) {
        for (const auto& algo : algos) {
            std::cout << "Running N=" << n << " Algo=" << algo << " ... " << std::flush;
            RunResult res = measure_execution(self_path, {"--worker", "synth", algo, std::to_string(n), "42"});
            
            if (res.success) {
                std::cout << res.wall_time_ms << " ms, " << res.max_rss_kb << " KB" << std::endl;
                csv_synth << n << "," << algo << "," << res.wall_time_ms << "," << res.max_rss_kb << "\n";
            } else {
                std::cout << "FAILED" << std::endl;
            }
        }
        // BF check
        if (n <= 5000) {
             std::cout << "Running N=" << n << " Algo=bf ... " << std::flush;
             RunResult res = measure_execution(self_path, {"--worker", "synth", "bf", std::to_string(n), "42"});
             if (res.success) {
                std::cout << res.wall_time_ms << " ms, " << res.max_rss_kb << " KB" << std::endl;
                csv_synth << n << ",bf," << res.wall_time_ms << "," << res.max_rss_kb << "\n";
             }
        }
    }
    csv_synth.close();

    // 2. Escenario Real
    std::cout << "\n--- [Escenario Real] Datasets ---" << std::endl;
    std::ofstream csv_real("performance_real_isolated.csv");
    csv_real << "Dataset,Algo,Time_ms,RAM_KB\n";

    std::vector<std::string> datasets = {
        "data/SINTETICOS1/DG_1M_262144_3d.txt",
        "data/SINTETICOS1/DH_1M_262144_3d.txt",
        "data/SINTETICOS1/DG_10M_262144_3d.txt" 
    };

    for (const auto& ds : datasets) {
        std::cout << "Dataset: " << ds << std::endl;
        for (const auto& algo : algos) { // k3, ck3v1, ck3v2, ryu
             std::cout << "  Algo=" << algo << " ... " << std::flush;
             RunResult res = measure_execution(self_path, {"--worker", "file", algo, ds});
             if (res.success) {
                std::cout << res.wall_time_ms << " ms, " << res.max_rss_kb << " KB" << std::endl;
                std::string ds_name = ds.substr(ds.find_last_of("/\\") + 1);
                csv_real << ds_name << "," << algo << "," << res.wall_time_ms << "," << res.max_rss_kb << "\n";
             } else {
                 std::cout << "FAILED (Check file path?)" << std::endl;
             }
        }
    }
    csv_real.close();

    return 0;
}