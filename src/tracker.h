#include "config.h"
#include <Arduino.h>
#include "modem/modem.h"
#include "gnss/gnss.h"
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
    uint32_t start_fixing;
    uint32_t blink;

    struct 
    {
        bool fixed;
        GNSS_DTO dto;
    } location;

    const esp_sleep_wakeup_cause_t wakeup_cause;
    
    void loop();  
    void deep_sleep(uint16_t battery_mv);
    void power_on_board();
    void power_off_board();
    void send_info(uint16_t battery_mv, bool charger_status);
    uint16_t read_battery_mv();
    bool acquiring_location();
    uint64_t calculate_sleep_duration(uint16_t battery_mv);
    bool read_charger_status();

    const char* server_base_url = "https://xevix.tplinkdns.com/location/";
    char server_url[50];

    // Minimal valid information
    static constexpr uint8_t required_info = GNSS_DTO_Flags::LOCATION | GNSS_DTO_Flags::SATELLITES | GNSS_DTO_Flags::HDOP;
    static constexpr int min_sat = 5;
    static constexpr float max_HDOP = 10.0f;

    // Time parameters
    static constexpr uint64_t sleep_duration_base = 3600ULL; // 60min = 3600s
    static constexpr uint32_t fix_timeout = 300000UL;        // 5min = 300'000ms
    static constexpr uint32_t blink_interval = 1000UL;       // 1s = 1000ms
    static constexpr uint64_t low_battery_multiplier = 4;    // 4 x 60min = 240min

    // Battery parameters
    static constexpr uint16_t battery_crit_mv = 3200;
    static constexpr uint16_t battery_low_mv = 3500;
    static constexpr uint16_t battery_overvoltage_mv = 4350;


};