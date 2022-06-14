//
// Created by 戴佳 on 2022/5/27.
//

#ifndef WEBSERVER_THREAD_POOL_HPP
#define WEBSERVER_THREAD_POOL_HPP

#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
#include <functional>

class ThreadPool {
public:
    explicit ThreadPool(int numOfThreads) : workerThreads_(std::vector<std::thread>(numOfThreads)),
                                            isShutdown_(false) {
        for (int i = 0; i < numOfThreads; ++i) {
            workerThreads_[i] = std::thread(run);
        }
    }

    ThreadPool &operator=(const ThreadPool &) = delete;

    ThreadPool &operator=(const ThreadPool &&) = delete;

    ~ThreadPool() {
        shutdown();
    }

    /**
     * 提交任务
     *
     * @param f 函数对象名
     * @param args 参数列表
     * @return std::future 对象
     */
    template<typename F, typename ...Args>
    auto submit(F &&f, Args &&... args) -> std::future<decltype(f(args...))> {
        std::function<decltype(f(args...))()> func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);

        auto taskPointer = std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);

        std::function<void()> wrapFunc = [taskPointer]() {
            (*taskPointer)();
        };

        taskQueue_.enqueue(wrapFunc);

        workerThreadsConditionVariable_.notify_one();

        return taskPointer->get_future();
    }

private:
    /**
     * 工作任务队列（线程安全）
     */
    template<typename T>
    class TaskQueue {
    public:
        TaskQueue() = default;

        TaskQueue(TaskQueue &) = default;

        ~TaskQueue() = default;

        bool empty() {
            const std::lock_guard<std::mutex> lockGuard(taskQueueMutex);
            return taskQueue.empty();
        }

        size_t size() {
            const std::lock_guard<std::mutex> lockGuard(taskQueueMutex);
            return taskQueue.size();
        }

        void enqueue(T &task) {
            const std::lock_guard<std::mutex> lockGuard(taskQueueMutex);
            taskQueue.push(task);
        }

        std::pair<T, bool> dequeue() {
            const std::lock_guard<std::mutex> lockGuard(taskQueueMutex);

            if (taskQueue.empty()) {
                return std::pair(T(), false);
            }

            T task = taskQueue.front();
            taskQueue.pop();
            return std::pair(task, true);
        }

    private:
        std::mutex taskQueueMutex;

        std::queue<T> taskQueue;
    };

    TaskQueue<std::function<void()>> taskQueue_;

    std::vector<std::thread> workerThreads_;

    std::mutex workerThreadsMutex_;

    std::condition_variable workerThreadsConditionVariable_;

    bool isShutdown_;

    // 线程池中的每个线程都要执行的任务代码
    std::function<void()> run = [&]() -> void {
        std::function<void()> func;

        bool dequeued;

        while (!isShutdown_) {
            dequeued = false;
            {
                std::unique_lock<std::mutex> lock(workerThreadsMutex_);

                workerThreadsConditionVariable_.wait(lock, [this]() -> bool {
                    return !taskQueue_.empty() || isShutdown_;
                });

                if (auto result = taskQueue_.dequeue(); result.second) {
                    func = result.first;
                    dequeued = result.second;
                }
            }

            if (dequeued) {
                func();
            }
        }
    };

    /**
     * 关闭线程池
     *
     * 在所有线程的任务完成后，关闭线程池
     */
    void shutdown() {
        isShutdown_ = true;
        workerThreadsConditionVariable_.notify_all();
        std::for_each(workerThreads_.begin(), workerThreads_.end(), [](std::thread &thread) -> void {
            if (thread.joinable()) {
                thread.join();
            }
        });
    }
};

/**
 * 返回单例线程池实例
 *
 * @return 单例线程池实例
 */
ThreadPool& getThreadPool() {
    static ThreadPool pool(10);
    return pool;
}

#endif //WEBSERVER_THREAD_POOL_HPP
