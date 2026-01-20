#include <gtest/gtest.h>
#include "semaphore.hpp"
#include <utils/test_helpers.cpp>
#include <latch>

using namespace std::chrono_literals;

TEST(Semaphore, AcquireBlocksUntilRelease) {
    NSemaphore::TSemaphore semaphore(0);

    auto future = NUtils::NTestHelpers::RunAsync([&]{
        semaphore.Acquire();
        return 1;
    });

    EXPECT_FALSE(NUtils::NTestHelpers::IsReadyAfter(future, 100ms));

    semaphore.Release();

    ASSERT_TRUE(NUtils::NTestHelpers::IsReadyAfter(future, 1s));
    EXPECT_EQ(future.get(), 1);
}

TEST(Semaphore, TryAcquire) {
    NSemaphore::TSemaphore semaphore(0);

    EXPECT_FALSE(semaphore.TryAcquire());

    semaphore.Release();

    EXPECT_TRUE(semaphore.TryAcquire());
    EXPECT_FALSE(semaphore.TryAcquire());
}

TEST(Semaphore, ReleaseNAllowsNAcquires) {
    NSemaphore::TSemaphore semaphore(0);

    semaphore.Release(3);

    semaphore.Acquire();
    semaphore.Acquire();
    semaphore.Acquire();

    EXPECT_FALSE(semaphore.TryAcquire());
}

TEST(Semaphore, MultipleWaitersAreWokenByReleaseN) {
    NSemaphore::TSemaphore semaphore(0);

    constexpr int N = 8;
    std::latch ready(N);
    std::atomic<int> passed{0};

    std::vector<std::thread> threads;
    threads.reserve(N);

    for (int i = 0; i < N; ++i) {
        threads.emplace_back([&]{
            ready.count_down();
            semaphore.Acquire();
            passed.fetch_add(1, std::memory_order_relaxed);
        });
    }

    ready.wait();

    std::this_thread::sleep_for(50ms);
    EXPECT_EQ(passed.load(std::memory_order_relaxed), 0);

    semaphore.Release(N);

    for (auto& t : threads) {
        t.join();
    }

    EXPECT_EQ(passed.load(std::memory_order_relaxed), N);
}

TEST(Semaphore, StressTokenPassingNoDeadlock) {
    constexpr int THREADS = 8;
    constexpr int ITERS = 50'000;

    NSemaphore::TSemaphore semaphore(1);
    std::atomic<long long> sum{0};

    std::vector<std::thread> threads;
    threads.reserve(THREADS);

    for (int t = 0; t < THREADS; ++t) {
        threads.emplace_back([&]{
            for (int i = 0; i < ITERS; ++i) {
                semaphore.Acquire();
                sum.fetch_add(1, std::memory_order_relaxed);
                semaphore.Release();
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    EXPECT_EQ(sum.load(std::memory_order_relaxed), 1LL * THREADS * ITERS);
}

TEST(Semaphore, ReleaseWakesOneWaiter) {
    NSemaphore::TSemaphore semaphore(0);

    std::latch ready(2);
    std::atomic<int> passed{0};

    auto worker = [&]{
        ready.count_down();
        semaphore.Acquire();
        passed.fetch_add(1, std::memory_order_relaxed);
    };

    std::thread a(worker);
    std::thread b(worker);

    ready.wait();

    semaphore.Release();

    for (int i = 0; i < 20 && passed.load(std::memory_order_relaxed) == 0; ++i) {
        std::this_thread::sleep_for(10ms);
    }

    EXPECT_GE(passed.load(std::memory_order_relaxed), 1);

    semaphore.Release();

    a.join();
    b.join();

    EXPECT_EQ(passed.load(std::memory_order_relaxed), 2);
}