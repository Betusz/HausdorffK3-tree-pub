#include "HausdorffK3-tree/ryu_kamata_3d.hpp"
#include <vector>
#include <limits>
#include <algorithm>
#include <cmath>
#include <random>
#include <iostream>

using namespace std;

// Función auxiliar para distancia euclidiana 3D
double distanceKm3D(const Point<3>& p1, const Point<3>& p2) {
    return std::sqrt(p1.distSq(p2));
}

// Generador aleatorio simple
int generarNumeroAleatorio(int min, int max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(min, max);
    return distrib(gen);
}

void tmdc(const std::vector<Point<3>> &A, const std::vector<Point<3>> &B, vector<double> &U, vector<double> &V, double &cmax, int lambda) {
    if (lambda <= 0) lambda = 1;
    if (B.empty() || A.empty()) return;

    int delta = max(1, (int)(B.size() / static_cast<double>(lambda))); 
    int osp = generarNumeroAleatorio(0, delta - 1); 
    int isp = A.size() / 2;
    double dist = 0.0;
    
    if (osp >= B.size()) osp = B.size() - 1;

    for (int x = osp; x < B.size(); x += delta) {
        double cmin = numeric_limits<double>::infinity();
        double dist1 = numeric_limits<double>::infinity();
        double dist2 = numeric_limits<double>::infinity();

        for (int y = isp - 1, z = isp; y >= 0 || z < A.size(); y--, z++) {
            bool updated = false;
            if (y >= 0) {
                dist1 = distanceKm3D(B[x], A[y]);
                if (dist1 < U[y]) {
                    U[y] = dist1;
                }
                updated = true;
            }

            if (z < A.size()) {
                dist2 = distanceKm3D(B[x], A[z]);
                if (dist2 < U[z]) {
                    U[z] = dist2;
                }
                updated = true;
            }
            
            if (!updated) break;

            double current_min = numeric_limits<double>::infinity();
            if (y >= 0) current_min = min(current_min, dist1);
            if (z < A.size()) current_min = min(current_min, dist2);

            dist = current_min;

            if (dist < V[x]) {
                V[x] = dist;
            }
            cmin = min(dist, cmin);
            
            if (cmin <= cmax) {
                 if (y >= 0 && z < A.size()) {
                    isp = dist1 < dist2 ? y : z;
                } else if (y >= 0) {
                    isp = y;
                } else {
                    isp = z;
                }
                break;
            }
        }

        if (cmin > cmax && cmin != numeric_limits<double>::infinity()) {
            cmax = cmin;
        }
    }
}

std::vector<Point<3>> rulingOut(const std::vector<Point<3>> &S, const std::vector<double> &W, double cmax) {
    std::vector<Point<3>> Sro;
    Sro.reserve(S.size());
    for (size_t i = 0; i < S.size(); i++) {
        // Mantenemos el punto si su distancia al vecino más cercano conocido hasta ahora (W[i])
        // es MAYOR que cmax. Si es menor o igual, este punto no puede ser el que defina
        // la distancia de Hausdorff (que es un maximo), porque ya sabemos que está "cerca" de alguien.
        // Espera, Hausdorff es max(min). Queremos encontrar el punto con la distancia minima MAS GRANDE.
        // Si W[i] (upper bound de min_dist) es ya menor que el maximo encontrado (cmax),
        // entonces el min_dist real será aun menor, asi que no supera cmax. SE DESCARTA.
        // Correcto: Si W[i] > cmax, aun tiene "potencial" de ser el maximo.
        if (W[i] > cmax) {
            Sro.push_back(S[i]);
        }
    }
    return Sro;
}

pair<double, vector<double>> directedHD(const vector<Point<3>> &A, const vector<Point<3>> &B, vector<double> &U, vector<double> &V) {
    double cmax = 0; // Se reinicia cmax para el barrido exacto?
    // En el paper, el paso TMDC da un cmax inicial (lower bound de h(A,B)).
    // Deberíamos usar ese cmax?
    // El codigo original usa cmax = 0 aqui.
    // Si usamos cmax=0, rulingOut(A, U, 0) descarta solo los que tienen U[i]=0 (coincidencia exacta).
    // Si TMDC encontró un cmax > 0, deberíamos usarlo para descartar más.
    // Voy a calcular el cmax de U antes de filtrar.
    
    for(double val : U) {
        if (val != numeric_limits<double>::infinity() && val > cmax) {
             // Esto no es correcto. U es upper bound de min_dist.
             // cmax debe ser un LOWER BOUND de la distancia de Hausdorff.
             // En TMDC, calculamos cmax como el maximo de los minimos encontrados para la muestra de B.
             // Ese cmax es un lower bound valido de h(B_sample, A).
             // Pero h(A, B) es distinto.
        }
    }
    // Siguiendo el código original cmax=0.
    
    vector<Point<3>> Aro = rulingOut(A, U, cmax); 
    int isp = B.size() / 2;

    for (int x = 0; x < Aro.size(); x++) {
        double cmin = numeric_limits<double>::infinity();
        double dist1 = numeric_limits<double>::infinity();
        double dist2 = numeric_limits<double>::infinity();

        for (int y = isp - 1, z = isp; y >= 0 || z < B.size(); y--, z++) {
            bool updated = false;
            if (y >= 0) {
                dist1 = distanceKm3D(Aro[x], B[y]);
                if (dist1 < V[y]) {
                    V[y] = dist1;
                }
                updated = true;
            }
            if (z < B.size()) {
                dist2 = distanceKm3D(Aro[x], B[z]);
                if (dist2 < V[z]) {
                    V[z] = dist2;
                }
                updated = true;
            }
            if (!updated) break;
            
            double current_dist = numeric_limits<double>::infinity();
            if (y >= 0) current_dist = min(current_dist, dist1);
            if (z < B.size()) current_dist = min(current_dist, dist2);

            cmin = min(current_dist, cmin);
            
            if (cmin <= cmax) {
                 if (y >= 0 && z < B.size()) {
                    isp = dist1 < dist2 ? y : z;
                } else if (y >= 0) {
                    isp = y;
                } else {
                    isp = z;
                }
                break;
            }
        }

        if (cmin > cmax && cmin != numeric_limits<double>::infinity()) {
            cmax = cmin;
        }
    }
    return {cmax, V};
}

double hausKamata3D(const vector<Point<3>> &A, const vector<Point<3>> &B, int lambda) {
    if (A.empty() || B.empty()) return 0.0; // O infinito?

    double cmax = 0;
    std::vector<double> U(A.size(), numeric_limits<double>::infinity()); 
    std::vector<double> V(B.size(), numeric_limits<double>::infinity()); 
    
    // El paper sugiere ordenar los puntos. El código original no tiene sort explícito aqui, 
    // pero el algoritmo asume cierta coherencia espacial para que 'isp' funcione bien.
    // Si los puntos son aleatorios, la heurística de isp vecino es inútil.
    // ¿Deberíamos ordenar A y B por curva Z o coordenadas?
    // Point<3> ya tiene operator< basado en Morton (Z-order).
    // Asumiremos que se pasan ordenados o los ordenamos.
    // Para no modificar los vectores const originales, hacemos copia?
    // El código original recibía const vector& y no ordenaba.
    // Si los datos no están ordenados, el rendimiento caerá a O(N^2) posiblemente.
    // Voy a hacer copias ordenadas para garantizar rendimiento, aunque cueste memoria.
    
    vector<Point<3>> sortedA = A;
    vector<Point<3>> sortedB = B;
    sort(sortedA.begin(), sortedA.end());
    sort(sortedB.begin(), sortedB.end());

    tmdc(sortedA, sortedB, U, V, cmax, lambda);

    auto A_B = directedHD(sortedA, sortedB, U, V);
    auto B_A = directedHD(sortedB, sortedA, A_B.second, U);

    return max(A_B.first, B_A.first);
}
