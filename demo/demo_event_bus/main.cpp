#include "DisplayBoard.hpp"
#include "Logger.hpp"
#include "RailwaySystem.hpp"
#include "TicketSystem.hpp"

int main()
{
    EventBus bus;

    // Subscribers
    Logger logger(bus);
    DisplayBoard display(bus);

    // Publishers
    RailwaySystem railway(bus);
    TicketSystem ticket(bus);

    // Trigger events
    railway.arriveTrain("Train 794");
    ticket.boardPassenger(42, "Train 794");
    railway.departTrain("Train 794");

    return 0;
}
