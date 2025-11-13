#ifndef EVENTBUSCPP_RAILWAYSYSTEM_HPP
#define EVENTBUSCPP_RAILWAYSYSTEM_HPP

#include <iostream>

#include "EventBus.hpp"

class RailwaySystem
{
public:
    explicit RailwaySystem(EventBus& bus) : bus(bus) {}

    void arriveTrain(const std::string& name) const
    {
        std::cout << "[RailwaySystem] " << name << " arrived." << std::endl;
        bus.trainArrived.notify(name);
    }

    void departTrain(const std::string& name) const
    {
        std::cout << "[RailwaySystem] " << name << " departed." << std::endl;
        bus.trainDeparted.notify(name);
    }

    void boardPassenger(int id, const std::string& name) const
    {
        std::cout << "[RailwaySystem] Passenger " << id << " boarded in train " << name << "." << std::endl;
        bus.passengerBoarded.notify(id, name);
    }

private:
    EventBus& bus;
};

#endif //EVENTBUSCPP_RAILWAYSYSTEM_HPP