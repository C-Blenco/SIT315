#include "Producer.h"

Producer::Producer(TrafficBuffer* trafficBuffer, int nTrafficLights, int nData, int nProducers, int prodId) 
{
    buffer = trafficBuffer;
    this->nTrafficLights = nTrafficLights;
    this->nData = nData;
    this->prodId = prodId;
    this->nProducers = nProducers;
}


void Producer::produce(bool output, bool input)
{
    time_t timeData = time(0);

    if (input)
    {
        std::ifstream in_stream("input.csv", std::ofstream::in);
        std::string line;
        int range_from = prodId * (nData / nProducers); // remove /5
        int range_to = range_from + (nData / nProducers); // remove /5
        if (range_to > nData) range_to = nData;
        int line_num = 0;
        TrafficData data;

        // Each line, do
        while (std::getline(in_stream, line))
        {
            line_num++;
            if (line_num > range_to) { break; }
            if (line_num < range_from) { continue; }

            std::stringstream lineStream(line);
            std::string result[3];
            int i = 0;


            printf("%s, ln: %d\n", lineStream.str().c_str(), line_num);
            while(lineStream.good())
            {
                getline(lineStream, result[i], ',');
                i++;
            }

            data = TrafficData {(time_t) strtol(result[0].c_str(), NULL, 10), atoi(result[1].c_str()), atoi(result[2].c_str())};
            printf("++producer %d: %d %d \n", prodId, data.TrafficLightId, data.nCars);

            buffer->create(data);
        }
        in_stream.close();
        printf("++ producer %d finished\n", prodId);
    }
    else
    {
        std::ofstream out_stream;
        if (output)
        {
            out_stream.open("output.csv", std::ofstream::out);
        }

        for (int i = 0; i < nData; i++)
        {
            TrafficData data;

            // Generate traffic data
            data.timeStamp = timeData + (5 * 60);
            data.nCars = rand() %20;
            data.TrafficLightId = (rand() % nTrafficLights) + 1;

            printf("producer %d: %d %d \n", prodId, data.TrafficLightId, data.nCars);

            buffer->create(data);

            if (output)
            {
                if (out_stream.is_open())
                {
                    out_stream << data.timeStamp << ',' << data.TrafficLightId << ',' << data.nCars << '\n';
                }
            }
        }

        if (output)
        {
            out_stream.close();
        }
    }
}

// void Producer::startThread()
// {
//     std::thread t(&Producer::produce, this);
//     t.detach();
// }
