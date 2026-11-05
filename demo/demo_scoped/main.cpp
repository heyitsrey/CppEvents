#include <iostream>
#include <string>
#include <vector>

#include "Event.hpp"

class WeatherStation
{
public:
    CppEvents::Event<double> temperatureChanged;

    void setTemperature(double celsius) const
    {
        std::cout << "\n[WeatherStation] New temperature: " << celsius << " C\n";
        temperatureChanged.notify(celsius);
    }
};

class SmartThermostat
{
public:
    explicit SmartThermostat(WeatherStation& station, std::string roomName)
        : roomName_(std::move(roomName))
    {
        // Subscribe using ScopedConnection:
        // No need to store a reference to WeatherStation or write a destructor!
        connection_ = station.temperatureChanged.subscribeScoped(
            std::pair(this, &SmartThermostat::onTemperatureChanged));

        std::cout << "[SmartThermostat] '" << roomName_ << "' connected.\n";
    }

    ~SmartThermostat()
    {
        std::cout << "[SmartThermostat] '" << roomName_ << "' destroyed (auto-disconnected).\n";
    }

    SmartThermostat(const SmartThermostat&) = delete;
    SmartThermostat& operator=(const SmartThermostat&) = delete;

    void onTemperatureChanged(double celsius) const
    {
        std::cout << "  -> [" << roomName_ << "] adjusting heating for " << celsius << " C\n";
    }

private:
    std::string roomName_;
    CppEvents::ScopedConnection connection_;
};

int main()
{
    WeatherStation station;

    // Living room thermostat lives for the entire program
    SmartThermostat livingRoom(station, "Living Room");

    {
        // Guest room thermostat only exists inside this inner scope
        SmartThermostat guestRoom(station, "Guest Room");

        station.setTemperature(21.5);
    } // guestRoom goes out of scope here and automatically unsubscribes!

    // Only livingRoom receives this update
    station.setTemperature(19.0);

    return 0;
}
