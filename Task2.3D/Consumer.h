#ifndef CONSUMER_H
#define CONSUMER_H

#include <stdlib.h>
#include <time.h>
#include "TrafficData.h"
#include "TrafficBuffer.h"
#include "TrafficTally.h"
#include <vector>


using namespace std;


class Consumer
{
    private:
    TrafficBuffer* buffer;
    int nTrafficLights;
    int nData;
    vector<TrafficTally*> trafficLights;
    int consId;
    int nConsumers;
    

    public:
    Consumer(TrafficBuffer* trafficBuffer, int nLights, int nData, vector<TrafficTally*> trafficLights, int nConsumers, int consId);

    void consume();
};

#endif