#include <Arduino.h>
#include "tracker.h"

#define TRACKER_ID 7

void setup()
{
    Tracker tracker{TRACKER_ID};
    tracker.run();
}

void loop()
{
    ;
}