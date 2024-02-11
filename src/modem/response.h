#pragma once
#include <Arduino.h>

struct Response
{
    int http_code = -1;
    String header = "";
    String body = "";
};