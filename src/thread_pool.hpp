#pragma once
#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>
#include <atomic>

class ThreadPool {
public:
    using Task = std::function<void()>;
    
    explicit ThreadPool(size_t num_threads);
    ~ThreadPool();
    
    void enqueue(Task task);
    void shutdown();
    
private:
    void worker();
    
    std::vector<std::thread> workers_;
    std::queue<Task> tasks_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::atomic<bool> running_;
    std::atomic<bool> shutdown_;
};