#ifndef CPPEVENTS_DEMO_TICKETSYSTEM_HPP
#define CPPEVENTS_DEMO_TICKETSYSTEM_HPP

#include <iostream>
#include <string>

#include "EventBus.hpp"

class TicketSystem
{
public:
    explicit TicketSystem(EventBus& bus)
        : bus_(bus)
    {
    }

    void boardPassenger(int id, const std::string& name) const
    {
        std::cout << "[TicketSystem] Passenger " << id << " boarded in train " << name << ".\n";
        bus_.passengerBoarded.notify(id, name);
    }

private:
    EventBus& bus_;
};

#endif // CPPEVENTS_DEMO_TICKETSYSTEM_HPP
