#include "config.h"
#include <Arduino.h>
#include "gnss/gnss.h"
#include "location_filter/location_filter.h"
#include "modem/modem.h"
#include "server/server_info.h"
#include <Stream.h>

class Tracker
{
public:
    Tracker();

    void run();
    
private:
    Stream& monitor;
    GNSS gnss;
    Modem modem;

    enum class Stage
    {
        INITIALIZING,
        FIXING,
        LOCATION_SETTLING,
        SENDING,
        DEEP_SLEEPING,
        LIGHT_SLEEPING,
    };

    enum SLEEP_ERRORS : int8_t
    {
        BATTERY_UNKNOWN         = -1,
        BATTERY_UNDERVOLTAGE    = -2,
        BATTERY_OVERVOLTAGE     = -3,
    };

    Stage stage;


    uint32_t fixing_timeout_timepoint;
    uint32_t settling_timeout_timepoint;

    struct 
    {
        bool valid;
        GNSS_DTO dto;
        LocationFilter filter;
    } location;

    uint16_t battery_mv;
    bool charger_status;
    const esp_sleep_wakeup_cause_t wakeup_cause;
    uint64_t sleep_duration;
    bool should_deep_sleep;
    int8_t sleep_error;
    
    void loop();  
    void deep_sleep();
    void light_sleep();
    void power_on_board();
    void power_off_board();
    void send_info();
    uint16_t read_battery_mv();
    uint16_t calibrate_adc_voltage(uint16_t raw);
    bool acquire_location(GNSS_DTO& location);
    void calculate_sleep_duration();
    bool read_charger_status();

    ServerInfo servers[2] = {
        {.host = "xevix.tplinkdns.com", .path = "location", .ssl = true, .break_on_success = false},
        {.host = "api.secondary.iotlocator.cloud", .path = "api/v1", .ssl = true, .break_on_success = true},
    };

    // Minimal valid information
    static constexpr uint8_t required_info = 
          GNSS_DTO_Flags::LOCATION 
        | GNSS_DTO_Flags::SPEED 
        | GNSS_DTO_Flags::SATELLITES 
        | GNSS_DTO_Flags::HDOP;
    static constexpr int min_sat = 5;
    static constexpr float max_HDOP = 20.0f;

    // Time parameters
    static constexpr uint64_t sleep_duration_base = 3600ULL;        // 60min = 3600s
    static constexpr uint32_t on_charge_sleep_duration = 60000UL;   // 1min = 60'000ms
    static constexpr uint32_t fix_timeout = 300000UL;               // 5min = 300'000ms
    static constexpr uint32_t location_settling_time = 20000UL;     // 20s = 20'000ms
    static constexpr uint64_t low_battery_multiplier = 4;           // 4 x 60min = 240min

    // Battery parameters
    static constexpr uint16_t battery_crit_mv = 3200;
    static constexpr uint16_t battery_low_mv = 3500;
    static constexpr uint16_t battery_overvoltage_mv = 4350;

    static constexpr char protocol_version = 'A';
};