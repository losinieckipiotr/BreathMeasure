#ifndef APP_H
#define APP_H

#include <string>
#include <list>
#include <thread>
#include <mutex>

#include "Detector.h"

class App
{
public:
    static bool parseArgs(
        const std::list<std::string>& args,
        int& tcpPort,
        bool& bt,
        std::string& serverAdr,
        int& channel);

    void SetDetector(Detector& detector);

    void StartSample();
    void StopSample();

private:
    bool sampleFlag = false;
    Detector* det  = nullptr;
    std::thread sampleThread;
    std::mutex appMut;
};

#endif // APP_H
