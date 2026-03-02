#include "HausdorffK3-tree/chausdorff_v2.hpp"
#include <queue>
#include <limits>
#include <cmath>
#include <iostream>

using DistSq = uint64_t;

struct CompactNodeStateV2 {
    uint64_t node_idx;      
    HyperRectangle<3> volume; 
    DistSq priority;        
    // int level; // Ya no es estrictamente necesario para chequear hoja, pero util para geometria

    bool operator<(const CompactNodeStateV2& other) const {
        return priority < other.priority;
    }
    bool operator>(const CompactNodeStateV2& other) const {
        return priority > other.priority;
    }
};

struct CompactMinComparatorV2 {
    bool operator()(const CompactNodeStateV2& a, const CompactNodeStateV2& b) const {
        return a.priority > b.priority;
    }
};

static HyperRectangle<3> get_child_volume(const HyperRectangle<3>& parent_vol, int octant) {
    Point<3> child_min = parent_vol.minCorner;
    Point<3> child_max = parent_vol.maxCorner;
    Point<3> center = parent_vol.center();

    for (size_t d = 0; d < 3; ++d) {
        if ((octant >> d) & 1) child_min[d] = center[d] + 1;
        else child_max[d] = center[d];
    }
    return HyperRectangle<3>(child_min, child_max);
}

// Check if a node is a leaf in the V2 tree
static bool is_leaf_v2(const k3tree::CK3TreeV2& tree, uint64_t node_idx) {
    if (node_idx == std::numeric_limits<uint64_t>::max()) return false;
    uint64_t r = tree.get_rank1(node_idx);
    return tree.is_leaf_node(r);
}

static void update_max_with_point_compact_v2(const Point<3>& p, const k3tree::CK3TreeV2& treeB, DistSq& supermax) {
    DistSq min_dist_found = std::numeric_limits<DistSq>::max();
    
    std::priority_queue<CompactNodeStateV2, std::vector<CompactNodeStateV2>, CompactMinComparatorV2> pq;
    
    HyperRectangle<3> rootB_vol = treeB.get_root_volume();
    DistSq d_root = rootB_vol.minDistSq(p);
    
    pq.push({std::numeric_limits<uint64_t>::max(), rootB_vol, d_root});

    while (!pq.empty()) {
        CompactNodeStateV2 curr = pq.top();
        pq.pop();

        if (curr.priority >= min_dist_found) continue;

        // Check Leaf
        if (is_leaf_v2(treeB, curr.node_idx)) {
            // Es una hoja negra (1 punto)
            uint64_t r = treeB.get_rank1(curr.node_idx);
            Point<3> bp = treeB.get_leaf_point(r);
            
            DistSq d = p.distSq(bp);
            if (d < min_dist_found) {
                min_dist_found = d;
                if (min_dist_found <= supermax) return;
            }
        } else {
            // Expandir
            uint64_t child_start;
            if (curr.node_idx == std::numeric_limits<uint64_t>::max()) child_start = 0;
            else child_start = treeB.get_children_start_index(curr.node_idx);

            for (int i = 0; i < 8; ++i) {
                if (treeB.child_exists(curr.node_idx, i)) {
                    // child_exists maneja la logica de si el bit en T es 1.
                    // Para la raíz virtual (MAX), sus hijos son 0..7
                    // Para nodo normal, hijos son child_start + i
                    uint64_t child_idx;
                    if (curr.node_idx == std::numeric_limits<uint64_t>::max()) child_idx = i;
                    else child_idx = child_start + i;
                    
                    HyperRectangle<3> child_vol = get_child_volume(curr.volume, i);
                    DistSq d = child_vol.minDistSq(p);
                    
                    if (d < min_dist_found) {
                        pq.push({child_idx, child_vol, d});
                    }
                }
            }
        }
    }
    
    if (min_dist_found > supermax && min_dist_found != std::numeric_limits<DistSq>::max()) {
        supermax = min_dist_found;
    }
}

static DistSq compute_compact_priority_v2(const CompactNodeStateV2& stateA, 
                                     const k3tree::CK3TreeV2& treeB, 
                                     DistSq current_max_sq, 
                                     bool& valid) {
    
    std::priority_queue<CompactNodeStateV2, std::vector<CompactNodeStateV2>, CompactMinComparatorV2> pq;

    HyperRectangle<3> rootB_vol = treeB.get_root_volume();
    DistSq initial_dist = stateA.volume.maxMaxDistSq(rootB_vol);
    
    if (initial_dist <= current_max_sq) {
        valid = false;
        return 0;
    }

    pq.push({std::numeric_limits<uint64_t>::max(), rootB_vol, initial_dist});

    while (!pq.empty()) {
        CompactNodeStateV2 currB = pq.top();
        pq.pop();

        if (is_leaf_v2(treeB, currB.node_idx)) {
            // Hoja negra en B
            // Un nodo hoja negro CUBRE el volumen, pero es un punto.
            // La distancia MaxMax entre VolumenA y PuntoB es valida.
            valid = true;
            return currB.priority;
        }

        uint64_t child_start;
        if (currB.node_idx == std::numeric_limits<uint64_t>::max()) child_start = 0;
        else child_start = treeB.get_children_start_index(currB.node_idx);

        bool expanded = false;
        for (int i = 0; i < 8; ++i) {
             // Aquí hay que tener cuidado. child_exists devuelve true si el bit es 1.
             // Pero la llamada necesita el índice correcto.
             // child_exists(MAX, i) -> ok
             // child_exists(node, i) -> ok
             if (treeB.child_exists(currB.node_idx, i)) {
                 expanded = true;
                 uint64_t child_idx;
                 if (currB.node_idx == std::numeric_limits<uint64_t>::max()) child_idx = i;
                 else child_idx = child_start + i;
                 
                 HyperRectangle<3> child_vol = get_child_volume(currB.volume, i);
                 
                 DistSq d = stateA.volume.maxMaxDistSq(child_vol);

                 if (d <= current_max_sq) {
                     valid = false;
                     return 0;
                 }
                 pq.push({child_idx, child_vol, d});
            }
        }
        
        // Si no se expandió ningún hijo (nodo vacío/muerto?), ignorar
        // (No debería pasar si is_leaf_v2 retorna false y child_exists funciona bien para nodos grises)
    }
    
    valid = true;
    return std::numeric_limits<DistSq>::max();
}


double directed_hausdorff_compact_v2(const k3tree::CK3TreeV2& treeA, const k3tree::CK3TreeV2& treeB) {
    DistSq supermax = 0;
    
    std::priority_queue<CompactNodeStateV2> pq;

    HyperRectangle<3> rootA_vol = treeA.get_root_volume();
    
    bool valid = false;
    CompactNodeStateV2 rootA_state = {std::numeric_limits<uint64_t>::max(), rootA_vol, 0};
    
    DistSq prio = compute_compact_priority_v2(rootA_state, treeB, supermax, valid);
    
    if (valid) {
        rootA_state.priority = prio;
        pq.push(rootA_state);
    }

    while (!pq.empty()) {
        CompactNodeStateV2 currA = pq.top();
        pq.pop();

        if (currA.priority <= supermax) {
            return std::sqrt(static_cast<double>(supermax));
        }

        if (is_leaf_v2(treeA, currA.node_idx)) {
            // Hoja negra en A
            uint64_t r = treeA.get_rank1(currA.node_idx);
            Point<3> p = treeA.get_leaf_point(r);
            update_max_with_point_compact_v2(p, treeB, supermax);
        } else {
            // Expandir
            uint64_t child_start;
            if (currA.node_idx == std::numeric_limits<uint64_t>::max()) child_start = 0;
            else child_start = treeA.get_children_start_index(currA.node_idx);

            for (int i = 0; i < 8; ++i) {
                if (treeA.child_exists(currA.node_idx, i)) {
                     uint64_t child_idx;
                     if (currA.node_idx == std::numeric_limits<uint64_t>::max()) child_idx = i;
                     else child_idx = child_start + i;
                     
                     HyperRectangle<3> child_vol = get_child_volume(currA.volume, i);
                     
                     bool is_valid = false;
                     CompactNodeStateV2 child_state = {child_idx, child_vol, 0};
                     
                     DistSq child_prio = compute_compact_priority_v2(child_state, treeB, supermax, is_valid);
                     
                     if (is_valid) {
                         child_state.priority = child_prio;
                         pq.push(child_state);
                     }
                }
            }
        }
    }

    return std::sqrt(static_cast<double>(supermax));
}

double hausdorff_distance_compact_v2(const std::vector<Point<3>>& setA, const std::vector<Point<3>>& setB) {
    k3tree::CK3TreeV2 treeA;
    treeA.build(setA);
    
    k3tree::CK3TreeV2 treeB;
    treeB.build(setB);

    double hAB = directed_hausdorff_compact_v2(treeA, treeB);
    double hBA = directed_hausdorff_compact_v2(treeB, treeA);

    return std::max(hAB, hBA);
}