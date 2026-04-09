#pragma once
#include <boost/asio.hpp>
#include <memory>
#include <atomic>
#include "thread_pool.hpp"

class EchoServer {
public:
    EchoServer(boost::asio::io_context& io_context, uint16_t port, size_t thread_count = 4);
    ~EchoServer();
    
    void start();
    void stop();
    bool is_running() const { return running_; }
    
private:
    void do_accept();
    void handle_client(std::shared_ptr<boost::asio::ip::tcp::socket> socket);
    
    boost::asio::io_context& io_context_;
    boost::asio::ip::tcp::acceptor acceptor_;
    ThreadPool thread_pool_;
    std::atomic<bool> running_;
};