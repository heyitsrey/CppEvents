#include "AlarmListener.hpp"

int main()
{
    Alarm alarm;
    AlarmListener listener(alarm);

    alarm.trigger("Intruder detected");
}
