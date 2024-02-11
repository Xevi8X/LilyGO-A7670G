#include "config.h"
#include <Arduino.h>
#include <TinyGPS++.h>
#include "modem/modem.h"

const char server[] = "https://webhook.site/37cbb902-5503-41d9-8206-f96e92419be6";

TinyGPSPlus gps;
Modem my_modem;

void setup()
{
    pinMode(BOARD_POWER_ON_PIN, OUTPUT);
    digitalWrite(BOARD_POWER_ON_PIN, HIGH);

    pinMode(BOARD_RST_PIN, OUTPUT);
    digitalWrite(BOARD_RST_PIN, LOW);

    pinMode(BOARD_GPS_WAKEUP_PIN, OUTPUT);
    digitalWrite(BOARD_GPS_WAKEUP_PIN, HIGH);

    pinMode(BOARD_MODEM_PWR_PIN, OUTPUT);
    digitalWrite(BOARD_MODEM_PWR_PIN, LOW);
    delay(100);
    digitalWrite(BOARD_MODEM_PWR_PIN, HIGH);
    delay(1000);
    digitalWrite(BOARD_MODEM_PWR_PIN, LOW);

    Serial.begin(115200);
    //Modem Serial port
    SerialAT.begin(115200, SERIAL_8N1, BOARD_MODEM_RX_PIN, BOARD_MODEM_TX_PIN);
    //GPS Serial port
    SerialGPS.begin(9600, SERIAL_8N1, BOARD_GPS_RX_PIN, BOARD_GPS_TX_PIN);

    delay(2000);

    my_modem.init();
}

void loop()
{

    // Read waiting bytes on GPS UART
    while (SerialGPS.available()) {
        int c = SerialGPS.read();
        if (gps.encode(c)) 
        {
            
        }
    }

    my_modem.https_post(server, "DUPA123");

    delay(3000);
}