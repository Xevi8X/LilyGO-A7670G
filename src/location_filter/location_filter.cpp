#include "location_filter.h"

void LocationFilter::push(const GNSS_DTO &location)
{
    if (!started)
    {
        best_location = location;
        started = true;
        return;
    }

    if (location.HDOP < best_location.HDOP)
    {
        best_location = location;
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
