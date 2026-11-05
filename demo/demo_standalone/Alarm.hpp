#ifndef CPPEVENTS_DEMO_ALARM_HPP
#define CPPEVENTS_DEMO_ALARM_HPP

#include <string>

#include "Event.hpp"

class Alarm
{
public:
    CppEvents::Event<std::string> triggered;

    void trigger(const std::string& message) const
    {
        triggered.notify(message);
    }
};

#endif // CPPEVENTS_DEMO_ALARM_HPP
