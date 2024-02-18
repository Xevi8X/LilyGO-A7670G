#include "gnss_L76K.h"
#include "../config.h"

void GNSS_L76K::init()
{
    pinMode(BOARD_GPS_WAKEUP_PIN, OUTPUT);
    digitalWrite(BOARD_GPS_WAKEUP_PIN, HIGH);
    GNSS_NMEA::init();
}

void GNSS_L76K::turn_off() 
{
    pinMode(BOARD_GPS_WAKEUP_PIN, OUTPUT);
    digitalWrite(BOARD_GPS_WAKEUP_PIN, LOW);
}
