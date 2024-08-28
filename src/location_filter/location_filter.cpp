#include "location_filter.h"

void LocationFilter::push(const GNSS_DTO &location)
{
    if (!started)
    {
        best_location = location;
        started = true;
        return;
    }

    if (location.flags & GNSS_DTO_Flags::SPEED && location.speed > min_moving_speed)
    {
        best_location = location;
        return;
    }

    if (location.flags & GNSS_DTO_Flags::HDOP 
        && location.HDOP < best_location.HDOP)
    {
        best_location = location;
        return;
    }

    if (!(best_location.flags & GNSS_DTO_Flags::HDOP))
    {
        best_location = location;
        return;
    }

}

GNSS_DTO LocationFilter::get() const
{
    return best_location;
}

bool LocationFilter::is_started() const
{
    return started;
}


void LocationFilter::reset()
{
    started = false;
}
