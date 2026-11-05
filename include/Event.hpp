#ifndef CPPEVENTS_EVENT_HPP
#define CPPEVENTS_EVENT_HPP

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <functional>
#include <memory>
#include <mutex>
#include <type_traits>
#include <utility>
#include <vector>

namespace CppEvents
{

    /**
     * @brief RAII wrapper that automatically disconnects a listener when destroyed.
     *
     * Safe to destroy even after the originating Event has already been destroyed.
     */
    class ScopedConnection
    {
    public:
        using ConnectionId = std::size_t;

        ScopedConnection() noexcept = default;

        ScopedConnection(ConnectionId id, std::function<void()> disconnect_fn) noexcept
            : id_(id)
            , disconnect_fn_(std::move(disconnect_fn))
        {
        }

        ~ScopedConnection() noexcept
        {
            disconnect();
        }

        ScopedConnection(const ScopedConnection&) = delete;
        ScopedConnection& operator=(const ScopedConnection&) = delete;

        ScopedConnection(ScopedConnection&& other) noexcept
            : id_(std::exchange(other.id_, 0))
            , disconnect_fn_(std::move(other.disconnect_fn_))
        {
        }

        ScopedConnection& operator=(ScopedConnection&& other) noexcept
        {
            if (this != &other)
            {
                disconnect();
                id_ = std::exchange(other.id_, 0);
                disconnect_fn_ = std::move(other.disconnect_fn_);
            }
            return *this;
        }

        /**
         * @brief Manually disconnects the listener from the event.
         */
        void disconnect() noexcept
        {
            if (disconnect_fn_)
            {
                auto fn = std::move(disconnect_fn_);
                disconnect_fn_ = nullptr;
                id_ = 0;
                fn();
            }
        }

        /**
         * @brief Detaches the connection from RAII management without unsubscribing.
         * @return The underlying ConnectionId.
         */
        ConnectionId release() noexcept
        {
            disconnect_fn_ = nullptr;
            return std::exchange(id_, 0);
        }

        [[nodiscard]] ConnectionId id() const noexcept
        {
            return id_;
        }

        [[nodiscard]] bool isConnected() const noexcept
        {
            return id_ != 0 && static_cast<bool>(disconnect_fn_);
        }

        explicit operator bool() const noexcept
        {
            return isConnected();
        }

    private:
        ConnectionId id_{0};
        std::function<void()> disconnect_fn_;
    };

    /**
     * @brief Thread-safe, C#-inspired event template supporting lambdas, free functions,
     *        and member functions.
     *
     * @tparam Args Argument types passed to event listeners upon notification.
     */
    template <typename... Args>
    class Event
    {
    public:
        using Handler = std::function<void(const Args&...)>;
        using ConnectionId = std::size_t;

        static constexpr ConnectionId InvalidConnectionId = 0;

        Event()
            : state_(std::make_shared<State>())
        {
        }

        ~Event() = default;

        Event(const Event&) = delete;
        Event& operator=(const Event&) = delete;
        Event(Event&&) = delete;
        Event& operator=(Event&&) = delete;

        /**
         * @brief Subscribes a callable (lambda, free function, or std::function).
         * @return Unique ConnectionId (> 0), or InvalidConnectionId (0) if the handler is empty.
         */
        ConnectionId operator+=(Handler handler)
        {
            if (!handler)
            {
                return InvalidConnectionId;
            }

            std::lock_guard<std::mutex> lock(state_->mutex);
            const ConnectionId id = ++state_->next_id;
            state_->listeners.push_back(std::make_shared<ListenerEntry>(id, std::move(handler)));
            return id;
        }

        /**
         * @brief Subscribes a member function (const or non-const) via std::pair(this, &Class::method).
         */
        template <typename T, typename Method,
                  typename = std::enable_if_t<std::is_member_function_pointer_v<Method>>>
        ConnectionId operator+=(std::pair<T*, Method> bound)
        {
            T* obj = bound.first;
            Method method = bound.second;
            if (!obj || !method)
            {
                return InvalidConnectionId;
            }

            return (*this) += [obj, method](const Args&... args)
            {
                (obj->*method)(args...);
            };
        }

        /**
         * @brief Subscribes a member function bound to a std::weak_ptr.
         *        If the target object has expired when the event fires, the call is safely skipped.
         */
        template <typename T, typename Method,
                  typename = std::enable_if_t<std::is_member_function_pointer_v<Method>>>
        ConnectionId operator+=(std::pair<std::weak_ptr<T>, Method> bound)
        {
            Method method = bound.second;
            if (bound.first.expired() || !method)
            {
                return InvalidConnectionId;
            }

            return (*this) += [weak_obj = std::move(bound.first), method](const Args&... args)
            {
                if (auto shared_obj = weak_obj.lock())
                {
                    (shared_obj.get()->*method)(args...);
                }
            };
        }

        /**
         * @brief Subscribes a member function bound to a std::shared_ptr.
         */
        template <typename T, typename Method,
                  typename = std::enable_if_t<std::is_member_function_pointer_v<Method>>>
        ConnectionId operator+=(std::pair<std::shared_ptr<T>, Method> bound)
        {
            if (!bound.first || !bound.second)
            {
                return InvalidConnectionId;
            }

            Method method = bound.second;
            return (*this) += [shared_obj = std::move(bound.first), method](const Args&... args)
            {
                (shared_obj.get()->*method)(args...);
            };
        }

        /**
         * @brief Subscribes a listener and returns an RAII ScopedConnection that automatically
         *        unsubscribes when destroyed.
         */
        template <typename Callable>
        [[nodiscard]] ScopedConnection subscribeScoped(Callable&& callable)
        {
            const ConnectionId id = (*this) += std::forward<Callable>(callable);
            if (id == InvalidConnectionId)
            {
                return ScopedConnection{};
            }

            std::weak_ptr<State> weak_state = state_;
            return ScopedConnection(id, [weak_state, id]() noexcept
            {
                if (auto state = weak_state.lock())
                {
                    state->remove(id);
                }
            });
        }

        /**
         * @brief Unsubscribes a listener by its ConnectionId (C#-style -= operator).
         */
        void operator-=(ConnectionId id) noexcept
        {
            state_->remove(id);
        }

        /**
         * @brief Unsubscribes a listener by its ConnectionId.
         */
        void unsubscribe(ConnectionId id) noexcept
        {
            state_->remove(id);
        }

        /**
         * @brief Removes all registered listeners from this event.
         */
        void clear() noexcept
        {
            std::lock_guard<std::mutex> lock(state_->mutex);
            for (const auto& entry : state_->listeners)
            {
                entry->active.store(false, std::memory_order_release);
            }
            state_->listeners.clear();
        }

        /**
         * @brief Invokes all registered listeners safely.
         *
         * Takes a lightweight snapshot of active listener entries under the lock and invokes
         * handlers outside the lock to prevent deadlocks and allow re-entrant subscriptions
         * or unsubscriptions during notification.
         */
        void notify(const Args&... args) const
        {
            std::vector<std::shared_ptr<ListenerEntry>> snapshot;
            {
                std::lock_guard<std::mutex> lock(state_->mutex);
                snapshot = state_->listeners;
            }

            for (const auto& entry : snapshot)
            {
                if (entry->active.load(std::memory_order_acquire))
                {
                    entry->handler(args...);
                }
            }
        }

        /**
         * @brief Convenience call operator equivalent to notify().
         */
        void operator()(const Args&... args) const
        {
            notify(args...);
        }

        /**
         * @brief Returns the number of currently registered listeners.
         */
        [[nodiscard]] std::size_t listenerCount() const noexcept
        {
            std::lock_guard<std::mutex> lock(state_->mutex);
            return state_->listeners.size();
        }

        /**
         * @brief Checks whether the event has no registered listeners.
         */
        [[nodiscard]] bool empty() const noexcept
        {
            std::lock_guard<std::mutex> lock(state_->mutex);
            return state_->listeners.empty();
        }

    private:
        struct ListenerEntry
        {
            ConnectionId id;
            Handler handler;
            std::atomic<bool> active{true};

            ListenerEntry(ConnectionId connection_id, Handler fn)
                : id(connection_id)
                , handler(std::move(fn))
                , active(true)
            {
            }
        };

        struct State
        {
            mutable std::mutex mutex;
            ConnectionId next_id{0};
            std::vector<std::shared_ptr<ListenerEntry>> listeners;

            void remove(ConnectionId id) noexcept
            {
                if (id == InvalidConnectionId)
                {
                    return;
                }

                std::lock_guard<std::mutex> lock(mutex);
                listeners.erase(
                    std::remove_if(listeners.begin(), listeners.end(),
                                   [id](const std::shared_ptr<ListenerEntry>& entry)
                                   {
                                       if (entry->id == id)
                                       {
                                           entry->active.store(false, std::memory_order_release);
                                           return true;
                                       }
                                       return false;
                                   }),
                    listeners.end());
            }
        };

        std::shared_ptr<State> state_;
    };

} // namespace CppEvents

#endif // CPPEVENTS_EVENT_HPP
