#include "gnss.h"
#include "gnss_L76K.h"

GNSS::GNSS()
{

}

GNSS::~GNSS() 
{
    if (backend)
    {
        delete backend;
    }
}

void GNSS::init()
{
    backend = new GNSS_L76K();

    if (backend) {
        backend->init();
    }
}

bool GNSS::update()
{
    if(backend)
    {
        return backend->update();
    }
    return false;
}

GNSS_DTO GNSS::get()
{
    if(backend)
    {
        return backend->get();
    }
    return GNSS_DTO();
}

void GNSS::turn_off() 
{
    if(backend)
    {
        backend->turn_off();
    }
}
