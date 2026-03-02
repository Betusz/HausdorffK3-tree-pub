/**
 * @file k3tree.hpp
 * @brief Define la clase K3Tree, un árbol kd tridimensional (K=3).
 */

#ifndef K3TREE_HPP
#define K3TREE_HPP

#include <vector>
#include <memory>
#include <array>
#include "Point.hpp"
#include "HyperRectangle.hpp"

namespace k3tree {

constexpr std::size_t K = 3; // Para un árbol 3D

/**
 * @struct Node
 * @brief Representa un nodo en el Octree (K3Tree).
 *        Un nodo puede ser interno (tiene hijos) o una hoja (tiene puntos).
 */
struct Node {
    std::vector<Point<K>> points; /**< Puntos almacenados en este nodo (solo si es hoja). */
    std::array<std::unique_ptr<Node>, 8> children; /**< Hijos del nodo (octantes). */
    HyperRectangle<K> volume; /**< El hiperrectángulo que cubre esta región del espacio. */

    /**
     * @brief Verifica si el nodo es una hoja.
     * @return true si no tiene hijos inicializados.
     */
    bool is_leaf() const {
        for (const auto& child : children) {
            if (child) return false;
        }
        return true;
    }
    
    Node() = default;
};

/**
 * @class K3Tree
 * @brief Implementa un Octree 3D para particionamiento espacial geométrico.
 */
class K3Tree {
public:
    /**
     * @brief Constructor por defecto.
     */
    K3Tree() : root(nullptr) {}

    /**
     * @brief Construye el Octree a partir de un vector de puntos.
     * @param points Vector de puntos.
     */
    void build(const std::vector<Point<K>>& points);

    /**
     * @brief Accede a la raíz del árbol (útil para algoritmos externos como Hausdorff).
     */
    const std::unique_ptr<Node>& get_root() const { return root; }

private:
    std::unique_ptr<Node> root; /**< Raíz del Octree. */

    /**
     * @brief Construye recursivamente el Octree.
     * @param points Puntos contenidos en la región actual.
     * @param region Hiperrectángulo que define la región actual.
     * @param depth Profundidad actual.
     */
    std::unique_ptr<Node> build_recursive(const std::vector<Point<K>>& points, const HyperRectangle<K>& region, int depth);
};

} // namespace k3tree

#endif // K3TREE_HPP