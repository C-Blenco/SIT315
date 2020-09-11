#ifndef PRODUCER_H
#define PRODUCER_H

#include <stdlib.h>
#include <time.h>
#include "TrafficData.h"
#include "TrafficBuffer.h"
#include <stdio.h>
// #include <pthread.h>
// #include <thread>
#include <fstream>
#include <string>
#include <sstream>

class Producer
{
    private:
    TrafficBuffer* buffer;
    int nTrafficLights;
    int nData;
    int prodId;
    int nProducers;
    // pthread_t pThread;

    public:
    Producer(TrafficBuffer* trafficBuffer, int nLights, int nData, int nProducers, int prodId);
    void produce(bool output, bool input);
    

    //void startThread();
};

#endif