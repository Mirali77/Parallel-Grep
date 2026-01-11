#include <gtest/gtest.h>

#include "blocking_queue.hpp"
#include <utils/test_helpers.cpp>

using namespace std::chrono_literals;

TEST(BlockingQueue, FifoSingleThread) {
    NLib::NThreadPool::TBlockingQueue<int> q;
    ASSERT_TRUE(q.Push(1));
    ASSERT_TRUE(q.Push(2));
    ASSERT_TRUE(q.Push(3));

    int x;
    ASSERT_TRUE(q.Pop(x)); EXPECT_EQ(x, 1);
    ASSERT_TRUE(q.Pop(x)); EXPECT_EQ(x, 2);
    ASSERT_TRUE(q.Pop(x)); EXPECT_EQ(x, 3);
}

TEST(BlockingQueue, PopBlocksUntilPush) {
    NLib::NThreadPool::TBlockingQueue<int> q;

    auto future = NUtils::NTestHelpers::RunAsync([&]{
        int x;
        bool ok = q.Pop(x);
        return std::pair<bool,int>{ok, x};
    });

    // Должен блокироваться, т.к. никого пока не пушнули
    EXPECT_FALSE(NUtils::NTestHelpers::IsReadyAfter(future, 100ms));

    ASSERT_TRUE(q.Push(42));

    ASSERT_TRUE(NUtils::NTestHelpers::IsReadyAfter(future, 1s));
    auto [ok, x] = future.get();
    EXPECT_TRUE(ok);
    EXPECT_EQ(x, 42);
}

TEST(BlockingQueue, Close) {
    NLib::NThreadPool::TBlockingQueue<int> q;
    ASSERT_TRUE(q.Push(1));
    int x;
    ASSERT_TRUE(q.Pop(x));
    
    q.Close();
    ASSERT_TRUE(q.IsClosed());
}

TEST(BlockingQueue, CloseWakesPop) {
    NLib::NThreadPool::TBlockingQueue<int> q;

    auto future = NUtils::NTestHelpers::RunAsync([&]{
        int x;
        bool ok = q.Pop(x);
        return std::pair<bool,int>{ok, x};
    });

    EXPECT_FALSE(NUtils::NTestHelpers::IsReadyAfter(future, 100ms));

    q.Close();

    ASSERT_TRUE(NUtils::NTestHelpers::IsReadyAfter(future, 1s));
    auto [ok, x] = future.get();

    ASSERT_FALSE(ok);
}

TEST(BlockingQueue, CloseDoesntAffectExistingItems) {
    NLib::NThreadPool::TBlockingQueue<int> q;
    ASSERT_TRUE(q.Push(1));
    ASSERT_TRUE(q.Push(2));

    q.Close();

    int x;
    ASSERT_TRUE(q.Pop(x)); EXPECT_EQ(x, 1);
    ASSERT_TRUE(q.Pop(x)); EXPECT_EQ(x, 2);
    EXPECT_FALSE(q.Pop(x));
}

TEST(BlockingQueue, ClosePreventsPush) {
    NLib::NThreadPool::TBlockingQueue<int> q;
    q.Close();

    ASSERT_FALSE(q.Push(1));
}

TEST(BlockingQueue, MPMCNoDubNoLoss) {
    // Multi-processor/Multi-consumer test
    auto attempts = 10;
    while (attempts--) {
        NLib::NThreadPool::TBlockingQueue<int> q;

        constexpr int P = 4, C = 4, PER_PRODUCER = 10'000;
        auto total = P * PER_PRODUCER;

        std::mutex consumedMutex;
        std::vector<int> consumed;
        consumed.reserve(total);

        std::vector<std::thread> producers;
        producers.reserve(P);
        for (int idx = 0; idx < P; idx++) {
            producers.emplace_back([&, idx]{
                int base = idx * PER_PRODUCER;
                for (int i = 0; i < PER_PRODUCER; i++) {
                    ASSERT_TRUE(q.Push(base + i));
                }
            });
        }

        std::vector<std::thread> consumers;
        consumers.reserve(C);
        for (int idx = 0; idx < C; idx++) {
            consumers.emplace_back([&]{
                int x;
                while (q.Pop(x)) {
                    std::lock_guard lock(consumedMutex);
                    consumed.push_back(x);
                }
            });
        }

        for (auto& p : producers) {
            p.join();
        }
        q.Close();
        for (auto& c : consumers) {
            c.join();
        }

        ASSERT_EQ(consumed.size(), total);

        std::vector<bool> seen(total);
        for (auto v : consumed) {
            ASSERT_GE(v, 0);
            ASSERT_LT(v, total);
            ASSERT_FALSE(seen[v]) << "Dublicate at " << v;
            seen[v] = true;
        }

        for (int idx = 0; idx < total; idx++) {
            ASSERT_TRUE(seen[idx]) << "Missing " << idx;
        }
    }
}
