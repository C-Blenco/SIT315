#include "TrafficBuffer.h"

TrafficBuffer::TrafficBuffer(int bufferSize)
{
    // Initialise buffer to size specified
    buffer = new TrafficData[bufferSize];
    buffSize = bufferSize;

    // Initialise semaphores
    sem_init(&nEmpty, 0, bufferSize);
    sem_init(&nFull, 0, 0);
    sem_init(&lock, 0, 1);
}

void TrafficBuffer::create(TrafficData data)
{
    sem_wait(&nEmpty);
    sem_wait(&lock);

    buffer[produceId] = data;
    produceId = (produceId + 1) % buffSize;

    sem_post(&lock);
    sem_post(&nFull);
}

TrafficData TrafficBuffer::remove()
{
    sem_wait(&nFull);
    sem_wait(&lock);


    // printf("----consumer idx: %d\n", consumeId);
    TrafficData data = buffer[consumeId];
    consumeId = (consumeId + 1) % buffSize;
    // printf("----consumer idx after: %d\n", consumeId);

    sem_post(&lock);
    sem_post(&nEmpty);

    return data;
}


void TrafficBuffer::destroy()
{
    sem_destroy(&nFull);
    sem_destroy(&nEmpty);
    sem_destroy(&lock);

    free(buffer);
}

void TrafficBuffer::printCurrent()
{
    printf("Buffer:\n");
    for (int i = 0; i < buffSize; i++)
    { 
        printf("%d, %d \n", buffer[i].TrafficLightId, buffer[i].nCars);
    }
}