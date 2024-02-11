#pragma once
#include "../config.h"

#include <Stream.h>

class SerialManager
{
public:
    static Stream& get_modem_serial();
    static Stream& get_gnss_serial();
    static Stream& get_monitor_serial();
};