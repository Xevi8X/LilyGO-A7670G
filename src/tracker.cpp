#include <esp_adc_cal.h>
#include <esp_task_wdt.h>
#include "tracker.h"
#include "serial_manager/serial_manager.h"

Tracker::Tracker()
    :   monitor{SerialManager::get_monitor_serial()},
        stage{Stage::INITIALIZING},
        start_location_settling{0},
        wakeup_cause{esp_sleep_get_wakeup_cause()}
{
    location.valid = false;

    // esp_task_wdt_init(WDT_TIMEOUT, true);  // enable panic so ESP32 restarts
    // esp_task_wdt_add(NULL);
}

void Tracker::run() 
{
#if defined(BATTERY_READ_TEST)
    monitor.println("Battery read test");
    while(true)
    {
        monitor.println(read_battery_mv());
        delay(1000);
    }
#endif

    while(true)
    {
        loop();
    }
}

void Tracker::loop() 
{
    GNSS_DTO gnss_dto;

    switch (stage)
    {
        case Stage::INITIALIZING:
            monitor.println("Initializing...");
            monitor.println("Wakeup cause: " + String(static_cast<uint8_t>(wakeup_cause)));
            power_on_board();
            gnss.init();

            monitor.println("Fixing...");
            timepoint = millis();
            stage = Stage::FIXING;
            break;

        case Stage::FIXING:
            if(millis() - timepoint > fix_timeout)
            {
                monitor.println("Timeout!");
                stage = Stage::SENDING;
            }
            else if (acquire_location(gnss_dto))
            {
                monitor.println("First fix!");
                location.filter.push(gnss_dto);
                timepoint = millis();
                stage = Stage::LOCATION_SETTLING;
            }
            break;

        case Stage::LOCATION_SETTLING:
            if(millis() - timepoint > location_settling_time)
            {
                if(location.filter.is_started())
                {
                    location.dto = location.filter.get();
                    location.valid = true;
                    monitor.println("Settled!");
                }
                stage = Stage::SENDING;
            }
            else if (acquire_location(gnss_dto))
            {
                location.filter.push(gnss_dto);
                monitor.println("Fix!");
            }
            break;

        case Stage::SENDING:
            battery_mv = read_battery_mv();
            monitor.println("Bat: " + String(battery_mv));

            charger_status = read_charger_status();
            monitor.println("Charger: " + String(charger_status));

            calculate_sleep_duration();

            if (modem.init())
            {
                send_info();
                monitor.println("Sent!");
            }
            else
            {
                monitor.println("Modem init failed!");
            }

            modem.turn_off();
            timepoint = millis();
            stage = should_deep_sleep ? Stage::DEEP_SLEEPING : Stage::LIGHT_SLEEPING;
            break;

        case Stage::DEEP_SLEEPING:
            monitor.println("Preparing for deep sleep!");
            gnss.turn_off(); // < turn off to save power
            deep_sleep();
            break; // < this never happen

        case Stage::LIGHT_SLEEPING:
            light_sleep();
            break;

    }
}

void Tracker::deep_sleep() 
{
    power_off_board();
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

void Tracker::light_sleep() 
{
    if (millis() < timepoint || millis() - timepoint > static_cast<uint32_t>(sleep_duration * 1000ULL) - location_settling_time)
    {
        GNSS_DTO gnss_dto;
        location.filter.reset();
        location.valid = false;
        if (acquire_location(gnss_dto))
        {
            monitor.println("Fix!");
            location.filter.push(gnss_dto);
            timepoint = millis();
            stage = Stage::LOCATION_SETTLING;
        }
        else
        {
            stage = Stage::SENDING;
        }
    }
    else
    {
        delay(500);
    }
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
    String IMEI = modem.get_IMEI();

    monitor.print("IMEI: ");
    monitor.println(IMEI);

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

    msg += ",";
    if (sleep_duration > 0)
    {
        msg += String(sleep_duration);
    }
    else
    {
        msg += String(sleep_error);
    }

    if (location.valid)
    {
        msg += ",";
        msg += String(location.dto.latitude,7);
        msg += ",";
        msg += String(location.dto.longitude,7);
        msg += ",";
        msg += String(location.dto.altitude,4);
        msg += ",";
        msg += String(location.dto.speed,2);
        msg += ",";
        msg += String(location.dto.course,1);
        msg += ",";
        msg += String(location.dto.HDOP);
        msg += ",";
        msg += String(location.dto.satellites);
    }
    else
    {
        msg += ",0,0,0,0,0,0,0";
    }
    String cpsi = modem.get_info();
    if (cpsi.length() > 0)
    {
        msg += ",";
        msg += cpsi;
    }

    for (auto& server : servers)
    {
        auto url = server.get_url(IMEI.c_str());
        auto url_str = String(url.c_str(),url.length());
        auto responce = modem.https_post(url_str, msg);

        monitor.println("Responce url:\n" + url_str);
        monitor.println("Responce code:\n" + String(responce.http_code));
        if (responce.http_code >= 200 && responce.http_code < 300)
        {
            monitor.println("Responce header:\n" + String(responce.header));
            monitor.println("Responce payload:\n" + String(responce.body));
            if (server.break_on_success)
            {
                break;
            }
        }
    }
    
}

uint16_t Tracker::read_battery_mv()
{
    esp_adc_cal_characteristics_t adc_chars;
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_12, ADC_WIDTH_BIT_12, 1100, &adc_chars);
    ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH_BIT_12));
    ESP_ERROR_CHECK(adc1_config_channel_atten(BOARD_ADC_CHANNEL, ADC_ATTEN_DB_12));
    return calibrate_adc_voltage(esp_adc_cal_raw_to_voltage(adc1_get_raw(BOARD_ADC_CHANNEL), &adc_chars) * 2);
}

uint16_t Tracker::calibrate_adc_voltage(uint16_t raw)
{
    // Temporiarly use a linear approximation, but it should be replaced by a proper two point esp calibration
    constexpr float coeff_a = 1.3017f;
    constexpr float coeff_b = -1360.9f;
    float val = coeff_a * raw + coeff_b;
    return val > 0.0f ? static_cast<uint32_t>(val) : 0U;
}

bool Tracker::acquire_location(GNSS_DTO& location) 
{
    while(!gnss.update())
    {
        ;
    }
        
    GNSS_DTO gnss_dto = gnss.get();

    if(gnss_dto.flags & required_info == required_info
        && gnss_dto.satellites > min_sat
        && gnss_dto.HDOP > 0.0f && gnss_dto.HDOP < max_HDOP)
    {
        location = gnss_dto;
        return true;
    }

    return false;
}

void Tracker::calculate_sleep_duration() 
{
    should_deep_sleep = true;
    if (charger_status)
    {
        sleep_duration = static_cast<uint64_t>(on_charge_sleep_duration) / 1000ULL;
        should_deep_sleep = false;
        return;
    }
    if (battery_mv == 0)
    {
        monitor.println("Battery voltage is invalid!");
        sleep_duration = 0LLU;
        sleep_error = SLEEP_ERRORS::BATTERY_UNKNOWN;
        return;
    }
    if (battery_mv < battery_crit_mv)
    {
        sleep_duration = 0LLU;
        sleep_error = SLEEP_ERRORS::BATTERY_UNDERVOLTAGE;
        return;
    }
    if (battery_mv > battery_overvoltage_mv)
    {
        sleep_duration = 0LLU;
        sleep_error = SLEEP_ERRORS::BATTERY_OVERVOLTAGE;
        return;
    }
    if (battery_mv < battery_low_mv)
    {
        sleep_duration = low_battery_multiplier * sleep_duration_base;
        return;
    }
    sleep_duration = sleep_duration_base;
}

bool Tracker::read_charger_status()
{
    pinMode(BOARD_CHARGER_STATUS_PIN, INPUT);
    return (digitalRead(BOARD_CHARGER_STATUS_PIN) == HIGH) ? true : false;
}
