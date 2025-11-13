#ifndef EVENTBUSCPP_ALARMLISTENER_HPP
#define EVENTBUSCPP_ALARMLISTENER_HPP

#include <iostream>

#include "Alarm.hpp"
#include "Event.hpp"

class AlarmListener
{
public:
    explicit AlarmListener(Alarm& alarm)
    {
        alarm.triggered += std::pair(this, &AlarmListener::onAlarm);
    }

    void onAlarm(const std::string& msg)
    {
        std::cout << "[AlarmListener] Alarm: " << msg << "\n";
    }
};

#endif //EVENTBUSCPP_ALARMLISTENER_HPP