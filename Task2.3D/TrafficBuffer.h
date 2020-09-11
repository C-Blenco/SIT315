#ifndef TRAFFIC_BUFFER_H
#define TRAFFIC_BUFFER_H

#include <semaphore.h>
#include "TrafficData.h"
#include <ostream>

class TrafficBuffer
{
    private:
    TrafficData* buffer;
    
    public:
    int consumeId = 0;
    int produceId = 0;
    int buffSize;

    sem_t nFull;
    sem_t nEmpty;
    sem_t lock;

    TrafficBuffer(int bufferSize);
    
    void create(TrafficData data);

    TrafficData remove();

    void destroy();

    void printCurrent();
};

#endif