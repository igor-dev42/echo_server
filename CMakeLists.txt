cmake_minimum_required(VERSION 3.14)
project(EchoServer VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#Boost
find_package(Boost REQUIRED COMPONENTS system)

add_executable(echo_server 
    src/main.cpp 
    src/server.cpp 
    src/thread_pool.cpp
)

target_include_directories(echo_server PRIVATE ${Boost_INCLUDE_DIRS} src)
target_link_libraries(echo_server ${Boost_LIBRARIES} pthread)

# Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
FetchContent_MakeAvailable(googletest)

# Tests
enable_testing()
add_executable(run_tests tests/test_main.cpp)
target_link_libraries(run_tests gtest_main gtest pthread)
target_include_directories(run_tests PRIVATE src)

add_test(NAME EchoServerTests COMMAND run_tests)