#ifndef CPPEVENTS_DEMO_ALARMLISTENER_HPP
#define CPPEVENTS_DEMO_ALARMLISTENER_HPP

#include <iostream>
#include <string>

#include "Alarm.hpp"

class AlarmListener
{
public:
    explicit AlarmListener(Alarm& alarm)
        : alarm_(alarm)
    {
        connectionId_ = alarm_.triggered += std::pair(this, &AlarmListener::onAlarm);
    }

    ~AlarmListener()
    {
        alarm_.triggered -= connectionId_;
    }

    AlarmListener(const AlarmListener&) = delete;
    AlarmListener& operator=(const AlarmListener&) = delete;

    void onAlarm(const std::string& msg) const
    {
        std::cout << "[AlarmListener] Alarm: " << msg << "\n";
    }

private:
    Alarm& alarm_;
    CppEvents::Event<std::string>::ConnectionId connectionId_{0};
};

#endif // CPPEVENTS_DEMO_ALARMLISTENER_HPP
