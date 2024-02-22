#include <esp_adc_cal.h>
#include <esp_task_wdt.h>
#include "tracker.h"
#include "serial_manager/serial_manager.h"

Tracker::Tracker(int id)
    : monitor{SerialManager::get_monitor_serial()}
{
    snprintf(server_url,sizeof(server_url)/sizeof(char), "%s%d", server_base_url, id);
    location.fixed = false;
    // esp_task_wdt_init(WDT_TIMEOUT, true);  // enable panic so ESP32 restarts
    // esp_task_wdt_add(NULL);
}

void Tracker::run() 
{
    power_on_board();
    gnss.init();
    start_fixing = millis();

    while(true)
    {
        loop();
    }
}

void Tracker::loop() 
{
    static bool location_fixing_done = false;
    if (!location_fixing_done)
    {
        if (acquiring_location())
        {
            gnss.turn_off(); // < turn off to save power
            location_fixing_done = true;
        }
        return;
    }
    
    auto battery_mv = read_battery_mv();
    monitor.println("Bat: " + String(battery_mv));

    if (modem.init())
    {
        send_info(battery_mv);
    }
    modem.turn_off();
    deep_sleep(battery_mv);
}

void Tracker::deep_sleep(uint16_t battery_mv) 
{
    power_off_board();
    uint64_t sleep_duration = calculate_sleep_duration(battery_mv);
    if (sleep_duration > 0)
    {
        // sanity check
        if (sleep_duration > UINT64_MAX / 1000000ULL)
        {
            sleep_duration = UINT64_MAX / 1000000ULL;
        }
        esp_sleep_enable_timer_wakeup(sleep_duration * 1000000ULL);
        delay(200);
    }
    esp_deep_sleep_start();
    power_on_board(); // < this never happen
}

void Tracker::power_on_board() 
{
    // Power on the board
    pinMode(BOARD_POWER_ON_PIN, OUTPUT);
    digitalWrite(BOARD_POWER_ON_PIN, HIGH);
    pinMode(BOARD_RST_PIN, OUTPUT);
    digitalWrite(BOARD_RST_PIN, LOW);
    delay(1000);
}

void Tracker::power_off_board() 
{
    digitalWrite(BOARD_POWER_ON_PIN, LOW);
}

void Tracker::send_info(uint16_t battery_mv) 
{
    String msg = String(battery_mv);
    if (location.fixed)
    {
        msg += ",";
        msg += String(location.dto.latitude,7);
        msg += ",";
        msg += String(location.dto.longitude,7);
        msg += ",";
        msg += String(location.dto.altitude,4);
        msg += ",";
        msg += String(location.dto.HDOP);
        msg += ",";
        msg += String(location.dto.satellites);
    }
    else
    {
        msg += ",0,0,0,0,0";
    }
    String cpsi = modem.get_info();
    if (cpsi.length() > 0)
    {
        msg += ",";
        msg += cpsi;
    }
    modem.https_post(server_url, msg);
}

uint16_t Tracker::read_battery_mv()
{
    esp_adc_cal_characteristics_t adc_chars;
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);
    return esp_adc_cal_raw_to_voltage(analogRead(BOARD_ADC_PIN), &adc_chars) * 2;
}

bool Tracker::acquiring_location() 
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
        location.fixed = true;
        location.dto = gnss_dto;
        monitor.println("Fixed!");
        return true;
    }
    else if(millis() - start_fixing > fix_timeout)
    {
        monitor.println("Timeout!");
        return true;
    }
    return false;
}

uint16_t Tracker::calculate_sleep_duration(uint16_t battery_mv) 
{
    if (battery_mv == 0)
    {
        monitor.println("Battery voltage is invalid!");
        return 0;
    }
    if (battery_mv < 3300 || battery_mv > 4400)
    {
        return 0;
    }
    if (battery_mv < 3700)
    {
        return 4 * sleep_duration_base;
    }
    return sleep_duration_base;
}
