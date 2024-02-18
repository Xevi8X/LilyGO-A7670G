#include <esp_adc_cal.h>
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
        message += ",";
        message += String(read_battery_mv());
        modem.https_post(server_url, message);
        deep_sleep();
    }

    if(millis() - start_fixing > fix_timeout)
    {
        String message = "Fix timeout! Bat: ";
        message += String(read_battery_mv());
        message += " mV";
        modem.https_post(server_url, message);
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
    pinMode(BOARD_POWER_ON_PIN, OUTPUT);
    digitalWrite(BOARD_POWER_ON_PIN, HIGH);
    pinMode(BOARD_RST_PIN, OUTPUT);
    digitalWrite(BOARD_RST_PIN, LOW);

    delay(3000);

    modem.init();
    gnss.init();
}

void Tracker::power_off_periphs() 
{
    gnss.turn_off();
    modem.turn_off();
    digitalWrite(BOARD_POWER_ON_PIN, LOW);
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

uint16_t Tracker::read_battery_mv()
{
    esp_adc_cal_characteristics_t adc_chars;
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);
    return esp_adc_cal_raw_to_voltage(analogRead(BOARD_ADC_PIN), &adc_chars) * 2;
}
