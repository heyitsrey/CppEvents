#ifndef CPPEVENTS_DEMO_RAILWAYSYSTEM_HPP
#define CPPEVENTS_DEMO_RAILWAYSYSTEM_HPP

#include <iostream>
#include <string>

#include "EventBus.hpp"

class RailwaySystem
{
public:
    explicit RailwaySystem(EventBus& bus)
        : bus_(bus)
    {
    }

    void arriveTrain(const std::string& name) const
    {
        std::cout << "[RailwaySystem] " << name << " arrived.\n";
        bus_.trainArrived.notify(name);
    }

    void departTrain(const std::string& name) const
    {
        std::cout << "[RailwaySystem] " << name << " departed.\n";
        bus_.trainDeparted.notify(name);
    }

    void boardPassenger(int id, const std::string& name) const
    {
        std::cout << "[RailwaySystem] Passenger " << id << " boarded in train " << name << ".\n";
        bus_.passengerBoarded.notify(id, name);
    }

private:
    EventBus& bus_;
};

#endif // CPPEVENTS_DEMO_RAILWAYSYSTEM_HPP
