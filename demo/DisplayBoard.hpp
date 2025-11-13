#ifndef EVENTBUSCPP_DISPLAYBOARD_HPP
#define EVENTBUSCPP_DISPLAYBOARD_HPP

#include <iostream>

#include "EventBus.hpp"

class DisplayBoard
{
public:
    explicit DisplayBoard(EventBus& bus)
    {
        bus.trainArrived      += std::pair(this, &DisplayBoard::showTrainArrived);
        bus.trainDeparted     += std::pair(this, &DisplayBoard::showTrainDeparted);
        bus.passengerBoarded  += std::pair(this, &DisplayBoard::showPassengerBoarded);
    }

    void showTrainArrived(const std::string& name)
    {
        std::cout << "[DisplayBoard] Arrival: " << name << "\n";
    }

    void showTrainDeparted(const std::string& name)
    {
        std::cout << "[DisplayBoard] Departure: " << name << "\n";
    }

    void showPassengerBoarded(int id, const std::string& name)
    {
        std::cout << "[DisplayBoard] Passenger " << id << " in " << name << "\n";
    }
};

#endif //EVENTBUSCPP_DISPLAYBOARD_HPP