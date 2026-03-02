#include <HausdorffK3-tree/k3tree.hpp>
#include <algorithm>
#include <limits>
#include <cmath>

namespace k3tree {

// --- Métodos Públicos ---

void K3Tree::build(const std::vector<Point<K>>& points) {
    if (points.empty()) {
        root = nullptr;
        return;
    }

    // 1. Calcular el Bounding Box global (HyperRectangle)
    Point<K> min_pt, max_pt;
    
    // Inicializar min con max posible, max con min posible
    for(size_t i=0; i<K; ++i) {
        min_pt[i] = std::numeric_limits<uint64_t>::max();
        max_pt[i] = std::numeric_limits<uint64_t>::min();
    }

    for (const auto& p : points) {
        for (size_t i = 0; i < K; ++i) {
            if (p[i] < min_pt[i]) min_pt[i] = p[i];
            if (p[i] > max_pt[i]) max_pt[i] = p[i];
        }
    }

    HyperRectangle<K> global_region(min_pt, max_pt);
    
    // 2. Iniciar construcción recursiva
    root = build_recursive(points, global_region, 0);
}

// --- Métodos Privados ---

std::unique_ptr<Node> K3Tree::build_recursive(const std::vector<Point<K>>& points, const HyperRectangle<K>& region, int depth) {
    auto node = std::make_unique<Node>();
    node->volume = region;

    // Condición de parada: Si hay pocos puntos o llegamos a un volumen unitario (punto único)
    // Para Hausdorff, tener hojas con 1 solo punto suele ser eficiente, o un pequeño bucket.
    // Aquí usaremos 1 punto como límite para partición pura, o si todos los puntos son iguales.
    bool all_same = true;
    if (!points.empty()) {
        const auto& first = points[0];
        for (size_t i = 1; i < points.size(); ++i) {
            if (points[i] != first) {
                all_same = false;
                break;
            }
        }
    }

    // Si solo hay 1 punto, o todos son iguales, o el volumen es 0 (punto), es hoja.
    if (points.size() <= 1 || all_same) {
        node->points = points;
        return node;
    }

    // Calcular el centro geométrico de la región actual
    Point<K> center;
    for (size_t i = 0; i < K; ++i) {
        // División entera
        center[i] = region.minCorner[i] + (region.maxCorner[i] - region.minCorner[i]) / 2;
    }

    // Crear buckets para los 8 hijos
    std::array<std::vector<Point<K>>, 8> buckets;

    for (const auto& p : points) {
        int octant = 0;
        for (size_t i = 0; i < K; ++i) {
            // Si la coordenada es mayor que el centro, bit = 1.
            // Nota: Usamos > center para la mitad superior. <= center para la inferior.
            if (p[i] > center[i]) {
                octant |= (1 << i); 
            }
        }
        buckets[octant].push_back(p);
    }

    // Construir hijos recursivamente
    bool is_internal = false;
    for (int i = 0; i < 8; ++i) {
        if (!buckets[i].empty()) {
            is_internal = true;
            
            // Calcular el HyperRectangle del hijo
            Point<K> child_min = region.minCorner;
            Point<K> child_max = region.maxCorner;

            for (size_t d = 0; d < K; ++d) {
                if ((i >> d) & 1) {
                    // Bit 1: Mitad superior [center + 1, max]
                    child_min[d] = center[d] + 1; 
                } else {
                    // Bit 0: Mitad inferior [min, center]
                    child_max[d] = center[d];
                }
            }
            
            // Si por alguna razón min > max (ej. colapso de enteros), ajustamos
            // (Esto no debería pasar si la lógica de parada funciona, pero por seguridad)
             bool valid = true;
             for(size_t d=0; d<K; ++d) {
                 if (child_min[d] > child_max[d]) valid = false;
             }

            if (valid) {
                HyperRectangle<K> child_region(child_min, child_max);
                node->children[i] = build_recursive(buckets[i], child_region, depth + 1);
            } else {
                 // Caso borde: puntos en el límite, agregarlos a este nodo como hoja si no se pudo dividir
                 // aunque con la lógica actual de buckets, deberían caer bien.
                 // Si bucket no vacío pero región inválida, forzamos hoja.
                 node->points.insert(node->points.end(), buckets[i].begin(), buckets[i].end());
            }
        }
    }
    
    // Si después de intentar dividir no se crearon hijos (raro, pero posible si puntos muy juntos), convertir en hoja
    if (!is_internal) {
        node->points = points;
    }

    return node;
}

} // namespace k3tree