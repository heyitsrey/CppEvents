#ifndef EVENTBUSCPP_EVENTBUS_HPP
#define EVENTBUSCPP_EVENTBUS_HPP

#include <string>

#include "Event.hpp"

struct EventBus
{
    Event<std::string> trainArrived;
    Event<std::string> trainDeparted;
    Event<int, std::string> passengerBoarded;
};

#endif //EVENTBUSCPP_EVENTBUS_HPP