#ifndef TRAFFICTALLY_H
#define TRAFFICTALLY_H

#include <pthread.h>

struct TrafficTally
{
    int trafficLightId;
    int nCars;

    pthread_mutex_t mutexLock;

    void update(int num)
    {
        pthread_mutex_lock(&mutexLock);
        nCars += num;
        pthread_mutex_unlock(&mutexLock);
    }
};

#endif