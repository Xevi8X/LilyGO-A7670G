#include "config.h"
#include <Arduino.h>
#include <TinyGPS++.h>

#define DUMP_AT_COMMANDS

// Your GPRS credentials, if any
const char apn[] = "internet";
const char gprsUser[] = "";
const char gprsPass[] = "";

const char server[] = "https://webhook.site/37cbb902-5503-41d9-8206-f96e92419be6";

const int port = 80;
String myid = "1234";

#include <TinyGsmClient.h>

#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, Serial);
TinyGsm        modem(debugger);
#else
TinyGsm        modem(SerialAT);
#endif

TinyGPSPlus gps;

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
    String name = modem.getModemName();
    Serial.print("Modem Name:");
    Serial.println(name);

    String modemInfo = modem.getModemInfo();
    Serial.print("Modem Info:");
    Serial.println(modemInfo);

    Serial.print("Waiting for network...");
    if (!modem.waitForNetwork()) {
        Serial.println(" fail");
        delay(10000);
        return;
    }
    Serial.println(" success");

    if (modem.isNetworkConnected()) {
        Serial.println("Network connected");
    }

    Serial.print(F("Connecting to "));
    Serial.print(apn);
    if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
        Serial.println(" fail");
        delay(10000);
        return;
    }
    Serial.println(" success");

    if (modem.isGprsConnected()) {
        Serial.println("GPRS connected");
    }

    Serial.print("IP: ");
    Serial.println(modem.getLocalIP());
}

void loop()
{
    // Read waiting bytes on GPS UART
    while (SerialGPS.available()) {
        int c = SerialGPS.read();
        if (gps.encode(c)) {
            // TODO: ready reading
        }
    }

    // Initialize HTTPS
    modem.https_begin();

    // Set GET URT
    if (!modem.https_set_url(server)) {
        Serial.println("Failed to set the URL. Please check the validity of the URL!");
        return;
    }

    // 
    modem.https_add_header("Accept-Language", "zh-CN,zh;q=0.9,en;q=0.8,en-GB;q=0.7,en-US;q=0.6");
    modem.https_add_header("Accept-Encoding", "gzip, deflate, br");
    modem.https_set_accept_type("application/json");
    modem.https_set_user_agent("TinyGSM/LilyGo-A76XX");

    String post_body = "This is post example!";

    int httpCode = modem.https_post(post_body);
    if (httpCode != 200) {
        Serial.print("HTTP post failed ! error code = "); 
        Serial.println(httpCode); 
        return;
    }

    // Get HTTPS header information
    String header = modem.https_header();
    Serial.print("HTTP Header : ");
    Serial.println(header);

    // Get HTTPS response
    String body = modem.https_body();
    Serial.print("HTTP body : ");
    Serial.println(body);

    delay(3000);
}