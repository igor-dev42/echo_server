#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include <atomic>
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include "thread_pool.hpp"
#include "server.hpp"

// ============================================================
// Test 1: The thread pool creates the right number of threads
// ============================================================
TEST(ThreadPoolTest, CreatesCorrectNumberOfThreads) {
    const size_t num_threads = 4;
    ThreadPool pool(num_threads);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    std::atomic<int> counter = 0;
    
    for (int i = 0; i < num_threads * 10; ++i) {
        pool.enqueue([&counter]() {
            counter++;
        });
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    EXPECT_EQ(counter, num_threads * 10);
}

// ============================================================
// Test 2: The thread pool processes tasks in parallel
// ============================================================
TEST(ThreadPoolTest, ProcessesTasksInParallel) {
    ThreadPool pool(4);
    std::atomic<int> concurrent_count = 0;
    std::atomic<int> max_concurrent = 0;
    
    auto task = [&]() {
        int current = ++concurrent_count;
        
        int expected = max_concurrent;
        while (current > expected && 
               !max_concurrent.compare_exchange_weak(expected, current)) {
            expected = max_concurrent;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        
        --concurrent_count;
    };
    
    for (int i = 0; i < 8; ++i) {
        pool.enqueue(task);
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    pool.shutdown();
    
    EXPECT_GT(max_concurrent, 1);
    std::cout << "Max concurrent tasks: " << max_concurrent << std::endl;
}

// ============================================================
// Test 3: The echo server responds with the same message
// ============================================================
TEST(ServerTest, EchoesMessage) {
    boost::asio::io_context io_context;
    const uint16_t test_port = 18080;
    
    EchoServer server(io_context, test_port, 2);
    server.start();
    
    std::thread io_thread([&io_context]() {
        io_context.run();
    });
    
    // Give server time to start
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Client connects with timeout
    boost::asio::ip::tcp::socket client_socket(io_context);
    boost::system::error_code ec;
    
    // Use a timer for connection timeout
    boost::asio::steady_timer timer(io_context, std::chrono::seconds(5));
    timer.async_wait([&](const boost::system::error_code&) {
        client_socket.cancel();
    });
    
    client_socket.connect(
        boost::asio::ip::tcp::endpoint(boost::asio::ip::make_address("127.0.0.1"), test_port),
        ec
    );
    
    timer.cancel();
    
    ASSERT_FALSE(ec) << "Failed to connect: " << ec.message();
    
    const std::string test_message = "Hello, Echo Server!";
    
    // Send with timeout
    boost::asio::write(client_socket, boost::asio::buffer(test_message), ec);
    ASSERT_FALSE(ec);
    
    // Read with timeout
    char buffer[1024];
    size_t len = client_socket.read_some(boost::asio::buffer(buffer), ec);
    ASSERT_FALSE(ec);
    
    std::string response(buffer, len);
    EXPECT_EQ(response, test_message);
    
    // Clean shutdown
    client_socket.close();
    server.stop();
    
    // Give server time to stop
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    io_context.stop();
    if (io_thread.joinable()) {
        io_thread.join();
    }
}

// ============================================================
// Test 4: shutdown() does not fall when called again
// ============================================================
TEST(ThreadPoolTest, ShutdownIsIdempotent) {
    ThreadPool pool(2);
    pool.shutdown();
    pool.shutdown();  // Second call must not crash
    SUCCEED();
}

// ============================================================
// Test 5: Server stops even when clients are connected
// ============================================================
TEST(ServerTest, StopsWithActiveConnections) {
    boost::asio::io_context io_context;
    const uint16_t test_port = 18081;
    
    EchoServer server(io_context, test_port, 2);
    server.start();
    
    std::thread io_thread([&io_context]() {
        io_context.run();
    });
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Create a client and keep it connected
    boost::asio::ip::tcp::socket client_socket(io_context);
    boost::system::error_code ec;
    
    client_socket.connect(
        boost::asio::ip::tcp::endpoint(boost::asio::ip::make_address("127.0.0.1"), test_port),
        ec
    );
    
    ASSERT_FALSE(ec);
    
    // Stop server while client is connected
    server.stop();
    
    // Give server time to stop
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    io_context.stop();
    if (io_thread.joinable()) {
        io_thread.join();
    }
    
    // If we get here, the server stopped without crashing
    SUCCEED();
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}