#include <gtest/gtest.h>

#include "thread_pool.hpp"
#include <utils/test_helpers.cpp>

#include <latch>

using namespace std::chrono_literals;

TEST(ThreadPool, PostExecutesManyTasks) {
    NLib::NThreadPool::TThreadPool threadPool(4);

    constexpr int N = 10'000;
    std::atomic<int> sum{0};
    std::latch done(N);

    for (int i = 0; i < N; i++) {
        ASSERT_TRUE(threadPool.Post([&]{
            sum++;
            done.count_down();
        }));
    }

    done.wait();
    EXPECT_EQ(sum.load(), N);
}

TEST(ThreadPool, SubmitExecutesManyTasks) {
    NLib::NThreadPool::TThreadPool threadPool(4);

    constexpr int N = 10'000;
    std::atomic<int> sum{0};
    std::vector<std::future<void>> futures;
    futures.reserve(N);

    for (int i = 0; i < N; i++) {
        futures.push_back(threadPool.Submit([&]{
            sum++;
        }).second);
    }

    for (auto& f : futures) {
        f.get();
    }

    EXPECT_EQ(sum.load(), N);
}

TEST(ThreadPool, SubmitReturnsValue) {
    NLib::NThreadPool::TThreadPool threadPool(4);

    auto [_, future] = threadPool.Submit([]{ return 777; });
    EXPECT_EQ(future.get(), 777);

    threadPool.Shutdown();
}

TEST(ThreadPool, SubmitPropagatesException) {
    NLib::NThreadPool::TThreadPool threadPool(4);

    auto [_, future] = threadPool.Submit([]{ throw std::runtime_error("kek"); });
    EXPECT_THROW(future.get(), std::runtime_error);
}

TEST(ThreadPool, ShutdownDoesntKillRunningTasks) {
    NLib::NThreadPool::TThreadPool threadPool(4);

    std::latch started(2), release(1), finished(2);

    for (int i = 0; i < 2; i++) {
        ASSERT_TRUE(threadPool.Post([&]{
            started.count_down();
            release.wait();
            finished.count_down();
        }));
    }

    started.wait();

    auto shutdown = NUtils::NTestHelpers::RunAsync([&]{
        threadPool.Shutdown();
    });

    ASSERT_FALSE(NUtils::NTestHelpers::IsReadyAfter(shutdown, 200ms));

    release.count_down();
    finished.wait();

    ASSERT_TRUE(NUtils::NTestHelpers::IsReadyAfter(shutdown, 1s));
}

TEST(ThreadPool, PostAfterShutdownFails) {
    NLib::NThreadPool::TThreadPool threadPool(4);
    threadPool.Shutdown();
    ASSERT_FALSE(threadPool.Post([]{}));
}

TEST(ThreadPool, SubmitAfterShutdownFails) {
    NLib::NThreadPool::TThreadPool threadPool(4);
    threadPool.Shutdown();
    auto [ok, future] = threadPool.Submit([]{});
    ASSERT_FALSE(ok);
    ASSERT_THROW(future.get(), std::runtime_error);
}

TEST(ThreadPool, PostFromManyThreads) {
    auto attempts = 10;
    while (attempts--) {
        NLib::NThreadPool::TThreadPool threadPool(4);

        constexpr int THREADS = 8;
        constexpr int PER_THREAD = 10'000;
        std::atomic<int> sum{0};
        std::latch done(THREADS * PER_THREAD);

        std::vector<std::thread> threads;
        threads.reserve(THREADS);
        for (int t = 0; t < THREADS; t++) {
            threads.emplace_back([&]{
                for (int i = 0; i < PER_THREAD; i++) {
                    ASSERT_TRUE(threadPool.Post([&]{
                        sum++;
                        done.count_down();
                    }));
                }
            });
        }

        for (auto& t : threads) {
            t.join();
        }
        done.wait();

        EXPECT_EQ(sum.load(), THREADS * PER_THREAD);
    }
}

TEST(ThreadPool, PostFromPost) {
    NLib::NThreadPool::TThreadPool threadPool(4);

    constexpr int CHILD = 50'000;
    std::atomic<int> sum{0};
    std::latch done(CHILD);

    ASSERT_TRUE(threadPool.Post([&]{
        for (int i = 0; i < CHILD; ++i) {
            ASSERT_TRUE(threadPool.Post([&]{
                sum++;
                done.count_down();
            }));
        }
    }));

    done.wait();
    EXPECT_EQ(sum.load(), CHILD);
}