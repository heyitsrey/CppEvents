#include "Alarm.hpp"
#include "AlarmListener.hpp"

int main()
{
    Alarm alarm;
    AlarmListener listener(alarm);

    alarm.trigger("Intruder detected");

    return 0;
}
