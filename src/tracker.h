#include "config.h"
#include <Arduino.h>
#include "modem/modem.h"
#include "gnss/gnss.h"
#include "location_filter/location_filter.h"
#include <Stream.h>

class Tracker
{
public:
    Tracker(int id = 1);

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
        SLEEPING,
    };

    Stage stage;
    uint32_t timepoint;

    uint32_t start_location_settling;
    struct 
    {
        bool valid;
        GNSS_DTO dto;
        LocationFilter filter;
    } location;

    uint16_t battery_mv;
    bool charger_status;
    const esp_sleep_wakeup_cause_t wakeup_cause;
    
    void loop();  
    void deep_sleep();
    void power_on_board();
    void power_off_board();
    void send_info();
    uint16_t read_battery_mv();
    uint16_t calibrate_adc_voltage(uint16_t raw);
    bool acquire_location(GNSS_DTO& location);
    uint64_t calculate_sleep_duration();
    bool read_charger_status();

    const char* server_base_url = "https://xevix.tplinkdns.com/location/";
    char server_url[50];

    // Minimal valid information
    static constexpr uint8_t required_info = GNSS_DTO_Flags::LOCATION | GNSS_DTO_Flags::SATELLITES | GNSS_DTO_Flags::HDOP;
    static constexpr int min_sat = 5;
    static constexpr float max_HDOP = 20.0f;

    // Time parameters
    static constexpr uint64_t sleep_duration_base = 3600ULL;    // 60min = 3600s
    static constexpr uint32_t fix_timeout = 300000UL;           // 5min = 300'000ms
    static constexpr uint32_t location_settling_time = 20000UL; // 20s = 20'000ms
    static constexpr uint64_t low_battery_multiplier = 4;       // 4 x 60min = 240min

    // Battery parameters
    static constexpr uint16_t battery_crit_mv = 3200;
    static constexpr uint16_t battery_low_mv = 3500;
    static constexpr uint16_t battery_overvoltage_mv = 4350;


};