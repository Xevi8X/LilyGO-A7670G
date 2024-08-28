#pragma once

#include "../gnss/gnss_dto.h"

class LocationFilter
{
public:
    void push(const GNSS_DTO& location);
    GNSS_DTO get() const;
    bool is_started() const;
    void reset();

private:
    bool started = false;
    GNSS_DTO best_location;

    static constexpr float min_moving_speed = 2.0f;
};
