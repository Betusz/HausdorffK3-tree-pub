#include <iostream>
#include <vector>
#include <chrono>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <random>
#include <set>

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#else
#include <sys/resource.h>
#endif

#include "HausdorffK3-tree/Point.hpp"
#include "HausdorffK3-tree/BaseLine.hpp"
#include "HausdorffK3-tree/hausdorff.hpp"
#include "HausdorffK3-tree/chausdorff_v2.hpp"
#include "HausdorffK3-tree/ryu_kamata_3d.hpp"

using namespace std;

// --- Utility Functions ---

template<std::size_t D>
vector<Point<D>> generateRandomPoints(size_t n, uint64_t maxCoord = 1000000) {
    vector<Point<D>> points;
    points.reserve(n);
    mt19937_64 rng(random_device{}());
    uniform_int_distribution<uint64_t> dist(0, maxCoord);

    set<Point<D>> unique_points;
    while(unique_points.size() < n) {
        array<uint64_t, D> coords;
        for (size_t j = 0; j < D; ++j) coords[j] = dist(rng);
        unique_points.insert(Point<D>(coords));
    }

    for(const auto& p : unique_points) points.push_back(p);
    return points;
}

vector<Point<3>> loadPoints(const string& path, size_t limit = 0) {
    vector<Point<3>> points;
    ifstream file(path);
    if (!file.is_open()) {
        cerr << "Error: Could not open file " << path << endl;
        return points;
    }
    string line;
    size_t count = 0;
    while (getline(file, line) && (limit == 0 || count < limit)) {
        stringstream ss(line);
        array<uint64_t, 3> coords;
        if (ss >> coords[0] >> coords[1] >> coords[2]) {
            points.emplace_back(coords);
            count++;
        }
    }
    return points;
}

void printMemoryUsage() {
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        cout << "   Peak Memory: " << pmc.PeakWorkingSetSize / (1024.0 * 1024.0) << " MB" << endl;
    }
#else
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
        // En Linux ru_maxrss está en KB
        cout << "   Peak Memory: " << usage.ru_maxrss / 1024.0 << " MB" << endl;
    }
#endif
}

// --- Benchmark Runner ---

struct Result {
    string algo;
    double distance;
    double wallTime;
};

void runBenchmark(const string& name, const vector<Point<3>>& A, const vector<Point<3>>& B, const string& selectedAlgo) {
    if (selectedAlgo != "all" && selectedAlgo != name) return;

    cout << "\n[Algorithm: " << name << "]" << endl;

    
    auto start = chrono::high_resolution_clock::now();
    double dist = -1.0;

    if (name == "taha") {
        set<Point<3>> setA(A.begin(), A.end());
        set<Point<3>> setB(B.begin(), B.end());
        dist = SYM_HDD_ND_Taha<3>(setA, setB);
    } else if (name == "k3") {
        dist = hausdorff_distance_k3(A, B);
    } else if (name == "ck3") {
        dist = hausdorff_distance_compact_v2(A, B);
    } else if (name == "ryu") {
        dist = hausKamata3D(A, B, 100); // Default lambda=100
    }

    auto end = chrono::high_resolution_clock::now();
    double wallTime = chrono::duration<double>(end - start).count();

    cout << "   Distance: " << fixed << setprecision(4) << dist << endl;
    cout << "   Wall Time: " << wallTime << " s" << endl;
    printMemoryUsage();
}

void showHelp() {
    cout << "Usage: snapshot_benchmark [options]" << endl;
    cout << "Options:" << endl;
    cout << "  --random <N>      Generate 2N random points (N for A, N for B)" << endl;
    cout << "  --file <PATH>     Load points from file and split into A and B" << endl;
    cout << "  --limit <N>       Limit total points from file (default: all)" << endl;
    cout << "  --algo <ALGO>     Algorithm: taha, k3, ck3, ryu, all (default: all)" << endl;
    cout << "  --help            Show this help message" << endl;
}

int main(int argc, char* argv[]) {
    if (argc == 1) {
        showHelp();
        return 0;
    }

    size_t n_random = 0;
    string file_path = "";
    size_t limit = 0;
    string selected_algo = "all";

    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        if (arg == "--random" && i + 1 < argc) n_random = stoull(argv[++i]);
        else if (arg == "--file" && i + 1 < argc) file_path = argv[++i];
        else if (arg == "--limit" && i + 1 < argc) limit = stoull(argv[++i]);
        else if (arg == "--algo" && i + 1 < argc) selected_algo = argv[++i];
        else if (arg == "--help") { showHelp(); return 0; }
    }

    vector<Point<3>> A, B;

    if (n_random > 0) {
        cout << "Generating " << n_random << " random points for each set..." << endl;
        A = generateRandomPoints<3>(n_random);
        B = generateRandomPoints<3>(n_random);
    } else if (!file_path.empty()) {
        cout << "Loading points from " << file_path << "..." << endl;
        auto all_points = loadPoints(file_path, limit);
        if (all_points.empty()) return 1;
        
        size_t mid = all_points.size() / 2;
        A.assign(all_points.begin(), all_points.begin() + mid);
        B.assign(all_points.begin() + mid, all_points.end());
        cout << "Loaded " << all_points.size() << " points. Split into A (" << A.size() << ") and B (" << B.size() << ")." << endl;
    } else {
        cerr << "Error: Must specify --random or --file" << endl;
        return 1;
    }

    runBenchmark("taha", A, B, selected_algo);
    runBenchmark("k3", A, B, selected_algo);
    runBenchmark("ck3", A, B, selected_algo);
    runBenchmark("ryu", A, B, selected_algo);

    return 0;
}
