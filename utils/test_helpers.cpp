#pragma once

#include <future>
#include <chrono>

namespace NUtils::NTestHelpers {
    template <class TFunction>
    auto RunAsync(TFunction&& function) {
        return std::async(std::launch::async, std::forward<TFunction>(function));
    }

    template <class T>
    bool IsReadyAfter(std::future<T>& future, std::chrono::milliseconds duration) {
        return future.wait_for(duration) == std::future_status::ready;
    }
}
