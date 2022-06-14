//
// Created by 戴佳 on 2022/5/27.
//

#ifndef WEBSERVER_HTTP_HANDLER_HPP
#define WEBSERVER_HTTP_HANDLER_HPP

#include <cstring>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

class HttpHeaders {
public:
    void put(const std::string &header, const std::string &attribute) {
        headers_[header] = attribute;
    }

    /**
     * 根据头字段的名字获得值
     *
     * @param header 头字段的名字
     * @return 头字段的值（以字符串的形式返回，可能是多值的）
     */
    std::string get(const std::string &header) {
        return headers_[header];
    }

    /**
     * 获取所有存在的头字段的名字
     *
     * @return
     */
    std::vector<std::string> keys() const {
        std::vector<std::string> keys_(headers_.size());
        std::transform(headers_.begin(), headers_.end(), keys_.begin(), [](auto pair) { return pair.first; });
        return keys_;
    }

    /**
     * 返回不包含任何头字段的 HttpHeaders
     *
     * @return 不包含任何头字段的 HttpHeaders
     */
    static HttpHeaders empty() {
        return {};
    }

private:
    std::unordered_map<std::string, std::string> headers_;
};

class HttpRequest {
public:
    std::string method;
    std::string url;
    std::string version;
    HttpHeaders headers;
    std::string body;
};

class HttpResponse {
public:
    HttpResponse(std::string version, std::string statusCode, std::string statusMessage,
                 HttpHeaders headers, std::string body) : version(std::move(version)),
                                                          statusCode(std::move(statusCode)),
                                                          statusMessage(std::move(statusMessage)),
                                                          headers(std::move(headers)),
                                                          body(std::move(body)) {}

    std::string version;
    std::string statusCode;
    std::string statusMessage;
    HttpHeaders headers;
    std::string body;
};

class HttpHandler {
public:
    /**
     * 将字节数组解析成 HttpRequest 实例对象
     * @param bytes 通过 Socket 传来的字节数组
     * @return HttpRequest 实例对象
     */
    static HttpRequest resolveRequest(std::string &&bytes) {
        HttpRequest request;

        size_t index = 0;

        // Method
        while (bytes[index] != ' ')
            ++index;
        size_t lenOfMethod = index;
        request.method = std::move(bytes.substr(0, lenOfMethod));
        ++index;

        // Url
        while (bytes[index] != ' ')
            ++index;
        size_t lenOfUrl = index - lenOfMethod - 1;
        request.url = std::move(bytes.substr(lenOfMethod + 1, lenOfUrl));
        ++index;

        // Version
        while (!(bytes[index] == '\r' && bytes[index + 1] == '\n'))
            ++index;
        size_t lenOfVersion = index - lenOfUrl - lenOfMethod - 2;
        request.version = std::move(bytes.substr(lenOfMethod + lenOfUrl + 2, lenOfVersion));
        index += 2;

        // Headers
        while (!(bytes[index] == '\r' && bytes[index + 1] == '\n')) {
            size_t begin = index, split = begin;
            while (!(bytes[index] == '\r' && bytes[index + 1] == '\n')) {
                if (bytes[index] == ':' && split == begin)
                    split = index;
                ++index;
            }
            request.headers.put(bytes.substr(begin, split - begin), bytes.substr(split + 2, index - split - 2));
            index += 2;
        }
        index += 2;

        // Body
        request.body = std::move(bytes.substr(index));

        return std::move(request);
    }

    /**
     * 将 HttpResponse 实例对象序列化为字节数组
     *
     * @param response HttpResponse 实例对象
     * @return 字节数组
     */
    static std::string serializeResponse(HttpResponse &&response) {
        std::string result = response.version + SPACE + response.statusCode + SPACE + response.statusMessage + CRLF;
        for (const auto &key: response.headers.keys()) {
            result += key + ":" + SPACE + response.headers.get(key) + CRLF;
        }
        return std::move(result + CRLF + response.body);
    }

private:
    static constexpr const char *SPACE = " ";
    static constexpr const char *CRLF = "\r\n";
};

#endif //WEBSERVER_HTTP_HANDLER_HPP
