#pragma once
#include <Arduino.h>
#include "gnss_dto.h"
#include "gnss_backend.h"

class GNSS
{
public:
    GNSS();
    ~GNSS();

    void init();
    void turn_off();

    bool update();
    GNSS_DTO get();

private:
    GNSS_Backend* backend;
};