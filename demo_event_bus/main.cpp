#include "Logger.hpp"
#include "RailwaySystem.hpp"
#include "DisplayBoard.hpp"
#include "TicketSystem.hpp"

int main()
{
    EventBus bus;

    // Subscriber
    Logger logger(bus);
    DisplayBoard display(bus);

    // Publisher
    RailwaySystem railway(bus);
    TicketSystem ticket(bus);

    // Trigger Events
    railway.arriveTrain("Train 794");
    ticket.boardPassenger(42, "Train 794");
    railway.departTrain("Train 794");

    return 0;
}
