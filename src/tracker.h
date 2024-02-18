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

    void loop();  
    void deep_sleep();
    void power_on_periphs();
    void power_off_periphs();
    void send_info();

    const char* server_base_url = "https://xevix.tplinkdns.com/location/";
    char server_url[50];

    // Minimal valid information
    static constexpr uint8_t required_info = GNSS_DTO_Flags::LOCATION | GNSS_DTO_Flags::SATELLITES | GNSS_DTO_Flags::HDOP;
    static constexpr int min_sat = 5;
    static constexpr float max_HDOP = 10.0f;

    // Time parameters
    static constexpr int sleep_duration = 600.0f; //< 10min
    static constexpr uint32_t fix_timeout = 120000UL; //< 2min
};