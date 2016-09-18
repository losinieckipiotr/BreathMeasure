#ifndef APP_H
#define APP_H

#include <string>
#include <list>
#include <thread>

#include "Detector.h"

class App
{
public:
    static bool parseArgs(
        const std::list<std::string>& args,
        unsigned int& tcpPort,
        bool& bt,
        std::string& serverAdr,
        int& channel);

    void StartSample(Detector& det);
    void StopSample(Detector& det);

    std::thread sampleThread;
};

#endif // APP_H
