#include <gtest/gtest.h>
#include <HausdorffK3-tree/HyperRectangle.hpp>

// Test fixture for HyperRectangle tests
class HyperRectangleTest : public ::testing::Test {
protected:
    using HyperRectangle3D = HyperRectangle<3>;
    using Point3D = Point<3>;

    Point3D p_zero;
    Point3D p1_min{{10, 20, 30}};
    Point3D p1_max{{20, 30, 40}};
    HyperRectangle3D hr1{p1_min, p1_max};
    HyperRectangle3D hr_zero{p_zero, p_zero};
};

// --- CONSTRUCTORS and ASSIGNMENT ---
TEST_F(HyperRectangleTest, DefaultConstructor) {
    HyperRectangle3D hr;
    ASSERT_EQ(hr.minCorner, p_zero);
    ASSERT_EQ(hr.maxCorner, p_zero);
}

TEST_F(HyperRectangleTest, ParameterizedConstructor) {
    ASSERT_EQ(hr1.minCorner, p1_min);
    ASSERT_EQ(hr1.maxCorner, p1_max);
}

TEST_F(HyperRectangleTest, CopyConstructor) {
    HyperRectangle3D hr_copy(hr1);
    ASSERT_EQ(hr_copy.minCorner, hr1.minCorner);
    ASSERT_EQ(hr_copy.maxCorner, hr1.maxCorner);
}

TEST_F(HyperRectangleTest, AssignmentOperator) {
    HyperRectangle3D hr_assigned;
    hr_assigned = hr1;
    ASSERT_EQ(hr_assigned.minCorner, hr1.minCorner);
    ASSERT_EQ(hr_assigned.maxCorner, hr1.maxCorner);
}

// --- GEOMETRIC PROPERTIES ---
TEST_F(HyperRectangleTest, Center) {
    Point3D expected_center{{15, 25, 35}};
    ASSERT_EQ(hr1.center(), expected_center);
    ASSERT_EQ(hr_zero.center(), p_zero);
}

// --- UNION ---
TEST_F(HyperRectangleTest, Union) {
    Point3D p2_min{{15, 25, 35}};
    Point3D p2_max{{25, 35, 45}};
    HyperRectangle3D hr2(p2_min, p2_max);

    HyperRectangle3D hr_union = hr1.Union(hr2);
    
    Point3D expected_min{{10, 20, 30}};
    Point3D expected_max{{25, 35, 45}};

    ASSERT_EQ(hr_union.minCorner, expected_min);
    ASSERT_EQ(hr_union.maxCorner, expected_max);
}


// --- DISTANCE to POINT ---
TEST_F(HyperRectangleTest, MinMaxDistSqToPoint) {
    // Point inside the rectangle
    Point3D p_inside{{15, 25, 35}};
    ASSERT_EQ(hr1.minDistSq(p_inside), 0);

    // Point outside the rectangle
    Point3D p_outside{{0, 0, 0}};
    // dists: (10-0)^2, (20-0)^2, (30-0)^2 -> 100, 400, 900
    ASSERT_EQ(hr1.minDistSq(p_outside), 100 + 400 + 900);

    // Point on a face
    Point3D p_on_face{{10, 25, 35}};
    ASSERT_EQ(hr1.minDistSq(p_on_face), 0);

    // Max distance from inside point
    // max(abs(15-10), abs(15-20))^2 = 5^2 = 25
    // max(abs(25-20), abs(25-30))^2 = 5^2 = 25
    // max(abs(35-30), abs(35-40))^2 = 5^2 = 25
    ASSERT_EQ(hr1.maxDistSq(p_inside), 25 + 25 + 25);

    // Max distance from outside point
    // max(abs(0-10), abs(0-20))^2 = 20^2 = 400
    // max(abs(0-20), abs(0-30))^2 = 30^2 = 900
    // max(abs(0-30), abs(0-40))^2 = 40^2 = 1600
    ASSERT_EQ(hr1.maxDistSq(p_outside), 400 + 900 + 1600);
}

// --- DISTANCE to HYPERRECTANGLE ---
TEST_F(HyperRectangleTest, MinMaxDistSqToHyperRectangle) {
    // Identical rectangle
    HyperRectangle3D hr1_copy(p1_min, p1_max);
    ASSERT_EQ(hr1.minMinDistSq(hr1_copy), 0);

    // Non-overlapping, touching rectangles
    Point3D p_touching_min{{20, 20, 30}};
    Point3D p_touching_max{{25, 25, 35}};
    HyperRectangle3D hr_touching(p_touching_min, p_touching_max);
    ASSERT_EQ(hr1.minMinDistSq(hr_touching), 0);

    // Non-overlapping, separated rectangles
    Point3D p_separated_min{{100, 100, 100}};
    Point3D p_separated_max{{110, 110, 110}};
    HyperRectangle3D hr_separated(p_separated_min, p_separated_max);
    // minCorner1={10,20,30}, maxCorner1={20,30,40}
    // minCorner2={100,100,100}, maxCorner2={110,110,110}
    // dists: (100-20)^2, (100-30)^2, (100-40)^2 -> 80^2, 70^2, 60^2
    ASSERT_EQ(hr1.minMinDistSq(hr_separated), 6400 + 4900 + 3600);

    // Overlapping rectangles
    Point3D p_overlap_min{{15, 25, 35}};
    Point3D p_overlap_max{{25, 35, 45}};
    HyperRectangle3D hr_overlap(p_overlap_min, p_overlap_max);
    ASSERT_EQ(hr1.minMinDistSq(hr_overlap), 0);

    // MaxMax distance for separated rectangles
    // max(abs(100-20), abs(110-10))^2 = max(80, 100)^2 = 10000
    // max(abs(100-30), abs(110-20))^2 = max(70, 90)^2 = 8100
    // max(abs(100-40), abs(110-30))^2 = max(60, 80)^2 = 6400
    ASSERT_EQ(hr1.maxMaxDistSq(hr_separated), 10000 + 8100 + 6400);
}

// --- FLOATING POINT DISTANCES ---
TEST_F(HyperRectangleTest, FloatingPointDistances) {
    Point3D p_outside{{0, 0, 0}};
    ASSERT_NEAR(hr1.minDist(p_outside), std::sqrt(1400.0), 1e-9);
    ASSERT_NEAR(hr1.maxDist(p_outside), std::sqrt(2900.0), 1e-9);

    Point3D p_separated_min{{100, 100, 100}};
    Point3D p_separated_max{{110, 110, 110}};
    HyperRectangle3D hr_separated(p_separated_min, p_separated_max);
    ASSERT_NEAR(hr1.minMinDist(hr_separated), std::sqrt(14900.0), 1e-9);
    ASSERT_NEAR(hr1.maxMaxDist(hr_separated), std::sqrt(24500.0), 1e-9);
}
