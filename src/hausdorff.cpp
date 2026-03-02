#include "HausdorffK3-tree/hausdorff.hpp"
#include <queue>
#include <algorithm>
#include <limits>
#include <vector>
#include <cmath>

// Usamos distancias al cuadrado (uint64_t) para evitar sqrt y flotantes durante el proceso.
using DistSq = uint64_t;

/**
 * @struct NodeWrapper
 * @brief Envoltorio para nodos en las colas de prioridad.
 */
struct NodeWrapper {
    const k3tree::Node* node;
    DistSq priority; // Puede ser maxMaxDist (Upper Bound) u otra métrica

    // Comparador para MaxHeap (mayor prioridad sale primero)
    bool operator<(const NodeWrapper& other) const {
        return priority < other.priority;
    }
    
    // Comparador para MinHeap (menor prioridad sale primero)
    bool operator>(const NodeWrapper& other) const {
        return priority > other.priority;
    }
};

struct MinHeapComparator {
    bool operator()(const NodeWrapper& a, const NodeWrapper& b) const {
        return a.priority > b.priority; // Menor priority sale primero
    }
};

/**
 * @brief Calcula el "potencial" (prioridad) de un nodo de A con respecto al árbol B.
 *        Equivalente a `isCandidate` en la implementación original.
 * 
 * Busca en el árbol B el nodo que MINIMIZA la distancia máxima (maxMaxDist) hacia el nodo A.
 * Esta distancia mínima-máxima es una cota superior de la distancia de Hausdorff para 
 * cualquier punto dentro del nodo A.
 * 
 * @param nodeA El nodo del árbol A que estamos evaluando.
 * @param rootB La raíz del árbol B.
 * @param current_max_sq La mejor distancia (cuadrada) de Hausdorff encontrada hasta ahora (supermax).
 * @param valid Salida booleana. Se pone en false si el nodo es podado (su prioridad <= current_max_sq).
 * @return La prioridad calculada (distancia mínima-máxima al cuadrado).
 */
static DistSq compute_node_priority(const k3tree::Node* nodeA, const k3tree::Node* rootB, DistSq current_max_sq, bool& valid) {
    if (!rootB) {
        valid = true; 
        return std::numeric_limits<DistSq>::max();
    }

    // MinHeap para explorar B: queremos encontrar el nodo B que esté "más cerca" de A en el peor caso (min maxMaxDist)
    std::priority_queue<NodeWrapper, std::vector<NodeWrapper>, MinHeapComparator> pq;
    
    DistSq initial_dist = nodeA->volume.maxMaxDistSq(rootB->volume);
    
    // Poda temprana: si incluso la raíz de B ya "cubre" a A dentro del rango conocido
    if (initial_dist <= current_max_sq) {
        valid = false;
        return 0;
    }

    pq.push({rootB, initial_dist});

    while (!pq.empty()) {
        NodeWrapper curr = pq.top();
        pq.pop();

        // Si es hoja, hemos encontrado el camino óptimo (minimax) en este trazo
        if (curr.node->is_leaf()) {
            valid = true;
            return curr.priority;
        }

        // Expandir hijos
        for (const auto& child : curr.node->children) {
            if (child) {
                DistSq d = nodeA->volume.maxMaxDistSq(child->volume);
                
                // CRUCIAL: Si encontramos CUALQUIER nodo en B que garantice que la distancia
                // máxima desde A es menor que lo que ya tenemos, A no puede contribuir a mejorar el máximo.
                // Poda de A.
                if (d <= current_max_sq) {
                    valid = false;
                    return 0;
                }
                pq.push({child.get(), d});
            }
        }
    }
    
    valid = true; // No debería llegar aquí si B no está vacío
    return std::numeric_limits<DistSq>::max();
}

/**
 * @brief Encuentra la distancia al vecino más cercano en B para un punto dado.
 *        Actualiza `current_max` si encuentra una distancia mayor.
 */
static void update_max_with_point(const Point<3>& p, const k3tree::Node* rootB, DistSq& current_max_sq) {
    // Esta función busca el NN de p en B.
    // Usamos una búsqueda estándar en B, pero podemos usar current_max_sq para podar?
    // No para podar la búsqueda del NN (queremos el min real), 
    // pero si el min real > current_max_sq, actualizamos current_max_sq.
    
    // Algoritmo standard NN en Octree/K-d tree
    
    DistSq best_dist_sq = std::numeric_limits<DistSq>::max();
    
    // Stack para DFS/Best-First. Usamos MinHeap para visitar nodos prometedores (minMinDist)
    std::priority_queue<NodeWrapper, std::vector<NodeWrapper>, MinHeapComparator> pq;
    
    if (rootB) {
        pq.push({rootB, rootB->volume.minDistSq(p)});
    }

    while (!pq.empty()) {
        NodeWrapper curr = pq.top();
        pq.pop();

        // Si la distancia mínima al volumen es mayor que la mejor distancia encontrada, podar
        if (curr.priority >= best_dist_sq) continue;

        if (curr.node->is_leaf()) {
            for (const auto& bp : curr.node->points) {
                DistSq d = p.distSq(bp);
                if (d < best_dist_sq) {
                    best_dist_sq = d;
                }
            }
        } else {
            for (const auto& child : curr.node->children) {
                if (child) {
                    DistSq d = child->volume.minDistSq(p);
                    if (d < best_dist_sq) {
                        pq.push({child.get(), d});
                    }
                }
            }
        }
    }

    if (best_dist_sq > current_max_sq && best_dist_sq != std::numeric_limits<DistSq>::max()) {
        current_max_sq = best_dist_sq;
    }
}

/**
 * @brief Calcula la distancia de Hausdorff dirigida h(A, B)
 */
static double directed_hausdorff_maxheap(const k3tree::K3Tree& treeA, const k3tree::K3Tree& treeB) {
    const auto& rootA = treeA.get_root();
    const auto& rootB = treeB.get_root();

    if (!rootA || !rootB) return 0.0;

    DistSq supermax = 0;

    // MaxHeap de nodos de A, ordenados por su potencial (maxMaxDist a B)
    std::priority_queue<NodeWrapper> pq; // MaxHeap por defecto
    
    // Inicialización
    bool valid = false;
    DistSq prio = compute_node_priority(rootA.get(), rootB.get(), supermax, valid);
    
    if (valid) {
        pq.push({rootA.get(), prio});
    }

    while (!pq.empty()) {
        NodeWrapper curr = pq.top();
        pq.pop();

        // Si el potencial de este nodo es menor que lo que ya hemos encontrado,
        // ningún hijo podrá superarlo (la métrica maxMaxDist es monótona decreciente al bajar).
        if (curr.priority <= supermax) {
            return std::sqrt(static_cast<double>(supermax)); // Early exit global
        }

        if (curr.node->is_leaf()) {
            // Procesar puntos en la hoja
            for (const auto& p : curr.node->points) {
                update_max_with_point(p, rootB.get(), supermax);
            }
            
            // Si después de actualizar con los puntos reales, el supermax supera
            // al siguiente en la cola, podríamos optimizar, pero el loop lo maneja.
        } else {
            // Expandir hijos de A
            for (const auto& child : curr.node->children) {
                if (child) {
                    bool is_valid = false;
                    DistSq child_prio = compute_node_priority(child.get(), rootB.get(), supermax, is_valid);
                    
                    if (is_valid) {
                        pq.push({child.get(), child_prio});
                    }
                }
            }
        }
    }

    return std::sqrt(static_cast<double>(supermax));
}

double hausdorff_distance_k3(const std::vector<Point<3>>& setA, const std::vector<Point<3>>& setB) {
    if (setA.empty() || setB.empty()) return 0.0;

    k3tree::K3Tree treeA;
    treeA.build(setA);

    k3tree::K3Tree treeB;
    treeB.build(setB);

    double h_AB = directed_hausdorff_maxheap(treeA, treeB);
    double h_BA = directed_hausdorff_maxheap(treeB, treeA);

    return std::max(h_AB, h_BA);
}
