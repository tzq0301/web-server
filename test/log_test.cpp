#include <gtest/gtest.h>

#include <src/log.hpp>

TEST(MainTest, BasicAssertions) {
    LogLevel level = LogLevel::INFO;
    Logger log(level, std::make_shared<TerminalLogAppender>(std::make_shared<LogFormatter>(level), level));
    log.debug("Hello world!");
    log.info("Hello world!");
    log.warning("Hello world!");
    log.error("Hello world!");
    log.fatal("Hello world!");
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
