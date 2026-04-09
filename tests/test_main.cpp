#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include <atomic>
#include "thread_pool.hpp"
#include "server.hpp"
#include <boost/asio.hpp>

// ============================================================
// Test 1: The thread pool creates the right number of threads
// ============================================================
TEST(ThreadPoolTest, CreatesCorrectNumberOfThreads) {
    const size_t num_threads = 4;
    ThreadPool pool(num_threads);
    
    // Giving threads time to start
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // It is not possible to directly check the number of threads,
    // but we can add tasks and make sure they are running
    std::atomic<int> counter = 0;
    
    for (int i = 0; i < num_threads * 10; ++i) {
        pool.enqueue([&counter]() {
            counter++;
        });
    }
    
    // Letting the tasks get done
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
        
        // Updating the maximum
        int expected = max_concurrent;
        while (current > expected && 
               !max_concurrent.compare_exchange_weak(expected, current)) {
            expected = max_concurrent;
        }
        
        // Simulate work
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        
        --concurrent_count;
    };
    
    // Running 8 tasks
    for (int i = 0; i < 8; ++i) {
        pool.enqueue(task);
    }
    
    // Giving Time to Complete
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    pool.shutdown();
    
    // At least 2 tasks were executed at the same time (weak parallel check)
    EXPECT_GT(max_concurrent, 1);
    std::cout << "Max concurrent tasks: " << max_concurrent << std::endl;
}

// ============================================================
// Тест 3: The echo server responds with the same message
// ============================================================
TEST(ServerTest, EchoesMessage) {
    boost::asio::io_context io_context;
    const uint16_t test_port = 18080;  // Use a different port for the test
    
    // Running the server in a separate thread
    EchoServer server(io_context, test_port, 2);
    server.start();
    
    std::thread io_thread([&io_context]() {
        io_context.run();
    });
    
    // Giving the server time to start
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // The client connects and sends a message
    boost::asio::ip::tcp::socket client_socket(io_context);
    boost::system::error_code ec;
    
    client_socket.connect(
        boost::asio::ip::tcp::endpoint(boost::asio::ip::make_address("127.0.0.1"), test_port),
        ec
    );
    
    ASSERT_FALSE(ec) << "Failed to connect: " << ec.message();
    
    const std::string test_message = "Hello, Echo Server!";
    const std::string test_message_with_newline = test_message + "\n";
    
    // Sending
    boost::asio::write(client_socket, boost::asio::buffer(test_message_with_newline), ec);
    ASSERT_FALSE(ec);
    
    // Reading the answer
    char buffer[1024];
    size_t len = client_socket.read_some(boost::asio::buffer(buffer), ec);
    ASSERT_FALSE(ec);
    
    std::string response(buffer, len);
    
    // Verify that the response contains our message
    EXPECT_TRUE(response.find(test_message) != std::string::npos);
    
    // Stopping the server
    server.stop();
    io_context.stop();
    if (io_thread.joinable()) io_thread.join();
}

// ============================================================
// Тест 4: shutdown() does not fall when called again
// ============================================================
TEST(ThreadPoolTest, ShutdownIsIdempotent) {
    ThreadPool pool(2);
    pool.shutdown();
    pool.shutdown();  // The second call must not fall
    SUCCEED();
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}