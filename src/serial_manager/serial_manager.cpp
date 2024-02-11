#include "serial_manager.h"
#include <Arduino.h>

Stream &SerialManager::get_modem_serial()
{
    static bool initialized = false;

    if(!initialized)
    {
        SerialAT.begin(115200, SERIAL_8N1, BOARD_MODEM_RX_PIN, BOARD_MODEM_TX_PIN);
        initialized = true;
    }
    return SerialAT;
}

Stream &SerialManager::get_gnss_serial()
{
    static bool initialized = false;

    if(!initialized)
    {
        SerialGPS.begin(9600, SERIAL_8N1, BOARD_GPS_RX_PIN, BOARD_GPS_TX_PIN);
        initialized = true;
    }
    return SerialGPS;
}

Stream &SerialManager::get_monitor_serial()
{
    static bool initialized = false;

    if(!initialized)
    {
        Serial.begin(115200);
        initialized = true;
    }
    return Serial;
}
