#include "HausdorffK3-tree/ck3tree_v2.hpp"
#include "../libcds/BitSequenceBuilderRG.h"
#include <queue>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <cstring>

namespace k3tree {

struct BuildNodeV2 {
    std::vector<Point<3>> points;
    HyperRectangle<3> region;
    int level;
};

CK3TreeV2::CK3TreeV2() : bits_T(nullptr), bits_B(nullptr), max_level(0) {}

CK3TreeV2::~CK3TreeV2() {
    if (bits_T) { delete bits_T; bits_T = nullptr; }
    if (bits_B) { delete bits_B; bits_B = nullptr; }
}

uint64_t CK3TreeV2::get_rank1(uint64_t bit_index) const {
    if (!bits_T || bit_index >= bits_T->getLength()) return 0;
    return bits_T->rank1(bit_index);
}

bool CK3TreeV2::is_leaf_node(uint64_t one_rank_in_T) const {
    // one_rank_in_T es 1-based (resultado de rank1)
    if (one_rank_in_T == 0) return false;
    // B tiene un bit por cada 1 en T. Accedemos con index 0-based.
    if (!bits_B || (one_rank_in_T - 1) >= bits_B->getLength()) return false;
    return bits_B->access(one_rank_in_T - 1);
}

Point<3> CK3TreeV2::get_leaf_point(uint64_t one_rank_in_T) const {
    // 1. Encontrar el índice en B (one_rank_in_T - 1)
    // 2. Calcular rank1 en B hasta ese índice. Eso nos dice cuántas hojas negras hubo antes.
    // 3. Ese valor (menos 1) es el índice en el vector leaves.
    uint64_t b_idx = one_rank_in_T - 1;
    uint64_t leaf_idx = bits_B->rank1(b_idx); 
    if (leaf_idx == 0 || leaf_idx > leaves.size()) {
         // Fallback o error (no debería pasar si la lógica es correcta)
         return Point<3>(); 
    }
    return leaves[leaf_idx - 1];
}

bool CK3TreeV2::child_exists(uint64_t node_idx, int child_octant) const {
    // Raíz virtual
    if (node_idx == std::numeric_limits<uint64_t>::max()) {
        if (!bits_T) return false;
        if (child_octant >= bits_T->getLength()) return false;
        return bits_T->access(child_octant);
    }

    // Un nodo "existe" como padre si es GRIS. Si es NEGRO, no tiene hijos en T.
    // Verificamos si el nodo actual es gris.
    uint64_t r = bits_T->rank1(node_idx);
    if (r == 0) return false;

    // Si es hoja negra, no tiene hijos (conceptualmente sus hijos no están en T)
    if (is_leaf_node(r)) return false;

    // Si es gris, calculamos dónde empiezan sus hijos
    // Necesitamos saber cuántos nodos GRISES hubo antes que este.
    // Nodos Grises anteriores = Total Nodos Anteriores (r-1) - Nodos Negros Anteriores
    
    // Nodos totales hasta aquí (exclusive): r - 1
    // Nodos negros hasta aquí (exclusive): rank1(B, r - 1)
    // Nodos grises hasta aquí (exclusive): (r - 1) - rank1(B, r - 1)
    
    // PERO cuidado: r es el rank del bit en T.
    // rank1(B, r-1) nos dice cuántos negros hay en los primeros r-1 unos.
    
    uint64_t total_ones_before = r - 1;
    uint64_t black_nodes_before = bits_B->rank1(total_ones_before); // rank en B usa índice 0-based, el ultimo indice valido es total_ones_before - 1... espera.
    // BitSequence::rank1(i) devuelve 1s en [0..i].
    // Queremos contar 1s en B indices [0 .. total_ones_before - 1].
    
    // Ajuste de índice para rank en B:
    // Si total_ones_before es 0, hay 0 negros.
    // Si total_ones_before > 0, miramos hasta el índice total_ones_before - 1.
    if (total_ones_before > 0) {
        // Corrección: libcds rank1(i) cuenta hasta i inclusive.
        // Pero bitset(i) pone el bit i.
        // bits_B tiene longitud igual al número de 1s en T.
        // Queremos saber cuantos 1s (negros) hay antes del bit que corresponde a 'node_idx'.
        // El bit correspondiente a 'node_idx' en B es el índice (r - 1).
        // Queremos rank en el rango [0, r-2].
        if (r > 1) {
             black_nodes_before = bits_B->rank1(r - 2); 
        } else {
             black_nodes_before = 0;
        }
    } else {
        black_nodes_before = 0;
    }
    
    uint64_t gray_nodes_before = total_ones_before - black_nodes_before;
    
    // Offset Raíz = 8
    // Cada nodo gris anterior genera 8 bits.
    uint64_t children_start = 8 + gray_nodes_before * 8;
    
    uint64_t child_pos = children_start + child_octant;
    if (child_pos >= bits_T->getLength()) return false;
    
    return bits_T->access(child_pos);
}

uint64_t CK3TreeV2::get_children_start_index(uint64_t node_idx) const {
    if (node_idx == std::numeric_limits<uint64_t>::max()) return 0;
    
    uint64_t r = bits_T->rank1(node_idx);
    if (r == 0) return 0;
    
    // Misma lógica que arriba para contar grises anteriores
    uint64_t total_ones_before = r - 1;
    uint64_t black_nodes_before = 0;
    if (r > 1) {
         black_nodes_before = bits_B->rank1(r - 2); 
    }
    uint64_t gray_nodes_before = total_ones_before - black_nodes_before;
    
    return 8 + gray_nodes_before * 8;
}

void CK3TreeV2::build(const std::vector<Point<3>>& points) {
    if (bits_T) { delete bits_T; bits_T = nullptr; }
    if (bits_B) { delete bits_B; bits_B = nullptr; }
    leaves.clear();
    
    if (points.empty()) return;

    // 1. Bounding Box
    Point<3> min_pt, max_pt;
    for(size_t i=0; i<3; ++i) {
        min_pt[i] = std::numeric_limits<uint64_t>::max();
        max_pt[i] = std::numeric_limits<uint64_t>::min();
    }
    for (const auto& p : points) {
        for (size_t i = 0; i < 3; ++i) {
            if (p[i] < min_pt[i]) min_pt[i] = p[i];
            if (p[i] > max_pt[i]) max_pt[i] = p[i];
        }
    }
    root_volume = HyperRectangle<3>(min_pt, max_pt);

    uint64_t max_side = 0;
    for(size_t i=0; i<3; ++i) {
        uint64_t side = max_pt[i] - min_pt[i];
        if (side > max_side) max_side = side;
    }
    max_level = 0;
    if (max_side > 0) max_level = std::ceil(std::log2(max_side + 1));
    if (max_level < 1) max_level = 1;

    // 2. BFS
    std::vector<bool> temp_bits_T;
    std::vector<bool> temp_bits_B;

    std::queue<BuildNodeV2> q;
    
    // La raíz virtual es especial.
    // Generamos sus hijos (nivel 0) manualmente para inicializar T y B.
    // La raíz NO tiene bit en T, ni en B. Solo sus hijos entran a T y B.
    
    // Simulamos que procesamos la raíz virtual para generar los primeros 8 bits de T
    // y sus correspondientes bits en B.
    
    std::array<std::vector<Point<3>>, 8> root_buckets;
    Point<3> center = root_volume.center();
    
    for (const auto& p : points) {
        int octant = 0;
        for (size_t i = 0; i < 3; ++i) {
            if (p[i] > center[i]) octant |= (1 << i);
        }
        root_buckets[octant].push_back(p);
    }
    
    for (int i = 0; i < 8; ++i) {
        if (!root_buckets[i].empty()) {
            temp_bits_T.push_back(true); // Existe
            
            // ¿Es hoja negra o gris?
            bool is_single_point = (root_buckets[i].size() == 1);
            
            // NOTA: Si estamos en max_level, también es hoja (aunque no necesariamente 1 punto,
            // pero en ck3tree comprimido asumimos que bajamos hasta aislar o max level).
            // Para simplificar, si llegamos a max_level con >1 puntos, los tratamos como "Negro" bucket?
            // El paper dice: "stop decomposition when submatrix with only one cell is found".
            // Si llegamos a max_level y hay colisión (puntos duplicados), no podemos dividir más.
            // Tratémoslo como hoja negra con el primer punto (o lista).
            // Por simplicidad de "leaves" vector (que guarda Point<3>), asumimos puntos únicos o representative.
            
            // Si es nivel máximo (level 1 es el de estos hijos), forzamos hoja.
            bool force_leaf = (1 >= max_level); 
            
            if (is_single_point || force_leaf) {
                temp_bits_B.push_back(true); // Negro
                leaves.push_back(root_buckets[i][0]);
                // NO encolamos
            } else {
                temp_bits_B.push_back(false); // Gris
                // Encolar
                Point<3> child_min = root_volume.minCorner;
                Point<3> child_max = root_volume.maxCorner;
                for (size_t d = 0; d < 3; ++d) {
                    if ((i >> d) & 1) child_min[d] = center[d] + 1;
                    else child_max[d] = center[d];
                }
                q.push({root_buckets[i], HyperRectangle<3>(child_min, child_max), 1});
            }
        } else {
            temp_bits_T.push_back(false); // No existe
            // No genera entrada en B
        }
    }
    
    // Procesar cola
    while (!q.empty()) {
        BuildNodeV2 curr = q.front();
        q.pop();

        std::array<std::vector<Point<3>>, 8> buckets;
        Point<3> c_center = curr.region.center();
        
        // Repartir puntos
        for (const auto& p : curr.points) {
            int octant = 0;
            for (size_t i = 0; i < 3; ++i) {
                if (p[i] > c_center[i]) octant |= (1 << i);
            }
            buckets[octant].push_back(p);
        }
        
        int next_level = curr.level + 1;
        
        for (int i = 0; i < 8; ++i) {
            if (!buckets[i].empty()) {
                temp_bits_T.push_back(true);
                
                bool is_single = (buckets[i].size() == 1);
                bool at_max = (next_level >= max_level);
                
                if (is_single || at_max) {
                    temp_bits_B.push_back(true); // Negro
                    leaves.push_back(buckets[i][0]);
                } else {
                    temp_bits_B.push_back(false); // Gris
                    
                    Point<3> child_min = curr.region.minCorner;
                    Point<3> child_max = curr.region.maxCorner;
                    for (size_t d = 0; d < 3; ++d) {
                        if ((i >> d) & 1) child_min[d] = c_center[d] + 1;
                        else child_max[d] = c_center[d];
                    }
                    q.push({buckets[i], HyperRectangle<3>(child_min, child_max), next_level});
                }
            } else {
                temp_bits_T.push_back(false);
            }
        }
    }
    
    // Empaquetar T
    if (!temp_bits_T.empty()) {
        size_t len = temp_bits_T.size();
        size_t uint_count = (len + 31) / 32;
        uint* raw = new uint[uint_count];
        std::memset(raw, 0, uint_count * sizeof(uint));
        for(size_t i=0; i<len; ++i) if(temp_bits_T[i]) cds_utils::bitset(raw, i);
        cds_static::BitSequenceBuilderRG builder(20);
        bits_T = builder.build(raw, len);
        delete[] raw;
    }
    
    // Empaquetar B
    if (!temp_bits_B.empty()) {
        size_t len = temp_bits_B.size();
        size_t uint_count = (len + 31) / 32;
        uint* raw = new uint[uint_count];
        std::memset(raw, 0, uint_count * sizeof(uint));
        for(size_t i=0; i<len; ++i) if(temp_bits_B[i]) cds_utils::bitset(raw, i);
        cds_static::BitSequenceBuilderRG builder(20);
        bits_B = builder.build(raw, len);
        delete[] raw;
    }
}

} // namespace k3tree