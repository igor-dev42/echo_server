#include "server.hpp"
#include <boost/asio.hpp>
#include <signal.h>
#include <iostream>
#include <memory>

std::unique_ptr<EchoServer> g_server;

void signal_handler(int sig) {
    std::cout << "\nSignal " << sig << " received. Shutting down..." << std::endl;
    if (g_server) {
        g_server->stop();
    }
}

int main(int argc, char* argv[]) {
    uint16_t port = 8080;
    if (argc > 1) port = static_cast<uint16_t>(std::stoi(argv[1]));
    
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    boost::asio::io_context io_context;
    g_server = std::make_unique<EchoServer>(io_context, port, 4);
    
    g_server->start();
    
    // Run io_context in separate thread for async operations
    std::thread io_thread([&io_context]() {
        io_context.run();
    });
    
    while (g_server && g_server->is_running()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    io_context.stop();
    if (io_thread.joinable()) io_thread.join();
    
    return 0;
}