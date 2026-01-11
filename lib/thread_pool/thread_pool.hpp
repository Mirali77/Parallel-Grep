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
        std::pair<bool, std::future<std::invoke_result_t<TFunction, TArgs...>>> // -> [is task pushed, future]
        Submit(TFunction&& function, TArgs&&... args){ // If you need future
            using TResult = std::invoke_result_t<TFunction, TArgs...>;

            auto promise = std::make_shared<std::promise<TResult>>();
            auto future = promise->get_future();

            auto task = [promise, func = std::bind(std::forward<TFunction>(function), std::forward<TArgs>(args)...)] {
                try {
                    if constexpr (std::is_void_v<TResult>) {
                        func();
                        promise->set_value();
                    } else {
                        promise->set_value(func());
                    }
                } catch (...) {
                    promise->set_exception(std::current_exception());
                }
            };

            if (!Tasks_.Push(std::move(task))) {
                promise->set_exception(std::make_exception_ptr(std::runtime_error("ThreadPool is shut down")));
                return {false, std::move(future)};
            }

            return {true, std::move(future)};
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
