#include "gnss_nmea.h"


class GNSS_L76K : public GNSS_NMEA
{
public:
    virtual ~GNSS_L76K() {}

    void init() override;

    void turn_off() override;
};