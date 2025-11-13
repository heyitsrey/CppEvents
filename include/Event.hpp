#ifndef EVENTBUSCPP_EVENT_HPP
#define EVENTBUSCPP_EVENT_HPP

#include <algorithm>
#include <functional>
#include <vector>

namespace CppEvents
{
    template<typename... Args>
    class Event
    {
    public:
        using Handler = std::function<void(Args...)>;

        // Lambdas or free functions
        void operator+=(Handler handler)
        {
            listeners.push_back(std::move(handler));
        }

        // Memberfunctions and instance in one line
        template<typename T, typename... FArgs>
        void operator+=(std::pair<T*, void (T::*)(FArgs...)> bound) {
            T* obj;
            void (T::*method)(FArgs...);
            std::tie(obj, method) = bound;

            listeners.push_back([obj, method](Args... args)
            {
                (obj->*method)(args...);
            });
        }

        void notify(Args... args)
        {
            for (auto& handler : listeners)
                handler(args...);
        }


    private:
        std::vector<Handler> listeners;
    };
}

#endif //EVENTBUSCPP_EVENT_HPP