#include <atomic>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "Event.hpp"

namespace
{
    int g_free_function_sum = 0;

    void freeFunctionListener(const int& value)
    {
        g_free_function_sum += value;
    }

    struct BaseListener
    {
        int base_sum{0};

        void onBaseEvent(int v)
        {
            base_sum += v;
        }
    };

    struct DerivedListener : BaseListener
    {
        int mutable_sum{0};
        mutable int const_sum{0};
        int noexcept_sum{0};
        int non_void_calls{0};

        void onMutable(int v)
        {
            mutable_sum += v;
        }

        void onConst(int v) const
        {
            const_sum += v;
        }

        void onNoexcept(int v) noexcept
        {
            noexcept_sum += v;
        }

        void onConstNoexcept(int v) const noexcept
        {
            const_sum += v;
        }

        int onReturningInt(int v)
        {
            non_void_calls += v;
            return non_void_calls;
        }

        static void onStatic(const int& v)
        {
            g_free_function_sum += v;
        }
    };

    struct CopyTracker
    {
        int value{0};
        static inline int copy_count{0};
        static inline int move_count{0};

        explicit CopyTracker(int v)
            : value(v)
        {
        }

        CopyTracker(const CopyTracker& other)
            : value(other.value)
        {
            ++copy_count;
        }

        CopyTracker(CopyTracker&& other) noexcept
            : value(other.value)
        {
            ++move_count;
        }

        CopyTracker& operator=(const CopyTracker&) = default;
        CopyTracker& operator=(CopyTracker&&) noexcept = default;
    };

} // namespace

TEST(EventTest, ParameterlessEventSubscribeNotifyAndOperatorCall)
{
    CppEvents::Event<> ev;
    EXPECT_TRUE(ev.empty());
    EXPECT_EQ(ev.listenerCount(), 0u);

    int counter = 0;
    auto id = ev += [&counter]() { ++counter; };

    EXPECT_NE(id, CppEvents::Event<>::InvalidConnectionId);
    EXPECT_FALSE(ev.empty());
    EXPECT_EQ(ev.listenerCount(), 1u);

    ev.notify();
    EXPECT_EQ(counter, 1);

    ev(); // operator() alias
    EXPECT_EQ(counter, 2);

    ev -= id;
    EXPECT_TRUE(ev.empty());
    EXPECT_EQ(ev.listenerCount(), 0u);

    ev.notify();
    EXPECT_EQ(counter, 2);
}

TEST(EventTest, MultiArgumentEventAndFifoInvocationOrder)
{
    CppEvents::Event<int, std::string, double> ev;
    std::vector<std::string> call_order;

    ev += [&](int id, const std::string& label, double factor)
    {
        call_order.push_back("first:" + label + ":" + std::to_string(id) + ":" +
                             std::to_string(static_cast<int>(factor)));
    };

    ev += [&](int id, const std::string& label, double factor)
    {
        call_order.push_back("second:" + label + ":" + std::to_string(id * 2) + ":" +
                             std::to_string(static_cast<int>(factor)));
    };

    ev.notify(7, "item", 3.0);

    ASSERT_EQ(call_order.size(), 2u);
    EXPECT_EQ(call_order[0], "first:item:7:3");
    EXPECT_EQ(call_order[1], "second:item:14:3");
}

TEST(EventTest, FreeFunctionsStaticMethodsMutableLambdasAndFunctors)
{
    g_free_function_sum = 0;
    CppEvents::Event<int> ev;

    ev += &freeFunctionListener;
    ev += &DerivedListener::onStatic;

    int captured_total = 0;
    ev += [accumulator = 0, &captured_total](int v) mutable
    {
        accumulator += v;
        captured_total = accumulator;
    };

    struct CustomFunctor
    {
        int* target;
        void operator()(int v) const
        {
            *target += v * 10;
        }
    };
    int functor_result = 0;
    ev += CustomFunctor{&functor_result};

    ev.notify(5);
    EXPECT_EQ(g_free_function_sum, 10);
    EXPECT_EQ(captured_total, 5);
    EXPECT_EQ(functor_result, 50);

    ev.notify(3);
    EXPECT_EQ(g_free_function_sum, 16);
    EXPECT_EQ(captured_total, 8);
    EXPECT_EQ(functor_result, 80);
}

TEST(EventTest, ZeroCopyArgumentForwardingDuringNotify)
{
    CopyTracker::copy_count = 0;
    CopyTracker::move_count = 0;

    CppEvents::Event<CopyTracker> ev;
    int sum = 0;

    ev += [&sum](const CopyTracker& tracker) { sum += tracker.value; };
    ev += [&sum](const CopyTracker& tracker) { sum += tracker.value * 2; };

    CopyTracker payload(10);
    ev.notify(payload);

    EXPECT_EQ(sum, 30);
    EXPECT_EQ(CopyTracker::copy_count, 0);
    EXPECT_EQ(CopyTracker::move_count, 0);
}

TEST(EventTest, MemberFunctionBindingsConstNonConstNoexceptBaseAndNonVoid)
{
    CppEvents::Event<int> ev;
    DerivedListener obj;
    const DerivedListener& const_obj = obj;

    auto id1 = ev += std::pair(&obj, &DerivedListener::onMutable);
    auto id2 = ev += std::pair(&obj, &DerivedListener::onConst);          // const method on non-const pointer
    auto id3 = ev += std::pair(&const_obj, &DerivedListener::onConst);    // const method on const pointer
    auto id4 = ev += std::pair(&obj, &DerivedListener::onNoexcept);
    auto id5 = ev += std::pair(&const_obj, &DerivedListener::onConstNoexcept);
    auto id6 = ev += std::pair(&obj, &DerivedListener::onBaseEvent);      // inherited from BaseListener
    auto id7 = ev += std::pair(&obj, &DerivedListener::onReturningInt);   // returns int (discarded)

    EXPECT_EQ(ev.listenerCount(), 7u);

    ev.notify(4);

    EXPECT_EQ(obj.mutable_sum, 4);
    EXPECT_EQ(obj.const_sum, 12); // 3 const bindings * 4
    EXPECT_EQ(obj.noexcept_sum, 4);
    EXPECT_EQ(obj.base_sum, 4);
    EXPECT_EQ(obj.non_void_calls, 4);

    ev -= id1;
    ev -= id2;
    ev -= id3;
    ev -= id4;
    ev -= id5;
    ev -= id6;
    ev -= id7;
    EXPECT_TRUE(ev.empty());
}

TEST(EventTest, SmartPointerBindingsWeakPtrExpirationAndSharedPtrLifetime)
{
    CppEvents::Event<int> ev;

    auto target = std::make_shared<DerivedListener>();
    std::weak_ptr<DerivedListener> weak_target = target;

    auto weak_id = ev += std::pair(weak_target, &DerivedListener::onMutable);
    EXPECT_NE(weak_id, CppEvents::Event<int>::InvalidConnectionId);

    ev.notify(6);
    EXPECT_EQ(target->mutable_sum, 6);

    // Destroy target; weak_ptr expires -> notify() must safely skip without crash
    target.reset();
    EXPECT_TRUE(weak_target.expired());
    ev.notify(6);
    ev -= weak_id;
    EXPECT_TRUE(ev.empty());

    // std::shared_ptr binding keeps target alive until unsubscribed
    auto shared_target = std::make_shared<DerivedListener>();
    std::weak_ptr<DerivedListener> observer = shared_target;

    auto shared_id = ev += std::pair(shared_target, &DerivedListener::onConst);
    shared_target.reset(); // Event still holds a shared_ptr copy
    ASSERT_FALSE(observer.expired());

    ev.notify(9);
    EXPECT_EQ(observer.lock()->const_sum, 9);

    ev -= shared_id;
    EXPECT_TRUE(observer.expired());
}

TEST(EventTest, NullPointersEmptyFunctionAndExpiredWeakPtrEdgeCases)
{
    CppEvents::Event<int> ev;

    // Empty std::function
    CppEvents::Event<int>::Handler empty_fn;
    EXPECT_EQ((ev += empty_fn), CppEvents::Event<int>::InvalidConnectionId);

    // Null object pointer
    DerivedListener* null_obj = nullptr;
    EXPECT_EQ((ev += std::pair(null_obj, &DerivedListener::onMutable)),
              CppEvents::Event<int>::InvalidConnectionId);

    // Null method pointer
    DerivedListener valid_obj;
    void (DerivedListener::*null_method)(int) = nullptr;
    EXPECT_EQ((ev += std::pair(&valid_obj, null_method)),
              CppEvents::Event<int>::InvalidConnectionId);

    // Already-expired std::weak_ptr
    std::weak_ptr<DerivedListener> expired_weak;
    EXPECT_EQ((ev += std::pair(expired_weak, &DerivedListener::onMutable)),
              CppEvents::Event<int>::InvalidConnectionId);

    // Valid weak_ptr with null method pointer
    auto valid_shared = std::make_shared<DerivedListener>();
    std::weak_ptr<DerivedListener> valid_weak = valid_shared;
    EXPECT_EQ((ev += std::pair(valid_weak, null_method)),
              CppEvents::Event<int>::InvalidConnectionId);

    // Null std::shared_ptr or null method
    std::shared_ptr<DerivedListener> null_shared;
    EXPECT_EQ((ev += std::pair(null_shared, &DerivedListener::onMutable)),
              CppEvents::Event<int>::InvalidConnectionId);
    EXPECT_EQ((ev += std::pair(valid_shared, null_method)),
              CppEvents::Event<int>::InvalidConnectionId);

    // subscribeScoped with empty callable
    auto scoped = ev.subscribeScoped(empty_fn);
    EXPECT_FALSE(scoped.isConnected());
    EXPECT_FALSE(static_cast<bool>(scoped));
    EXPECT_EQ(scoped.id(), CppEvents::Event<int>::InvalidConnectionId);

    EXPECT_TRUE(ev.empty());
    EXPECT_NO_THROW(ev.notify(100));
}

TEST(EventTest, UnsubscribeEdgeCasesInvalidIdUnknownIdDoubleUnsubscribeAndClear)
{
    CppEvents::Event<int> ev;

    // Unsubscribing on an empty event
    ev -= CppEvents::Event<int>::InvalidConnectionId;
    ev -= 99999;
    ev.clear();
    EXPECT_TRUE(ev.empty());

    int a = 0;
    int b = 0;
    int c = 0;
    auto id_a = ev += [&a](int v) { a += v; };
    auto id_b = ev += [&b](int v) { b += v; };
    auto id_c = ev += [&c](int v) { c += v; };
    EXPECT_LT(id_a, id_b);
    EXPECT_LT(id_b, id_c);

    // Remove middle listener & double remove (must be no-op)
    ev -= id_b;
    ev -= id_b;
    EXPECT_EQ(ev.listenerCount(), 2u);

    ev.notify(5);
    EXPECT_EQ(a, 5);
    EXPECT_EQ(b, 0);
    EXPECT_EQ(c, 5);

    // Clear all remaining
    ev.clear();
    EXPECT_TRUE(ev.empty());
    ev.notify(5);
    EXPECT_EQ(a, 5);
    EXPECT_EQ(c, 5);

    // Subscribe again after clear()
    auto id_d = ev += [&b](int v) { b += v; };
    EXPECT_GT(id_d, id_c);
    ev.notify(7);
    EXPECT_EQ(b, 7);
}

TEST(EventTest, ReentrancySelfUnsubscribeAndPeerUnsubscribeDuringNotify)
{
    CppEvents::Event<> ev;
    int calls_a = 0;
    int calls_b = 0;
    int calls_c = 0;
    CppEvents::Event<>::ConnectionId id_a = 0;
    CppEvents::Event<>::ConnectionId id_b = 0;

    id_a = ev += [&]()
    {
        ++calls_a;
        ev -= id_a; // Self-unsubscribe
        ev -= id_b; // Unsubscribe B before B is reached in snapshot
    };
    id_b = ev += [&]() { ++calls_b; };
    ev += [&]() { ++calls_c; };

    ev.notify();
    EXPECT_EQ(calls_a, 1);
    EXPECT_EQ(calls_b, 0); // B was unsubscribed before its turn
    EXPECT_EQ(calls_c, 1);
    EXPECT_EQ(ev.listenerCount(), 1u);

    ev.notify();
    EXPECT_EQ(calls_a, 1);
    EXPECT_EQ(calls_b, 0);
    EXPECT_EQ(calls_c, 2);
}

TEST(EventTest, ReentrancySubscribeClearAndNestedNotify)
{
    // 1. Dynamically subscribing a new listener during notify()
    {
        CppEvents::Event<> ev;
        int first_calls = 0;
        int dynamically_added_calls = 0;

        ev += [&]()
        {
            ++first_calls;
            if (first_calls == 1)
            {
                ev += [&]() { ++dynamically_added_calls; };
            }
        };

        ev.notify();
        EXPECT_EQ(first_calls, 1);
        EXPECT_EQ(dynamically_added_calls, 0);

        ev.notify();
        EXPECT_EQ(first_calls, 2);
        EXPECT_EQ(dynamically_added_calls, 1);
    }

    // 2. Calling clear() inside a listener during notify()
    {
        CppEvents::Event<> ev;
        int calls = 0;
        ev += [&]()
        {
            ++calls;
            ev.clear();
        };
        ev += [&]() { ++calls; };

        ev.notify();
        EXPECT_EQ(calls, 1);
        EXPECT_TRUE(ev.empty());
    }

    // 3. Recursive / nested notify() inside a listener
    {
        CppEvents::Event<int> ev;
        int total = 0;
        ev += [&](int depth)
        {
            total += depth;
            if (depth > 1)
            {
                ev.notify(depth - 1);
            }
        };

        ev.notify(3);
        EXPECT_EQ(total, 6); // 3 + 2 + 1
    }
}

TEST(ScopedConnectionTest, RaiiLifecycleMoveSemanticsReleaseAndReverseDestructionOrder)
{
    CppEvents::Event<int> ev;
    int sum_a = 0;
    int sum_b = 0;

    {
        CppEvents::ScopedConnection default_conn;
        EXPECT_FALSE(default_conn.isConnected());
        EXPECT_EQ(default_conn.id(), 0u);
        default_conn.disconnect();

        auto conn_a = ev.subscribeScoped([&sum_a](int v) { sum_a += v; });
        auto conn_b = ev.subscribeScoped([&sum_b](int v) { sum_b += v; });
        EXPECT_EQ(ev.listenerCount(), 2u);

        // Move construction
        CppEvents::ScopedConnection moved_a(std::move(conn_a));
        EXPECT_FALSE(conn_a.isConnected());
        EXPECT_TRUE(moved_a.isConnected());

        // Move assignment onto active ScopedConnection disconnects its previous target (conn_b)
        conn_b = std::move(moved_a);
        EXPECT_FALSE(moved_a.isConnected());
        EXPECT_TRUE(conn_b.isConnected());
        EXPECT_EQ(ev.listenerCount(), 1u);

        ev.notify(10);
        EXPECT_EQ(sum_a, 10);
        EXPECT_EQ(sum_b, 0);
    }
    // conn_b went out of scope -> automatically disconnected
    EXPECT_TRUE(ev.empty());

    // release() detaches RAII without unsubscribing
    CppEvents::Event<int>::ConnectionId raw_id = 0;
    {
        auto conn = ev.subscribeScoped([&sum_a](int v) { sum_a += v; });
        raw_id = conn.release();
        EXPECT_FALSE(conn.isConnected());
        EXPECT_NE(raw_id, 0u);
    }
    EXPECT_EQ(ev.listenerCount(), 1u);
    ev.notify(5);
    EXPECT_EQ(sum_a, 15);
    ev -= raw_id;
    EXPECT_TRUE(ev.empty());

    // Reverse destruction order: Event destroyed BEFORE ScopedConnection
    CppEvents::ScopedConnection outliving_conn;
    {
        CppEvents::Event<int> temporary_event;
        outliving_conn = temporary_event.subscribeScoped([](int) {});
        EXPECT_TRUE(outliving_conn.isConnected());
    }
    EXPECT_NO_THROW(outliving_conn.disconnect());
    EXPECT_FALSE(outliving_conn.isConnected());
}

TEST(EventTest, ExceptionSafetyDoesNotDeadlockOrCorruptState)
{
    CppEvents::Event<int> ev;
    int before_throw = 0;
    int after_throw = 0;

    ev += [&before_throw](int v) { before_throw += v; };
    auto throw_id = ev += [](int) { throw std::runtime_error("listener error"); };
    ev += [&after_throw](int v) { after_throw += v; };

    EXPECT_THROW(ev.notify(5), std::runtime_error);
    EXPECT_EQ(before_throw, 5);
    EXPECT_EQ(after_throw, 0);

    // Event mutex must NOT remain locked after exception
    ev -= throw_id;
    EXPECT_NO_THROW(ev.notify(3));
    EXPECT_EQ(before_throw, 8);
    EXPECT_EQ(after_throw, 3);
}

TEST(EventTest, MultithreadedStressTestConcurrentSubscribeUnsubscribeAndNotify)
{
    CppEvents::Event<int> ev;
    std::atomic<int> total_notifications{0};
    std::atomic<bool> stop{false};

    auto notifier_fn = [&]()
    {
        for (int i = 0; i < 1500; ++i)
        {
            ev.notify(1);
        }
    };

    auto churn_fn = [&]()
    {
        while (!stop.load(std::memory_order_relaxed))
        {
            auto scoped = ev.subscribeScoped([&total_notifications](int v)
            {
                total_notifications.fetch_add(v, std::memory_order_relaxed);
            });
            auto raw_id = ev += [&total_notifications](int v)
            {
                total_notifications.fetch_add(v, std::memory_order_relaxed);
            };
            ev -= raw_id;
        }
    };

    std::thread n1(notifier_fn);
    std::thread n2(notifier_fn);
    std::thread c1(churn_fn);
    std::thread c2(churn_fn);

    n1.join();
    n2.join();
    stop.store(true, std::memory_order_relaxed);
    c1.join();
    c2.join();

    EXPECT_TRUE(ev.empty());
    EXPECT_GE(total_notifications.load(), 0);
}
