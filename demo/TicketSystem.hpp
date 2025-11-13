#ifndef EVENTBUSCPP_TICKETSYSTEM_HPP
#define EVENTBUSCPP_TICKETSYSTEM_HPP

#include <iostream>

#include "EventBus.hpp"

class TicketSystem
{
public:
    explicit TicketSystem(EventBus& bus) : bus(bus) {}

    void boardPassenger(int id, const std::string& name) const
    {
        std::cout << "[TicketSystem] Passenger " << id << " boarded in train " << name << "." << std::endl;
        bus.passengerBoarded.notify(id, name);
    }

private:
    EventBus& bus;
};

#endif //EVENTBUSCPP_TICKETSYSTEM_HPP