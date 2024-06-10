#include <esp_adc_cal.h>
#include <esp_task_wdt.h>
#include "tracker.h"
#include "serial_manager/serial_manager.h"

Tracker::Tracker(int id)
    :   monitor{SerialManager::get_monitor_serial()},
        start_location_settling{0},
        wakeup_cause{esp_sleep_get_wakeup_cause()}
{
    snprintf(server_url,sizeof(server_url)/sizeof(char), "%s%d", server_base_url, id);
    location.fixed = false;
    monitor.println("Wakeup cause: " + String(static_cast<uint8_t>(wakeup_cause)));

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
    
    battery_mv = read_battery_mv();
    monitor.println("Bat: " + String(battery_mv));

    charger_status = read_charger_status();
    monitor.println("Charger: " + String(charger_status));

    if (modem.init())
    {
        send_info();
    }
    modem.turn_off();
    deep_sleep();
}

void Tracker::deep_sleep() 
{
    power_off_board();
    uint64_t sleep_duration = calculate_sleep_duration();
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
    // always wake up on charger status change
    esp_sleep_enable_ext1_wakeup(1ULL << BOARD_CHARGER_STATUS_PIN, charger_status ? ESP_EXT1_WAKEUP_ALL_LOW : ESP_EXT1_WAKEUP_ANY_HIGH);
    monitor.println("Bye!");
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

void Tracker::send_info() 
{

    String msg = String(static_cast<uint8_t>(wakeup_cause));

    msg += ",";
    msg += String(battery_mv);

    if (charger_status)
    {
        msg += ",1";
    }
    else
    {
        msg += ",0";
    }

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
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_12, ADC_WIDTH_BIT_12, 1100, &adc_chars);
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
        if(!location.fixed)
        {
            location.fixed = true;
            start_location_settling = millis();
            monitor.println("Fixed!");
            
            // sanity
            location.dto = gnss_dto;
        }
        location.filter.push(gnss_dto);
    }
    else if(start_location_settling > 0U && (millis() - start_location_settling > location_settling_time))
    {
        if(location.filter.is_started())
        {
            location.dto = location.filter.get();
            monitor.println("Settled!");
        }
        return true;
    }
    else if(millis() - start_fixing > fix_timeout)
    {
        monitor.println("Timeout!");
        return true;
    }
    return false;
}

uint64_t Tracker::calculate_sleep_duration() 
{
    if (battery_mv == 0)
    {
        monitor.println("Battery voltage is invalid!");
        return 0LLU;
    }
    if (battery_mv < battery_crit_mv || battery_mv > battery_overvoltage_mv)
    {
        return 0LLU;
    }
    if (battery_mv < battery_low_mv)
    {
        return low_battery_multiplier * sleep_duration_base;
    }
    return sleep_duration_base;
}

bool Tracker::read_charger_status()
{
    pinMode(BOARD_CHARGER_STATUS_PIN, INPUT);
    return (digitalRead(BOARD_CHARGER_STATUS_PIN) == HIGH) ? true : false;
}
