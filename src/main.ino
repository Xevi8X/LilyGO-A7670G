#include "config.h"
#include <Arduino.h>
#include "modem/modem.h"
#include "gnss/gnss.h"

const char server[] = "https://webhook.site/37cbb902-5503-41d9-8206-f96e92419be6";

const uint8_t required_info = GNSS_DTO_Flags::LOCATION | GNSS_DTO_Flags::SATELLITES;

GNSS gnss;
Modem modem;

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

    modem.init();
    gnss.init();
}

void loop()
{
    while(!gnss.update())
    {
        ;
    }

    auto gnss_dto = gnss.get();

    if(gnss_dto.flags & required_info == required_info)
    {
        String message = "Loc: ";
        message += String(gnss_dto.latitude,7);
        message += ", ";
        message += String(gnss_dto.longitude,7);
        message += ", ";
        message += String(gnss_dto.satellites);
        modem.https_post(server, message);
        delay(5000);
    }

    Serial.print("Sat: ");
    Serial.println(gnss_dto.satellites);
    delay(100); 
}