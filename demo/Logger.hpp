#ifndef EVENTBUSCPP_LOGGER_HPP
#define EVENTBUSCPP_LOGGER_HPP

#include <iostream>

#include "EventBus.hpp"

class Logger {
public:
    Logger(EventBus& bus)
    {
        bus.trainArrived += std::pair(this, &Logger::onTrainArrived);
        bus.trainDeparted   += std::pair(this, &Logger::onTrainDeparted);
        bus.passengerBoarded += std::pair(this, &Logger::onPassengerBoarded);
    }

    void onTrainArrived(const std::string& name) {
        std::cout << "[Logger] " << name << " arrived." << std::endl;
    }

    void onTrainDeparted(const std::string& name) {
        std::cout << "[Logger] " << name << " departed." << std::endl;
    }

    void onPassengerBoarded(int id, const std::string& name) {
        std::cout << "[Logger] Passenger " << id << " boarded in train " << name << "." << std::endl;
    }
};

#endif //EVENTBUSCPP_LOGGER_HPP