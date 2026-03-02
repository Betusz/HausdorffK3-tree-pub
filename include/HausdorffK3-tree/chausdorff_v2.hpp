#ifndef CHAUSDORFF_V2_HPP
#define CHAUSDORFF_V2_HPP

#include <vector>
#include "Point.hpp"
#include "ck3tree_v2.hpp"

// Calculates Directed Hausdorff distance using the optimized ck3-tree (v2)
double directed_hausdorff_compact_v2(const k3tree::CK3TreeV2& treeA, const k3tree::CK3TreeV2& treeB);

// Symmetric Hausdorff distance using optimized ck3-tree (v2)
double hausdorff_distance_compact_v2(const std::vector<Point<3>>& setA, const std::vector<Point<3>>& setB);

#endif // CHAUSDORFF_V2_HPP