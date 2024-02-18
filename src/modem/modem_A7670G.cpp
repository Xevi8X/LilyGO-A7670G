#include "modem_A7670G.h"
#include "../serial_manager/serial_manager.h"

Modem_A7670G::Modem_A7670G()
    : modem(SerialManager::get_modem_serial()), monitor{SerialManager::get_monitor_serial()}
{}

void Modem_A7670G::init() 
{
    // power on by power key
    pinMode(BOARD_MODEM_PWR_PIN, OUTPUT);
    digitalWrite(BOARD_MODEM_PWR_PIN, LOW);
    delay(100);
    digitalWrite(BOARD_MODEM_PWR_PIN, HIGH);
    delay(1000);
    digitalWrite(BOARD_MODEM_PWR_PIN, LOW);

    modem.init();
    String name = modem.getModemName();
    monitor.print("Modem Name:");
    monitor.println(name);

    String modemInfo = modem.getModemInfo();
    monitor.print("Modem Info:");
    monitor.println(modemInfo);

    monitor.print("Waiting for network...");
    if (!modem.waitForNetwork()) {
        monitor.println(" fail");
        delay(10000);
        return;
    }
    monitor.println(" success");

    if (modem.isNetworkConnected()) {
        monitor.println("Network connected");
    }

    monitor.print(F("Connecting to "));
    monitor.print(apn);
    if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
        monitor.println(" fail");
        delay(10000);
        return;
    }
    monitor.println(" success");

    if (modem.isGprsConnected()) {
        monitor.println("GPRS connected");
    }

    monitor.print("IP: ");
    monitor.println(modem.getLocalIP());
}

void Modem_A7670G::turn_off() 
{
    modem.poweroff();
}

Response Modem_A7670G::https_get(const String &url)
{
    Response response;

    modem.https_begin();
    if (!modem.https_set_url(url)) {
        Serial.println("Failed to set the URL. Please check the validity of the URL!");
        return response;
    }

    modem.https_add_header("Accept-Encoding", "gzip, deflate, br");
    modem.https_set_accept_type("plain/text");
    modem.https_set_user_agent("lilygo-A7670G");

    response.http_code = modem.https_get();
    response.header = modem.https_header();
    response.body = modem.https_body();

    return response;
}

Response Modem_A7670G::https_post(const String &url, uint8_t *payload, size_t size)
{
    Response response;

    modem.https_begin();
    if (!modem.https_set_url(url)) {
        Serial.println("Failed to set the URL. Please check the validity of the URL!");
        return response;
    }

    modem.https_add_header("Accept-Encoding", "gzip, deflate, br");
    modem.https_set_accept_type("plain/text");
    modem.https_set_user_agent("lilygo-A7670G");

    response.http_code = modem.https_post(payload, size);
    response.header = modem.https_header();
    response.body = modem.https_body();

    return response;
}

String Modem_A7670G::get_info()
{
    String cpsi;
    if(modem.getSystemInformation(cpsi));
    {
        return cpsi;
    }
    return "";
}
