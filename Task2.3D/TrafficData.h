#ifndef TRAFFICDATA_H
#define TRAFFICDATA_H

#include <time.h>

struct TrafficData
{
    time_t timeStamp;
    int TrafficLightId;
    int nCars;
};

#endif