//
// Created by 戴佳 on 2022/5/27.
//

#ifndef WEBSERVER_LOG_HPP
#define WEBSERVER_LOG_HPP

#include <cinttypes>
#include <fmt/chrono.h>
#include <fmt/core.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <memory>
#include <utility>

/**
 * 日志等级（DEBUG 最低，FATAL 最高）
 */
enum class LogLevel {
    DEBUG = 0,
    INFO = 1,
    WARNING = 2,
    ERROR = 3,
    FATAL = 4,
};

/**
 * 日志事件
 *
 * 使用 LogFormatter 将 LogEvent 格式化为「可打印」的日志信息
 */
class LogEvent {
public:
    explicit LogEvent(std::string content) : content_(std::move(content)) {}

    std::string content_;
};

/**
 * 将 LogEvent 格式化为「可打印」的日志信息
 */
class LogFormatter {
public:
    explicit LogFormatter(LogLevel level) : level_(level) {}

    std::string format(LogLevel level, std::shared_ptr<LogEvent> logEvent) {
        std::stringstream ss;

        ss << fmt::format("{:%Y-%m-%d %H:%M:%S}. [", fmt::localtime(std::time(nullptr)));
        switch (level) {
            case LogLevel::DEBUG:
                ss << "DEBUG  ";
                break;
            case LogLevel::INFO:
                ss << "INFO   ";
                break;
            case LogLevel::WARNING:
                ss << "WARNING";
                break;
            case LogLevel::ERROR:
                ss << "ERROR  ";
                break;
            case LogLevel::FATAL:
                ss << "FATAL  ";
                break;
        }
        ss << "] : " << logEvent->content_;

        return ss.str();
    }

private:
    LogLevel level_;
};

/**
 * 日志输出器
 *
 * 实现该接口完成实现
 */
class LogAppender {
public:
    LogAppender(std::shared_ptr<LogFormatter> formatter, LogLevel level) : formatter_(std::move(formatter)),
                                                                           level_(level) {}

    virtual ~LogAppender() = default;

    virtual void log(LogLevel level, std::shared_ptr<LogEvent> logEvent) = 0;

protected:
    LogLevel level_;
    std::shared_ptr<LogFormatter> formatter_;
};

/**
 * 日志终端输出器
 */
class TerminalLogAppender : public LogAppender {
public:
    TerminalLogAppender(std::shared_ptr<LogFormatter> formatter, LogLevel level)
            : LogAppender(std::move(formatter), level) {}

    void log(LogLevel level, std::shared_ptr<LogEvent> logEvent) override {
        if (level < level_) {
            return;
        }

        std::cout << formatter_->format(level, logEvent) << std::endl;
    }

    ~TerminalLogAppender() override = default;
};

/**
 * 日志文件输出器
 */
class FileLogAppender : public LogAppender {
public:
    explicit FileLogAppender(std::shared_ptr<LogFormatter> formatter, LogLevel level, const std::string &filename)
            : LogAppender(std::move(formatter), level), filename_(filename), file_(filename, std::ios::out) {}

    void log(LogLevel level, std::shared_ptr<LogEvent> logEvent) override {
        if (level < level_) {
            return;
        }

        file_ << formatter_->format(level, logEvent) << std::endl;
    }

    ~FileLogAppender() override {
        file_.close();
    }

private:
    std::string filename_;
    std::ofstream file_;
};

/**
 * 日志类
 */
class Logger {
public:
    Logger(LogLevel level, std::shared_ptr<LogAppender> appender) : level_(level), appender_(std::move(appender)) {}

    void debug(const std::string &content) {
        appender_->log(LogLevel::DEBUG, std::make_shared<LogEvent>(content));
    }

    void info(const std::string &content) {
        appender_->log(LogLevel::INFO, std::make_shared<LogEvent>(content));
    }

    void warning(const std::string &content) {
        appender_->log(LogLevel::WARNING, std::make_shared<LogEvent>(content));
    }

    void error(const std::string &content) {
        appender_->log(LogLevel::ERROR, std::make_shared<LogEvent>(content));
    }

    void fatal(const std::string &content) {
        appender_->log(LogLevel::FATAL, std::make_shared<LogEvent>(content));
    }

    LogLevel getLogLevel() const {
        return level_;
    }

private:
    LogLevel level_;
    std::shared_ptr<LogAppender> appender_;
};

#endif //WEBSERVER_LOG_HPP
