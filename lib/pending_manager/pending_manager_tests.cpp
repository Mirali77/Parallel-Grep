#include <gtest/gtest.h>
#include "pending_manager.hpp"
#include <utils/test_helpers.cpp>
#include <latch>
#include <lib/thread_pool/thread_pool.hpp>

using namespace std::chrono_literals;

TEST(Pending, WaitZeroReturnsImmediatelyWhenZero) {
    NLib::NPendingManager::TPendingManager pendingManager;

    auto future = NUtils::NTestHelpers::RunAsync([&]{
        pendingManager.Wait();
        return 1;
    });

    ASSERT_TRUE(NUtils::NTestHelpers::IsReadyAfter(future, 100ms));
    EXPECT_EQ(future.get(), 1);
}

TEST(Pending, WaitZeroBlocksUntilCounterReachesZero) {
    NLib::NPendingManager::TPendingManager pendingManager;

    pendingManager.Increment();

    auto future = NUtils::NTestHelpers::RunAsync([&]{
        pendingManager.Wait();
        return 1;
    });

    EXPECT_FALSE(NUtils::NTestHelpers::IsReadyAfter(future, 100ms));

    pendingManager.Decrement();

    ASSERT_TRUE(NUtils::NTestHelpers::IsReadyAfter(future, 1s));
    EXPECT_EQ(future.get(), 1);
}

TEST(Pending, WaitZeroWakesAllWaiters) {
    NLib::NPendingManager::TPendingManager pendingManager;

    pendingManager.Increment();

    auto f1 = NUtils::NTestHelpers::RunAsync([&]{
        pendingManager.Wait();
        return 1;
    });

    auto f2 = NUtils::NTestHelpers::RunAsync([&]{
        pendingManager.Wait();
        return 1;
    });

    EXPECT_FALSE(NUtils::NTestHelpers::IsReadyAfter(f1, 100ms));
    EXPECT_FALSE(NUtils::NTestHelpers::IsReadyAfter(f2, 100ms));

    pendingManager.Decrement();

    ASSERT_TRUE(NUtils::NTestHelpers::IsReadyAfter(f1, 1s));
    ASSERT_TRUE(NUtils::NTestHelpers::IsReadyAfter(f2, 1s));
    EXPECT_EQ(f1.get(), 1);
    EXPECT_EQ(f2.get(), 1);
}

TEST(Pending, ManyIncrementsThenManyDecrements) {
    NLib::NPendingManager::TPendingManager pendingManager;

    NLib::NThreadPool::TThreadPool threadPool(4);
    constexpr int N = 50'000;
    std::latch started(N);

    for (int i = 0; i < N; ++i) {
        pendingManager.Increment();
    }

    for (int i = 0; i < N; ++i) {
        threadPool.Post([&]{
            started.count_down();
            pendingManager.Decrement();
        });
    }

    started.wait();

    auto future = NUtils::NTestHelpers::RunAsync([&]{
        pendingManager.Wait();
        return 1;
    });

    ASSERT_TRUE(NUtils::NTestHelpers::IsReadyAfter(future, 2s));
    EXPECT_EQ(future.get(), 1);
}

TEST(Pending, StressIncDecFromManyThreadsNoDeadlock) {
    NLib::NPendingManager::TPendingManager pendingManager;

    constexpr int THREADS = 8;
    constexpr int ITERS = 100'000;

    std::atomic<long long> ops{0};
    std::vector<std::thread> threads;
    threads.reserve(THREADS);

    for (int t = 0; t < THREADS; ++t) {
        threads.emplace_back([&]{
            for (int i = 0; i < ITERS; ++i) {
                pendingManager.Increment();
                pendingManager.Decrement();
                ops.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    auto future = NUtils::NTestHelpers::RunAsync([&]{
        pendingManager.Wait();
        return 1;
    });

    ASSERT_TRUE(NUtils::NTestHelpers::IsReadyAfter(future, 1s));
    EXPECT_EQ(future.get(), 1);

    EXPECT_EQ(ops.load(std::memory_order_relaxed), 1LL * THREADS * ITERS);
}
