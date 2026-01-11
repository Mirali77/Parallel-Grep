#pragma once

#include "blocking_queue.hpp"
#include <atomic>
#include <functional>
#include <future>
#include <iostream>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>


namespace NLib::NThreadPool {
    class TThreadPool {
    public:
        explicit TThreadPool(std::size_t threads = std::thread::hardware_concurrency());
        TThreadPool(const TThreadPool&) = delete;
        TThreadPool& operator=(const TThreadPool&) = delete;

        ~TThreadPool();
        
        template <class TFunction, class... TArgs>
        std::future<std::invoke_result_t<TFunction, TArgs...>>
        Submit(TFunction&& function, TArgs&&... args){ // If you need future
            using TResult = std::invoke_result_t<TFunction, TArgs...>;

            auto taskPtr = std::make_shared<std::packaged_task<TResult()>>(
                std::bind(std::forward<TFunction>(function), std::forward(args)...)
            );

            auto future = taskPtr->get_future();

            if (!Tasks_.Push([&taskPtr] {
                (*taskPtr)();
            })) {
                try {
                    throw std::runtime_error("Thread pool is shut down");
                } catch (...) {
                    taskPtr->set_exception(std::current_exception());
                }
            }

            return future;
        }

        template <class TFunction>
        bool Post(TFunction&& function) { // If you want to post and forget
            return Tasks_.Push(std::function<void()>(std::forward<TFunction>(function)));
        }

        void Shutdown();

    private:
        void GetWorkerLoop();

        std::atomic<bool> StopFlg_;
        TBlockingQueue<std::function<void()>> Tasks_;
        std::vector<std::thread> Workers_;
    };
}
