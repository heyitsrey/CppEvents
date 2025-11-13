#ifndef EVENTBUSCPP_EVENTBUS_HPP
#define EVENTBUSCPP_EVENTBUS_HPP

#include <string>

#include "Event.hpp"

struct EventBus
{
    CppEvents::Event<std::string> trainArrived;
    CppEvents::Event<std::string> trainDeparted;
    CppEvents::Event<int, std::string> passengerBoarded;
};

#endif //EVENTBUSCPP_EVENTBUS_HPP