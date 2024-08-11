#include "modem_A7670G.h"
#include "../serial_manager/serial_manager.h"

Modem_A7670G::Modem_A7670G()
    : modem(SerialManager::get_modem_serial()), monitor{SerialManager::get_monitor_serial()}
{}

bool Modem_A7670G::init() 
{
    // power on by power key
    pinMode(BOARD_MODEM_DTR_PIN, OUTPUT);
    digitalWrite(BOARD_MODEM_DTR_PIN, LOW);
    pinMode(BOARD_MODEM_PWR_PIN, OUTPUT);
    digitalWrite(BOARD_MODEM_PWR_PIN, LOW);
    delay(100);
    digitalWrite(BOARD_MODEM_PWR_PIN, HIGH);
    delay(1000);
    digitalWrite(BOARD_MODEM_PWR_PIN, LOW);
    digitalWrite(BOARD_MODEM_DTR_PIN, HIGH);

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
        return false;
    }
    monitor.println(" success");

    if (modem.isNetworkConnected()) {
        monitor.println("Network connected");
    }
    else
    {
        return false;
    }

    monitor.print(F("Connecting to "));
    monitor.print(apn);
    if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
        monitor.println(" fail");
        return false;
    }
    monitor.println(" success");

    if (modem.isGprsConnected()) {
        monitor.println("GPRS connected");
    }

    monitor.print("IP: ");
    monitor.println(modem.getLocalIP());

    // modem.sendAT(GF("+CSSLCFG=\"ignorelocaltime\",0,1"));  // Ignore SSL certificate time
    // if (modem.waitResponse(5000L) != 1) monitor.println("Failed to set SSL certificate time");

    // modem.sendAT(GF("+CCLK=\"24/08/11,13:00:00+01\""));
    // if (modem.waitResponse(5000L) != 1) monitor.println("Failed to set time");

    // modem.sendAT(GF("+CSSLCFG=\"sslversion\",0,3"));  // TLS 1.2
    // if (modem.waitResponse(5000L) != 1) monitor.println("Failed to set SSL version");

    // modem.sendAT(GF("+CSSLCFG=\"enableSNI\",0,1"));  // SNI
    // if (modem.waitResponse(5000L) != 1) monitor.println("Failed to enable SNI");

    return true;
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
    modem.https_set_accept_type("*/*");
    modem.https_set_user_agent("lilygo-A7670G");

    for (uint8_t i = 0; i < retry; i++) {
        response.http_code = modem.https_get();
        if (response.http_code > 0) {
            response.header = modem.https_header();
            response.body = modem.https_body();
            break;
        }
    }
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

    modem.https_add_header("Connection", "keep-alive");
    modem.https_add_header("Accept-Encoding", "gzip, deflate, br");
    modem.https_set_content_type("text/plain");
    modem.https_set_accept_type("*/*");
    modem.https_set_user_agent("lilygo-A7670G");

    for (uint8_t i = 0; i < retry; i++) {
        response.http_code = modem.https_post(payload, size);
        if (response.http_code > 0) {
            response.header = modem.https_header();
            response.body = modem.https_body();
            break;
        }
    }
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
