cmake_minimum_required(VERSION 3.14)
project(EchoServer VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")

# ============================================
# DEPENDENCIES
# ============================================
find_package(Boost REQUIRED COMPONENTS system)

# ============================================
# STATIC LIBRARY (shared logic)
# ============================================
set(LIBRARY_SOURCES
    src/server.cpp
    src/thread_pool.cpp
)

add_library(echo_lib STATIC ${LIBRARY_SOURCES})

target_include_directories(echo_lib PRIVATE ${Boost_INCLUDE_DIRS} src)
target_link_libraries(echo_lib ${Boost_LIBRARIES} pthread)

# ============================================
# MAIN SERVER EXECUTABLE
# ============================================
add_executable(echo_server src/main.cpp)
target_link_libraries(echo_server echo_lib)

# ============================================
# UNIT TESTS (Google Test)
# ============================================
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
FetchContent_MakeAvailable(googletest)

add_executable(run_tests tests/test_main.cpp)
target_link_libraries(run_tests 
    echo_lib
    gtest_main 
    gtest 
    ${Boost_LIBRARIES} 
    pthread
)

target_include_directories(run_tests PRIVATE src)

enable_testing()
add_test(NAME EchoServerTests COMMAND run_tests)

# ============================================
# ADDITIONAL TARGETS
# ============================================
# Convenient test runner (make check)
add_custom_target(check
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS run_tests
    COMMENT "Running unit tests..."
)

# Convenient full clean (make clean_all)
add_custom_target(clean_all
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}
    COMMENT "Removing entire build directory..."
)

# Build configuration info
message(STATUS "========================================")
message(STATUS "EchoServer Configuration:")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Boost version: ${Boost_VERSION}")
message(STATUS "========================================")
message(STATUS "  Build server: make echo_server")
message(STATUS "  Run tests: make check")
message(STATUS "  Clean all: make clean_all")
message(STATUS "========================================")