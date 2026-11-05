#ifndef CPPEVENTS_DEMO_LOGGER_HPP
#define CPPEVENTS_DEMO_LOGGER_HPP

#include <iostream>
#include <string>

#include "EventBus.hpp"

class Logger
{
public:
    explicit Logger(EventBus& bus)
        : bus_(bus)
    {
        arrivedId_  = bus_.trainArrived     += std::pair(this, &Logger::onTrainArrived);
        departedId_ = bus_.trainDeparted    += std::pair(this, &Logger::onTrainDeparted);
        boardedId_  = bus_.passengerBoarded += std::pair(this, &Logger::onPassengerBoarded);
    }

    ~Logger()
    {
        bus_.trainArrived     -= arrivedId_;
        bus_.trainDeparted    -= departedId_;
        bus_.passengerBoarded -= boardedId_;
    }

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void onTrainArrived(const std::string& name) const
    {
        std::cout << "[Logger] " << name << " arrived.\n";
    }

    void onTrainDeparted(const std::string& name) const
    {
        std::cout << "[Logger] " << name << " departed.\n";
    }

    void onPassengerBoarded(int id, const std::string& name) const
    {
        std::cout << "[Logger] Passenger " << id << " boarded in train " << name << ".\n";
    }

private:
    EventBus& bus_;
    CppEvents::Event<std::string>::ConnectionId arrivedId_{0};
    CppEvents::Event<std::string>::ConnectionId departedId_{0};
    CppEvents::Event<int, std::string>::ConnectionId boardedId_{0};
};

#endif // CPPEVENTS_DEMO_LOGGER_HPP
