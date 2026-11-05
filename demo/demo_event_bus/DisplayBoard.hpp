#ifndef CPPEVENTS_DEMO_DISPLAYBOARD_HPP
#define CPPEVENTS_DEMO_DISPLAYBOARD_HPP

#include <iostream>
#include <string>

#include "EventBus.hpp"

class DisplayBoard
{
public:
    explicit DisplayBoard(EventBus& bus)
        : bus_(bus)
    {
        arrivedId_  = bus_.trainArrived     += std::pair(this, &DisplayBoard::showTrainArrived);
        departedId_ = bus_.trainDeparted    += std::pair(this, &DisplayBoard::showTrainDeparted);
        boardedId_  = bus_.passengerBoarded += std::pair(this, &DisplayBoard::showPassengerBoarded);
    }

    ~DisplayBoard()
    {
        bus_.trainArrived     -= arrivedId_;
        bus_.trainDeparted    -= departedId_;
        bus_.passengerBoarded -= boardedId_;
    }

    DisplayBoard(const DisplayBoard&) = delete;
    DisplayBoard& operator=(const DisplayBoard&) = delete;

    void showTrainArrived(const std::string& name) const
    {
        std::cout << "[DisplayBoard] Arrival: " << name << "\n";
    }

    void showTrainDeparted(const std::string& name) const
    {
        std::cout << "[DisplayBoard] Departure: " << name << "\n";
    }

    void showPassengerBoarded(int id, const std::string& name) const
    {
        std::cout << "[DisplayBoard] Passenger " << id << " in " << name << "\n";
    }

private:
    EventBus& bus_;
    CppEvents::Event<std::string>::ConnectionId arrivedId_{0};
    CppEvents::Event<std::string>::ConnectionId departedId_{0};
    CppEvents::Event<int, std::string>::ConnectionId boardedId_{0};
};

#endif // CPPEVENTS_DEMO_DISPLAYBOARD_HPP
