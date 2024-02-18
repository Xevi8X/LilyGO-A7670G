#include "Arduino.h"
#include "modem.h"
#include "modem_A7670G.h"

Modem::Modem()
{

}

Modem::~Modem()
{
    if (backend)
    {
        delete backend;
    }
}

void Modem::init() {
    backend = new Modem_A7670G();

    if (backend) {
        backend->init();
    }
}

void Modem::turn_off() 
{
    if(backend)
    {
        backend->turn_off();
    }
}

Response Modem::https_get(const String &url)
{
    if(!backend)
    {
        return Response();
    }
    return backend->https_get(url);
}

Response Modem::https_post(const String &url, const String &payload)
{
   return https_post(url, (uint8_t*) payload.c_str(), payload.length());
}

Response Modem::https_post(const String &url, uint8_t *payload, size_t size)
{
    if(!backend)
    {
        return Response();
    }
    return backend->https_post(url, payload, size);
}

String Modem::get_info()
{
    if(!backend)
    {
        return "";
    }
    return backend->get_info();
}
