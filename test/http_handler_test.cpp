//
// Created by 戴佳 on 2022/5/27.
//

#include <gtest/gtest.h>

#include <src/http_handler.hpp>

TEST(HttpHandlerResolveRequest, BasicAssertions) {
    HttpRequest request = HttpHandler::resolveRequest("GET /get HTTP/1.1\r\nHost: localhost:8080\r\n\r\nI am body");
    ASSERT_EQ("GET", request.method);
    ASSERT_EQ("/get", request.url);
    ASSERT_EQ("HTTP/1.1", request.version);
    ASSERT_EQ("localhost:8080", request.headers.get("Host"));
    ASSERT_EQ("I am body", request.body);
}

TEST(HttpHandlerSerializeResponseTest, BasicAssertions) {
    HttpHeaders headers;
    headers.put("Host", "localhost:8080");
    headers.put("User-Agent", "Mozilla/5.0");
    HttpResponse response("HTTP/1.1", "403", "Forbidden", headers, "I am body");

    ASSERT_EQ("HTTP/1.1 403 Forbidden\r\nUser-Agent: Mozilla/5.0\r\nHost: localhost:8080\r\n\r\nI am body",
              HttpHandler::serializeResponse(std::move(response)));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
