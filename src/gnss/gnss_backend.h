#pragma once
#include "gnss_dto.h"

class GNSS_Backend
{
public:
    virtual ~GNSS_Backend() {}

    virtual void init() {}

    virtual void turn_off() {}

    virtual bool update() = 0;

    virtual GNSS_DTO get() = 0;
};