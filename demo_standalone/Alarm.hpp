#ifndef EVENTBUSCPP_ALARM_HPP
#define EVENTBUSCPP_ALARM_HPP

#include <string>

#include "Event.hpp"

class Alarm
{
public:
    CppEvents::Event<const std::string&> triggered;

    void trigger(const std::string& message)
    {
        triggered.notify(message);
    }
};

#endif //EVENTBUSCPP_ALARM_HPP