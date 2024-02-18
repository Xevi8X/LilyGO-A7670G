#include "tracker.h"
#include "serial_manager/serial_manager.h"

Tracker::Tracker(int id)
    : monitor{SerialManager::get_monitor_serial()}
{
    snprintf(server_url,sizeof(server_url)/sizeof(char), "%s%d", server_base_url, id);
    power_on_periphs();
    send_info();
    start_fixing = millis();
}

void Tracker::run() 
{

    while(true)
    {
        loop();
    }
}

void Tracker::loop() 
{
    while(!gnss.update())
    {
        ;
    }

    auto gnss_dto = gnss.get();

    if(gnss_dto.flags & required_info == required_info
        && gnss_dto.satellites > min_sat
        && gnss_dto.HDOP > 0.0f && gnss_dto.HDOP < max_HDOP)
    {
        String message = String(gnss_dto.latitude,7);
        message += ",";
        message += String(gnss_dto.longitude,7);
        message += ",";
        message += String(gnss_dto.altitude,5);
        message += ",";
        message += String(gnss_dto.HDOP);
        message += ",";
        message += String(gnss_dto.satellites);
        modem.https_post(server_base_url, message);
        deep_sleep();
    }

    if(millis() - start_fixing > fix_timeout)
    {
        modem.https_post(server_url,"Fix timeout!");
        deep_sleep();
    }
}

void Tracker::deep_sleep() 
{
    power_off_periphs();
    esp_sleep_enable_timer_wakeup(sleep_duration * 1000000ULL);
    delay(200);
    esp_deep_sleep_start();
    power_on_periphs(); // < this never happen
}

void Tracker::power_on_periphs() 
{
    modem.init();
    gnss.init();
}

void Tracker::power_off_periphs() 
{
    modem.turn_off();
    gnss.turn_off();
}

void Tracker::send_info() 
{
    modem.https_post(server_url,"Starting tracking!");
    String cpsi = modem.get_info();
    if(cpsi.length() > 0)
    {
        modem.https_post(server_url,cpsi);
    }
}
