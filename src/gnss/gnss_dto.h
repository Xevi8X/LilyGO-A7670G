#pragma once
#include <stdint.h>

enum GNSS_DTO_Flags : uint8_t
{
    TIME        = (1 << 0),
    LOCATION    = (1 << 1),
    ALTITUDE    = (1 << 2),
    HDOP        = (1 << 3),
    SPEED       = (1 << 4),
    SATELLITES  = (1 << 5),
};

struct GNSS_DTO
{
    uint8_t flags       = 0;
    uint32_t date       = 0;
    uint32_t time       = 0;
    float latitude      = 0.0f;
    float longitude     = 0.0f;
    float altitude      = 0.0f;
    float HDOP          = 0.0f;
    float speed         = 0.0f;
    float course        = 0.0f;
    int satellites      = 0;
};
