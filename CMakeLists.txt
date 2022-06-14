cmake_minimum_required(VERSION 3.22)
project(WebServer)

set(CMAKE_CXX_STANDARD 23)

include_directories(.)

include(FetchContent)

FetchContent_Declare(googletest URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

FetchContent_Declare(fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt
        USES_TERMINAL_DOWNLOAD TRUE)
FetchContent_MakeAvailable(fmt)

find_package(Threads REQUIRED) # for pthread

add_executable(WebServer src/main.cpp src/thread_pool.hpp src/http_handler.hpp src/server.hpp src/log.hpp)
target_link_libraries(WebServer fmt::fmt)

############################################################################
# GTEST >>>
############################################################################
enable_testing()

add_executable(main_test test/main_test.cpp)
target_link_libraries(main_test gtest_main)

add_executable(thread_pool_test test/thread_pool_test.cpp)
target_link_libraries(thread_pool_test gtest_main)

add_executable(http_handler_test test/http_handler_test.cpp)
target_link_libraries(http_handler_test gtest_main)

add_executable(log_test test/log_test.cpp)
target_link_libraries(log_test gtest_main fmt::fmt)

add_executable(file_test test/file_util_test.cpp)
target_link_libraries(file_test gtest_main)

include(GoogleTest)

gtest_discover_tests(main_test thread_pool_test http_handler_test log_test file_test)

############################################################################
# <<< GTEST
############################################################################
