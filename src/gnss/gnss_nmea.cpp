#include "gnss_nmea.h"
#include "../serial_manager/serial_manager.h"

GNSS_NMEA::GNSS_NMEA()
    : serial_gps{SerialManager::get_gnss_serial()}
{

}

bool GNSS_NMEA::update() {
    while (serial_gps.available()) {
        int c = serial_gps.read();
        if (gps.encode(c)) 
        {
            return true;
        }
    }
    return false;
}

bool is_zero(double val)
{
    return abs(val) < 1e-4;
}

GNSS_DTO GNSS_NMEA::get()
{
    GNSS_DTO dto;
    dto.flags = 0;

    if(gps.time.isValid() && gps.time.age() < validity 
        && gps.date.isValid() && gps.date.age() < validity)
    {
        dto.date = gps.date.value();
        dto.time = gps.time.value();
        dto.flags |= GNSS_DTO_Flags::TIME;
    }

    if(gps.location.isValid() && gps.location.age() < validity
        && !is_zero(gps.location.lat()) && !is_zero(gps.location.lng()))
    {
        dto.latitude = static_cast<float>(gps.location.lat());
        dto.longitude = static_cast<float>(gps.location.lng());
        dto.flags |= GNSS_DTO_Flags::LOCATION;
    }

    if(gps.altitude.isValid() && gps.altitude.age() < validity)
    {
        dto.altitude = static_cast<float>(gps.altitude.meters());
        dto.flags |= GNSS_DTO_Flags::ALTITUDE;
    }

    if(gps.speed.isValid() && gps.speed.age() < validity
        && gps.course.isValid() && gps.course.age() < validity)
    {
        dto.speed = static_cast<float>(gps.speed.mps());
        dto.course = static_cast<float>(gps.course.deg());
        dto.flags |= GNSS_DTO_Flags::SPEED;
    }

    if(gps.satellites.isValid() && gps.satellites.age() < validity)
    {
        dto.satellites = gps.satellites.value();
        dto.flags |= GNSS_DTO_Flags::SATELLITES;
    }

    return dto;
}
