#include <src/server.hpp>

int main() {
    LogLevel level = LogLevel::INFO;
    auto logFormatter = std::make_shared<LogFormatter>(level);
    auto logAppender = std::make_shared<TerminalLogAppender>(logFormatter, level);
    Logger logger(level, logAppender);

    Server server("127.0.0.1", 8080, logger);
    server.setup();
}
