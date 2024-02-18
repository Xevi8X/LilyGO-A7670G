#pragma once
#include "gnss_backend.h"
#include "gnss_dto.h"
#include <TinyGPS++.h>


class GNSS_NMEA : public GNSS_Backend
{
public:
    GNSS_NMEA();
    virtual ~GNSS_NMEA() {}

    bool update() override;

    GNSS_DTO get() override;

private:
    TinyGPSPlus gps;
    Stream& serial_gps;

    constexpr static uint32_t validity = 60000U; 
};