#include <gtest/gtest.h>

#include "task_tracker.hpp"
#include "utils/test_helpers.cpp"

#include <atomic>
#include <chrono>
#include <latch>
#include <thread>
#include <vector>

using namespace std::chrono_literals;

TEST(TaskTracker, WaitReturnsAfterAllTasksDone) {
    NLib::NThreadPool::TThreadPool threadPool(4);
    NLib::NTaskTracker::TTaskTracker tracker(threadPool);

    constexpr int N = 10'000;
    std::atomic<int> counter{0};
    std::latch allStarted(N);

    for (int i = 0; i < N; ++i) {
        ASSERT_TRUE(tracker.Submit([&]{
            counter.fetch_add(1, std::memory_order_relaxed);
            allStarted.count_down();
        }));
    }

    allStarted.wait();
    tracker.Wait();

    EXPECT_EQ(counter.load(std::memory_order_relaxed), N);
}

TEST(TaskTracker, WaitBlocksUntilTasksFinish) {
    NLib::NThreadPool::TThreadPool threadPool(2);
    NLib::NTaskTracker::TTaskTracker tracker(threadPool);

    std::latch started(1);
    std::latch release(1);

    ASSERT_TRUE(tracker.Submit([&]{
        started.count_down();
        release.wait();
    }));

    started.wait();

    auto future = NUtils::NTestHelpers::RunAsync([&]{
        tracker.Wait();
        return true;
    });

    EXPECT_FALSE(NUtils::NTestHelpers::IsReadyAfter(future, 100ms));

    release.count_down();

    ASSERT_TRUE(NUtils::NTestHelpers::IsReadyAfter(future, 1s));
    EXPECT_TRUE(future.get());
}

TEST(TaskTracker, SupportRecursiveTaskSubmissions) {
    NLib::NThreadPool::TThreadPool threadPool(4);
    NLib::NTaskTracker::TTaskTracker tracker(threadPool);

    constexpr int CHILDREN = 50'000;
    std::atomic<int> doneChildren{0};
    std::latch doneChildrenLatch(CHILDREN);

    ASSERT_TRUE(tracker.Submit([&]{
        for (int i = 0; i < CHILDREN; ++i) {
            bool ok = tracker.Submit([&]{
                doneChildren.fetch_add(1, std::memory_order_relaxed);
                doneChildrenLatch.count_down();
            });
            ASSERT_TRUE(ok);
        }
    }));

    doneChildrenLatch.wait();
    tracker.Wait();

    EXPECT_EQ(doneChildren.load(std::memory_order_relaxed), CHILDREN);
}

TEST(TaskTracker, SubmitAfterDoneReturnsFalseAndTaskIsNotExecuted) {
    NLib::NThreadPool::TThreadPool threadPool(2);
    NLib::NTaskTracker::TTaskTracker tracker(threadPool);

    std::atomic<int> counter{0};

    ASSERT_TRUE(tracker.Submit([&]{
        counter.fetch_add(1, std::memory_order_relaxed);
    }));

    tracker.Wait();
    EXPECT_EQ(counter.load(std::memory_order_relaxed), 1);

    EXPECT_FALSE(tracker.Submit([&]{
        counter.fetch_add(1000, std::memory_order_relaxed);
    }));

    std::this_thread::sleep_for(50ms);
    EXPECT_EQ(counter.load(std::memory_order_relaxed), 1);
}

TEST(TaskTracker, SubmitAfterPoolShutdownDoesntLockWait) {
    NLib::NThreadPool::TThreadPool threadPool(2);
    NLib::NTaskTracker::TTaskTracker tracker(threadPool);

    threadPool.Shutdown();

    bool ok = tracker.Submit([]{
        // Nothing
    });
    EXPECT_FALSE(ok);

    auto future = NUtils::NTestHelpers::RunAsync([&]{
        tracker.Wait();
        return true;
    });

    ASSERT_TRUE(NUtils::NTestHelpers::IsReadyAfter(future, 200ms));
    EXPECT_TRUE(future.get());
}