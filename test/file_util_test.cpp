#include <gtest/gtest.h>

#include <src/file_util.hpp>

//TEST(MainTest, BasicAssertions) {
//    ASSERT_EQ(1, 3 - 2);
//}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
