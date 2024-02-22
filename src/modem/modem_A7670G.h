#include "modem_backend.h"
#include "../config.h"
#include <TinyGsmClient.h>
#include <StreamDebugger.h>

class Modem_A7670G : public Modem_Backend
{
public:
    Modem_A7670G();
    virtual ~Modem_A7670G() {}

    bool init() override;
    void turn_off() override;

    Response https_get(const String& url) override;
    Response https_post(const String& url, uint8_t *payload, size_t size) override;

    String get_info() override;

private:
    Stream&        monitor;
    TinyGsm        modem;

    const char* apn = "internet";
    const char* gprsUser = "";
    const char* gprsPass = "";

    static constexpr uint16_t timeout_ms = 20000;
    static constexpr uint8_t retry = 5;
};