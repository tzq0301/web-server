//
// Created by 戴佳 on 2022/5/27.
//

#include <gtest/gtest.h>

#include <src/thread_pool.hpp>

int add(int a, int b) {
    return a + b;
}

TEST(ThreadPoolTest, BasicAssertions) {
    ASSERT_EQ(5, getThreadPool().submit(add, 2, 3).get());
//    getThreadPool().shutdown();
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

