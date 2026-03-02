#ifndef CK3TREE_V2_HPP
#define CK3TREE_V2_HPP

#include <vector>
#include <cstdint>
#include "Point.hpp"
#include "HyperRectangle.hpp"
#include "../libcds/BitSequence.h"

namespace k3tree {

class CK3TreeV2 {
public:
    CK3TreeV2();
    ~CK3TreeV2();

    void build(const std::vector<Point<3>>& points);

    // Métodos de acceso
    bool is_leaf_node(uint64_t one_rank_in_T) const; // Verifica si un nodo (identificado por su rank en T) es hoja negra
    Point<3> get_leaf_point(uint64_t one_rank_in_T) const; // Obtiene el punto de una hoja negra
    
    // Navegación
    // Devuelve true si el hijo existe.
    // Además, puede informar si ese hijo es una hoja final o un nodo interno
    bool child_exists(uint64_t node_idx, int child_octant) const;

    // Obtiene el índice en T donde empiezan los bloques de hijos del nodo actual
    // PRECONDICIÓN: El nodo debe ser GRIS (interno)
    uint64_t get_children_start_index(uint64_t node_idx) const;

    // Obtiene el rank1 de un índice en T. Útil para pasar a las funciones is_leaf_node
    uint64_t get_rank1(uint64_t bit_index) const;

    // Root info
    HyperRectangle<3> get_root_volume() const { return root_volume; }
    int get_max_level() const { return max_level; }

private:
    cds_static::BitSequence *bits_T; // Topología: 1 si existe hijo, 0 si no
    cds_static::BitSequence *bits_B; // Tipo: 1 si es Hoja Negra (1 punto), 0 si es Gris (Interno)
    
    std::vector<Point<3>> leaves;    // Puntos almacenados en las hojas negras
    
    HyperRectangle<3> root_volume;
    int max_level;
};

} // namespace k3tree

#endif // CK3TREE_V2_HPP