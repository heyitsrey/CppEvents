#ifndef CPPEVENTS_DEMO_EVENTBUS_HPP
#define CPPEVENTS_DEMO_EVENTBUS_HPP

#include <string>

#include "Event.hpp"

struct EventBus
{
    CppEvents::Event<std::string> trainArrived;
    CppEvents::Event<std::string> trainDeparted;
    CppEvents::Event<int, std::string> passengerBoarded;
};

#endif // CPPEVENTS_DEMO_EVENTBUS_HPP
