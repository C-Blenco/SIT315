#include "Consumer.h"

Consumer::Consumer(TrafficBuffer* trafficBuffer, int nTrafficSignals, int nData, vector<TrafficTally*> trafficLights, int nConsumers, int consId) 
{
    buffer = trafficBuffer;
    this->nData = nData;
    this->trafficLights = trafficLights;
    this->consId = consId;
    this->nConsumers = nConsumers;
}

void Consumer::consume()
{
    for (int i = 0; i < nData; i++)
    {
        TrafficData data = buffer->remove();

        printf("--consumer %d: %d %d \n", consId, data.TrafficLightId, data.nCars);
        
        trafficLights[data.TrafficLightId - 1]->update(data.nCars);
        printf("cons %d, %d\n", consId, i);
    }
    printf("--consumer %d finished\n", consId);
}