#pragma once
#include <Arduino.h>
#include "response.h"
#include "modem_backend.h"

class Modem
{
public:
    Modem();
    ~Modem();

    bool init();
    void turn_off();

    Response https_get(const String& url);
    Response https_post(const String& url, const String& payload);
    Response https_post(const String& url, uint8_t *payload, size_t size);

    String get_info();

private:
    Modem_Backend* backend;
};