#include <gtest/gtest.h>
#include <HausdorffK3-tree/Point.hpp>
#include <set>

// Test fixture for Point tests
class ExhaustivePointTest : public ::testing::Test {
protected:
    Point<3> p_zero;
    Point<3> p1{{1, 2, 3}};
    Point<3> p2{{4, 5, 6}};
    Point<3> p1_copy{{1, 2, 3}};
};

// --- CONSTRUCTORS and ASSIGNMENT ---
TEST_F(ExhaustivePointTest, DefaultConstructor) {
    for (size_t i = 0; i < 3; ++i) {
        ASSERT_EQ(p_zero[i], 0);
    }
}

TEST_F(ExhaustivePointTest, ArrayConstructor) {
    ASSERT_EQ(p1[0], 1);
    ASSERT_EQ(p1[1], 2);
    ASSERT_EQ(p1[2], 3);
}

TEST_F(ExhaustivePointTest, CopyConstructor) {
    Point<3> p_copy(p1);
    ASSERT_EQ(p_copy, p1);
}

TEST_F(ExhaustivePointTest, AssignmentOperator) {
    Point<3> p_assigned;
    p_assigned = p2;
    ASSERT_EQ(p_assigned, p2);
}

// --- ACCESSORS ---
TEST_F(ExhaustivePointTest, IndexOperator) {
    ASSERT_EQ(p1[0], 1);
    p1[0] = 10;
    ASSERT_EQ(p1[0], 10);
}

TEST_F(ExhaustivePointTest, IndexOperatorConst) {
    const Point<3> p_const{{10, 20, 30}};
    ASSERT_EQ(p_const[1], 20);
}

TEST_F(ExhaustivePointTest, IndexOperatorOutOfRange) {
    ASSERT_THROW(p1[3], std::out_of_range);
    const Point<3> p_const{{1,2,3}};
    ASSERT_THROW(p_const[3], std::out_of_range);
}

// --- EQUALITY ---
TEST_F(ExhaustivePointTest, EqualityOperators) {
    ASSERT_TRUE(p1 == p1_copy);
    ASSERT_FALSE(p1 == p2);
    ASSERT_TRUE(p1 != p2);
    ASSERT_FALSE(p1 != p1_copy);
}

// --- DISTANCE ---
TEST_F(ExhaustivePointTest, Distance) {
    ASSERT_EQ(p1.distSq(p1_copy), 0);
    ASSERT_NEAR(p1.distanciaA(p1_copy), 0.0, 1e-9);

    // dist(p1, p2) = sqrt((4-1)^2 + (5-2)^2 + (6-3)^2) = sqrt(9+9+9) = sqrt(27)
    ASSERT_EQ(p1.distSq(p2), 27);
    ASSERT_NEAR(p1.distanciaA(p2), std::sqrt(27.0), 1e-9);
}

// --- MORTON CODE ---
TEST_F(ExhaustivePointTest, MortonCodeConversion) {
    ASSERT_EQ(Point<3>::fromMorton(p1.toMorton()), p1);
    ASSERT_EQ(Point<3>::fromMorton(p2.toMorton()), p2);
    ASSERT_EQ(Point<3>::fromMorton(p_zero.toMorton()), p_zero);
}

TEST_F(ExhaustivePointTest, MortonCodeUniqueness) {
    ASSERT_NE(p1.toMorton(), p2.toMorton());
    Point<3> p_alt{{2, 1, 3}};
    ASSERT_NE(p1.toMorton(), p_alt.toMorton());
}

// --- COMPARISON OPERATORS ---
TEST_F(ExhaustivePointTest, ComparisonOperators) {
    Point<3> a{{1, 2, 3}};
    Point<3> b{{10, 20, 30}};

    // The comparison operators are based on the Morton codes of the points.
    // Let's verify this explicitly.
    uint64_t morton_a = a.toMorton();
    uint64_t morton_b = b.toMorton();

    ASSERT_LT(morton_a, morton_b); // Using Google Test's own macros for clarity
    ASSERT_GT(morton_b, morton_a);

    // Now, test the operators on the points themselves
    ASSERT_TRUE(a < b);
    ASSERT_TRUE(b > a);
    ASSERT_TRUE(a <= b);
    ASSERT_TRUE(b >= a);
    ASSERT_TRUE(a <= a);
    ASSERT_TRUE(a >= a);
    ASSERT_FALSE(a > b);
    ASSERT_FALSE(b < a);
}

TEST_F(ExhaustivePointTest, SetInsertion) {
    std::set<Point<3>> point_set;
    point_set.insert(p1);
    point_set.insert(p2);
    point_set.insert(p1_copy); // Should not be inserted again

    ASSERT_EQ(point_set.size(), 2);
    ASSERT_NE(point_set.find(p1), point_set.end());
    ASSERT_NE(point_set.find(p2), point_set.end());
}
