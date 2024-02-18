#include <Arduino.h>
#include "tracker.h"


void setup()
{
    pinMode(BOARD_POWER_ON_PIN, OUTPUT);
    digitalWrite(BOARD_POWER_ON_PIN, HIGH);

    pinMode(BOARD_RST_PIN, OUTPUT);
    digitalWrite(BOARD_RST_PIN, LOW);

    Tracker tracker;

    tracker.run();
}

void loop()
{
    ;
}