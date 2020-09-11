#include "Producer.h"
#include "Consumer.h"
#include "TrafficTally.h"
#include <pthread.h>
#include <map>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>

using namespace std;
int nTrafficLights;
int nProducers;
int nConsumers;
int buffSize;
int nData;
bool input;

/**
 * Functions created to pass produce and consume functions into
 * the pthread_create function.
 * 
 * Accepts the producer/consumer object pointer, and executes the
 * respective function
 */
void *executeProducer(void *object)
{
    ((Producer *)object)->produce(false, false);
    return NULL;
}

void *executeConsumer(void *object)
{
    ((Consumer *)object)->consume();
    return NULL;
}

bool sortNumCars(TrafficTally* x, TrafficTally* y) 
{ 
    return x->nCars > y->nCars;
}

int main(int argc, char** argv)
{
    // Initialise seed for random function
    srand((unsigned int)time(NULL));

    /**
     * Args:
     *  1 - number of traffic lights
     *  2 - number of producers
     *  3 - number of consumers
     *  4 - buffer size
     *  5 - number of data produced
     */
    if (argc != 6)
    {
        printf("Please enter each of the following arguments: \n");
        printf("    - Number of traffic lights \n");
        printf("    - Number of producer threads \n");
        printf("    - Number of consumer threads \n");
        printf("    - Size of the bounded-buffer \n");
        printf("    - Amount of data to produce \n");
        printf("    - Use input.csv (true/false) \n");
        exit(0);
    }
    if (argc > 1) nTrafficLights = atoi(argv[1]);
    if (argc > 2) nProducers = atoi(argv[2]);
    if (argc > 3) nConsumers = atoi(argv[3]);
    if (argc > 4) buffSize = atoi(argv[4]);
    if (argc > 5) nData = atoi(argv[5]);
    //if (argc > 6) input = argv[6] == "true";

    // if (input)
    // {
    //     ifstream f_stream("input.csv", ios::in);
    //     string line;
    //     getline(f_stream, line);  
    //     f_stream.close();

    //     stringstream ss(line);
    //     int result[5];
    //     int i = 0;

    //     while (ss.good())
    //     {
    //         string item;
    //         getline(ss, item, ',');
    //         result[i] = stoi(item);
    //         i++;
    //     }
    //     result >> nTrafficLights >> nProducers >> nConsumers >> buffSize >> nData;

    // }

    // Create traffic light vector to store a tally of cars
    vector<TrafficTally*> trafficLights;
    for (int i = 0; i < nTrafficLights; i++)
    {
        trafficLights.push_back(new TrafficTally {i + 1, 0});
    }

    // Create bounded-buffer
    TrafficBuffer* buffer = new TrafficBuffer(buffSize);

    // Create producer and consumer arrays
    typedef Producer* ProducerPtr;
    ProducerPtr* producers = new ProducerPtr[nProducers];
    typedef Consumer* ConsumerPtr;
    ConsumerPtr* consumers = new ConsumerPtr[nConsumers];

    pthread_t producerThreads[nProducers];
    pthread_t consumerThreads[nConsumers];

    // Create producer and consumer threads, and execute their produce function.
    for (int i = 0; i < nProducers; i++)
    {
        producers[i] = new Producer(buffer, nTrafficLights, nData, nProducers, i);
        pthread_create(&producerThreads[i], NULL, executeProducer, producers[i]);
    }
    for (int i = 0; i < nConsumers; i++)
    {
        consumers[i] = new Consumer(buffer, nTrafficLights, nData, trafficLights, nConsumers, i);
        pthread_create(&consumerThreads[i], NULL, executeConsumer, consumers[i]);
    }

    // Join producer and consumer threads (waits till finish)
    for (int i = 0; i < nProducers; i++)
    {
        pthread_join(producerThreads[i], NULL);
    }
    for (int i = 0; i < nConsumers; i++)
    {
        pthread_join(consumerThreads[i], NULL);
    }

    // Print data consumed by consumers
    sort(trafficLights.begin(), trafficLights.end(), sortNumCars);
    printf("Data:\n");
    for(int i = 0; i < nTrafficLights; i++)
    {
        printf("%d. Light: %d, No. Cars: %d\n", i + 1, trafficLights[i]->trafficLightId, trafficLights[i]->nCars);
    }

    

    //buffer->printCurrent();
    buffer->destroy();

    free(producers);
    free(consumers);
}
