#pragma once
#include <Arduino.h>
#include "response.h"

class Modem_Backend
{
public:
    virtual ~Modem_Backend() {}

    virtual void init() {};
    virtual void turn_off() {};

    virtual Response https_get(const String& url) = 0;
    virtual Response https_post(const String& url, uint8_t *payload, size_t size) = 0;
};