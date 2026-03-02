#include "gtest/gtest.h"
#include "HausdorffK3-tree/Point.hpp"
#include <array>
#include <stdexcept>

// Test fixture for Point<D> to easily create points of a specific dimension
template <std::size_t D>
struct PointTestFixture : public ::testing::Test {
    using TestPoint = Point<D>;
};

// Define test suites for different dimensions
using PointTest2D = PointTestFixture<2>;
using PointTest3D = PointTestFixture<3>;
using PointTest5D = PointTestFixture<5>;

// --- Test Default Constructor ---
TEST_F(PointTest2D, DefaultConstructorInitializesToZero) {
    TestPoint p;
    for (std::size_t i = 0; i < 2; ++i) {
        ASSERT_EQ(p[i], 0ULL);
    }
}

TEST_F(PointTest3D, DefaultConstructorInitializesToZero) {
    TestPoint p;
    for (std::size_t i = 0; i < 3; ++i) {
        ASSERT_EQ(p[i], 0ULL);
    }
}

// --- Test Array Constructor ---
TEST_F(PointTest2D, ArrayConstructorInitializesCorrectly) {
    std::array<uint64_t, 2> values = {10ULL, 20ULL};
    TestPoint p(values);
    ASSERT_EQ(p[0], 10ULL);
    ASSERT_EQ(p[1], 20ULL);
}

TEST_F(PointTest3D, ArrayConstructorInitializesCorrectly) {
    std::array<uint64_t, 3> values = {1ULL, 2ULL, 3ULL};
    TestPoint p(values);
    ASSERT_EQ(p[0], 1ULL);
    ASSERT_EQ(p[1], 2ULL);
    ASSERT_EQ(p[2], 3ULL);
}

// --- Test Copy Constructor ---
TEST_F(PointTest2D, CopyConstructorCreatesEqualPoint) {
    std::array<uint64_t, 2> values = {10ULL, 20ULL};
    TestPoint original(values);
    TestPoint copy = original; // Uses copy constructor
    ASSERT_EQ(original, copy);
    ASSERT_NE(&original, &copy); // Ensure it's a deep copy
}

// --- Test Assignment Operator ---
TEST_F(PointTest3D, AssignmentOperatorCreatesEqualPoint) {
    std::array<uint64_t, 3> values1 = {1ULL, 2ULL, 3ULL};
    std::array<uint64_t, 3> values2 = {4ULL, 5ULL, 6ULL};
    TestPoint p1(values1);
    TestPoint p2(values2);
    p1 = p2; // Uses assignment operator
    ASSERT_EQ(p1, p2);
    ASSERT_NE(&p1, &p2); // Ensure it's a deep copy
}

TEST_F(PointTest2D, AssignmentOperatorSelfAssignment) {
    std::array<uint64_t, 2> values = {10ULL, 20ULL};
    TestPoint p(values);
    p = p; // Self-assignment
    ASSERT_EQ(p[0], 10ULL);
    ASSERT_EQ(p[1], 20ULL);
}

// --- Test operator[] (Access to Coordinates) ---
TEST_F(PointTest2D, IndexOperatorReadAccess) {
    std::array<uint64_t, 2> values = {100ULL, 200ULL};
    const TestPoint p(values);
    ASSERT_EQ(p[0], 100ULL);
    ASSERT_EQ(p[1], 200ULL);
}

TEST_F(PointTest3D, IndexOperatorWriteAccess) {
    TestPoint p;
    p[0] = 5ULL;
    p[1] = 10ULL;
    p[2] = 15ULL;
    ASSERT_EQ(p[0], 5ULL);
    ASSERT_EQ(p[1], 10ULL);
    ASSERT_EQ(p[2], 15ULL);
}

TEST_F(PointTest5D, IndexOperatorThrowsOutOfRangeForReadAccess) {
    const TestPoint p;
    ASSERT_THROW(p[5], std::out_of_range); // D is 5, index 5 is out of range
}

TEST_F(PointTest5D, IndexOperatorThrowsOutOfRangeForWriteAccess) {
    TestPoint p;
    ASSERT_THROW(p[5] = 10ULL, std::out_of_range); // D is 5, index 5 is out of range
}

// --- Test Equality Operators (== and !=) ---
TEST_F(PointTest2D, EqualityOperatorReturnsTrueForIdenticalPoints) {
    std::array<uint64_t, 2> values = {1ULL, 2ULL};
    TestPoint p1(values);
    TestPoint p2(values);
    ASSERT_TRUE(p1 == p2);
}

TEST_F(PointTest2D, EqualityOperatorReturnsFalseForDifferentPoints) {
    std::array<uint64_t, 2> values1 = {1ULL, 2ULL};
    std::array<uint64_t, 2> values2 = {1ULL, 3ULL};
    TestPoint p1(values1);
    TestPoint p2(values2);
    ASSERT_FALSE(p1 == p2);
}

TEST_F(PointTest3D, InequalityOperatorReturnsTrueForDifferentPoints) {
    std::array<uint64_t, 3> values1 = {1ULL, 2ULL, 3ULL};
    std::array<uint64_t, 3> values2 = {1ULL, 2ULL, 4ULL};
    TestPoint p1(values1);
    TestPoint p2(values2);
    ASSERT_TRUE(p1 != p2);
}

TEST_F(PointTest3D, InequalityOperatorReturnsFalseForIdenticalPoints) {
    std::array<uint64_t, 3> values = {1ULL, 2ULL, 3ULL};
    TestPoint p1(values);
    TestPoint p2(values);
    ASSERT_FALSE(p1 != p2);
}
