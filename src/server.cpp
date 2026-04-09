#include "server.hpp"
#include <iostream>
#include <system_error>

EchoServer::EchoServer(boost::asio::io_context& io_context, uint16_t port, size_t thread_count)
    : io_context_(io_context)
    , acceptor_(io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
    , thread_pool_(thread_count)
    , running_(false) {
    
    std::cout << "Server listening on port " << port << std::endl;
}

EchoServer::~EchoServer() {
    stop();
}

void EchoServer::start() {
    running_ = true;
    do_accept();
}

void EchoServer::stop() {
    if (!running_.exchange(false)) return;
    
    boost::system::error_code ec;
    acceptor_.close(ec);
    thread_pool_.shutdown();
    
    std::cout << "Server stopped" << std::endl;
}

void EchoServer::do_accept() {
    auto socket = std::make_shared<boost::asio::ip::tcp::socket>(io_context_);
    
    acceptor_.async_accept(*socket, [this, socket](const boost::system::error_code& error) {
        if (!error && running_) {
            std::cout << "New connection from " << socket->remote_endpoint() << std::endl;
            thread_pool_.enqueue([this, socket]() {
                handle_client(socket);
            });
        } else if (error && running_) {
            std::cerr << "Accept error: " << error.message() << std::endl;
        }
        
        if (running_) do_accept();
    });
}

void EchoServer::handle_client(std::shared_ptr<boost::asio::ip::tcp::socket> socket) {
    try {
        char data[1024];
        
        while (running_) {
            // Exception-based error handling
            size_t len = socket->read_some(boost::asio::buffer(data));
            
            // Send back
            boost::asio::write(*socket, boost::asio::buffer(data, len));
        }
    } catch (const boost::system::system_error& e) {
        if (e.code() == boost::asio::error::eof) {
            std::cout << "Client disconnected" << std::endl;
        } else {
            std::cerr << "Client error: " << e.what() << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Unexpected error: " << e.what() << std::endl;
    }
    
    boost::system::error_code ignore;
    socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignore);
    socket->close(ignore);
}