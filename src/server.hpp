//
// Created by 戴佳 on 2022/5/27.
//

#ifndef WEBSERVER_SERVER_HPP
#define WEBSERVER_SERVER_HPP

#include <arpa/inet.h>
#include <atomic>
#include <cassert>
#include <cstring>
#include <iostream>
#include <src/file_util.hpp>
#include <src/http_handler.hpp>
#include <src/log.hpp>
#include <src/thread_pool.hpp>
#include <string>
#include <sys/socket.h>
#include <unistd.h>


class Server {
public:
    Server(const std::string &address, int port, Logger logger, int backup = 5) : isShutdown(false),
                                                                                  log(std::move(logger)) {
        socketFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

        // bind：把一个地址族中的特定地址赋给 socket
        memset(&serverAddress, 0, sizeof(serverAddress));
        serverAddress.sin_family = AF_INET; // IPv4
        serverAddress.sin_addr.s_addr = inet_addr(address.c_str());
        serverAddress.sin_port = htons(port);
        int bindResult = bind(socketFd, (struct sockaddr *) &serverAddress, sizeof(serverAddress));
        if (bindResult != 0) {
            log.error(fmt::format("Fail to bind socket fd {}:{}", address, port));
            exit(1);
        }

        // listen：将 socket 变为被动类型的，等待客户的连接请求
        if (listen(socketFd, backup) == -1) {
            log.error(fmt::format("Fail to listen socket fd {}:{}", address, port));
            exit(2);
        }
    }

    ~Server() {
        isShutdown = true;
        close(socketFd);
    }

    /**
     * 启动服务器
     */
    void setup() {
        int connection;
        char clientIP[INET_ADDRSTRLEN] = "";
        struct sockaddr_in clientAddr{};
        socklen_t clientAddrLen = sizeof(clientAddr);

        log.info("Already setup and ready to accept requests.");

        while (!isShutdown) {
            connection = accept(socketFd, (struct sockaddr *) &clientAddr, &clientAddrLen);
            if (connection < 0) {
                log.warning("Fail to accept a new connection");
                continue;
            }

            inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
            log.info("Connection built: " + std::string(clientIP) + ":" +
                     std::to_string(ntohs(clientAddr.sin_port)));

            // 使用线程池进行 HTTP 解析任务的派发
            log.info("Submit to Thread Pool");
            getThreadPool().submit([this, connection]() {
                char buf[8192];
                memset(buf, 0, sizeof(buf));
                long len = recv(connection, buf, sizeof(buf), 0);
                assert(len >= 0);

                HttpRequest request = HttpHandler::resolveRequest(buf);
                log.info(fmt::format("{} request for {}", request.method, request.url));

                if (request.url == "/" || request.url == "/index") { // 首页
                    if (auto result = FileUtil::getStaticResource("index.html"); result.second) {
                        HttpResponse httpResponse("HTTP/1.1", "200", "OK", HttpHeaders::empty(), result.first);
                        std::string response = HttpHandler::serializeResponse(std::move(httpResponse));
                        send(connection, response.c_str(), response.size(), 0);
                    } else {
                        log.error("Cannot get file index.html");
                    }
                } else if (auto result = FileUtil::getStaticResource(request.url.substr(1)); result.second) {
                    HttpResponse httpResponse("HTTP/1.1", "200", "OK", HttpHeaders::empty(), result.first);
                    std::string response = HttpHandler::serializeResponse(std::move(httpResponse));
                    send(connection, response.c_str(), response.size(), 0);
                } else if (result = FileUtil::getStaticResource("404.html"); result.second) { // 404
                    HttpResponse httpResponse("HTTP/1.1", "404", "Not Found", HttpHeaders::empty(), result.first);
                    std::string response = HttpHandler::serializeResponse(std::move(httpResponse));
                    send(connection, response.c_str(), response.size(), 0);
                } else {
                    assert(0);
                }

                close(connection);
            });
        }
    }

    bool shutdown() {
        isShutdown = true;
        return true;
    }

private:
    int socketFd;

    struct sockaddr_in serverAddress{};

    std::atomic<bool> isShutdown;

    Logger log;
};

#endif //WEBSERVER_SERVER_HPP
