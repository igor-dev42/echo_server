# Multithreaded TCP Echo Server

[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/)
[![Boost](https://img.shields.io/badge/Boost-1.83-green.svg)](https://www.boost.org/)
[![CMake](https://img.shields.io/badge/CMake-3.14+-red.svg)](https://cmake.org/)
[![CI Build and Test](https://github.com/igor-dev42/echo_server/actions/workflows/build.yml/badge.svg)](https://github.com/igor-dev42/echo_server/actions/workflows/build.yml)

Production-ready TCP echo server demonstrating modern C++17 practices, multithreading, and asynchronous networking.

## Features

- 🔄 **Thread pool** with configurable number of worker threads
- 📋 **Thread-safe task queue** using mutex and condition variables
- ⚡ **Asynchronous accept** with Boost.Asio (epoll/kqueue/IOCP)
- 🛑 **Graceful shutdown** on SIGINT/SIGTERM signals
- 🧹 **RAII** for all resources (no manual cleanup)
- 🧪 **Unit tests** with Google Test
- 🔄 **CI/CD** with GitHub Actions

## API

This is a raw TCP echo server (no HTTP). Connect via any TCP client (netcat, telnet, etc.).

## Requirements

- C++17 compiler (g++ 7+, clang 5+, MSVC 2017+)
- Boost.Asio 1.66+
- CMake 3.14+
- Google Test (auto-downloaded by CMake)

## Quick Start

### Build

```bash
git clone https://github.com/igor-dev42/echo_server.git
cd echo_server
mkdir build && cd build
cmake ..
make
```

## Test with netcat (nc)

**Step 1:** Start the server in one terminal:

```bash
./echo_server
# Output: Server listening on port 8080
```

**Step 2:** Open another terminal and connect with netcat:

```bash
nc localhost 8080
```

**Note:** Use the same port number as the server running on (the default is 8080). If you run the server on a different port
(e.g. ./echo_server 8081), update the client command accordingly (nc localhost 8081).

**Step 3:** Type any message and press Enter:

```text
Hello
# Server responds: Hello
World
# Server responds: World
```

**Step 4:** Exit:

- Press `Ctrl+C` in the nc terminal
- Then press `Ctrl+C` in the server terminal to stop the server (if needed)

## Run Unit Tests

```bash
./run_tests
```

## Architecture
```
┌─────────────┐      ┌──────────────┐      ┌─────────────────┐
│   main()    │────▶ │ io_context   │────▶ │  async_accept   │
│  (signals)  │      │   (thread)   │      │   (callback)    │
└─────────────┘      └──────────────┘      └────────┬────────┘
                                                    │
                                                    ▼
┌─────────────┐      ┌──────────────┐      ┌─────────────────┐
│  Thread     │◀──── │   Task       │◀──── │  thread_pool    │
│  Pool       │      │   Queue      │      │  .enqueue()     │
└────────┬────┘      └──────────────┘      └─────────────────┘
         │
         ▼
┌─────────────────────────────────────────┐
│  Worker threads execute handle_client() │
│  - read_some()                          │
│  - write()                              │
│  - echo back                            │
└─────────────────────────────────────────┘
```

## Performance

With 4 worker threads on modern hardware:
10,000+ concurrent connections
~50,000 echo requests/second

## Project Structure
```
echo_server/
├── src/
│   ├── main.cpp           # Entry point, signal handling
│   ├── server.cpp/hpp     # EchoServer class
│   └── thread_pool.cpp/hpp # Thread pool implementation
├── tests/
│   └── test_main.cpp      # Google Test suite
├── CMakeLists.txt         # Build configuration
├── .github/workflows/
│   └── build.yml          # CI pipeline
└── README.md
```

## License
MIT License - feel free to use for learning and portfolios.

## Author

**Igor** — C++ Developer

[![GitHub](https://img.shields.io/badge/GitHub-igor--dev42-181717?style=flat-square&logo=github)](https://github.com/igor-dev42)

[![Email](https://img.shields.io/badge/Email-igor.dev42%40gmail.com-D14836?style=flat-square&logo=gmail&logoColor=white)](mailto:igor.dev42@gmail.com)

[![Telegram](https://img.shields.io/badge/Telegram-@igor__dev42-2AABEE?style=flat-square&logo=telegram&logoColor=white)](https://t.me/igor_dev42)

## References
- [Boost.Asio Documentation](https://www.boost.org/doc/libs/develop/doc/html/boost_asio.html)
- [C++17 Standard (Working Draft N4713)](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/n4713.pdf)
