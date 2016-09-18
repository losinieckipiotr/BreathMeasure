#include "App.h"

using namespace std;

bool App::parseArgs(
        const list<string>& args,
        int& tcpPort,
        bool& bt,
        string& serverAdr,
        int& channel)
    {
        //sprawdzanie i zmiana argumentow
        auto it = args.begin();
        auto argsEnd = args.end();
        while (it != argsEnd)
        {
            if (*it == "-tcp")
            {
                ++it;
                if (it != argsEnd)
                    tcpPort = stoul(*it);
                else
                    return false;
            }
            //wylacz tcp
            if (*it == "-notcp")
            {
                tcpPort = -1;
            }
            //wlacz bluetooth
            else if(*it == "-bt")
            {
                bt = true;
            }
            //adres mac telefonu
            else if (*it == "-adr")
            {
                ++it;
                if (it != argsEnd)
                    serverAdr = *it;
                else
                    return false;
            }
            //numer kanalu rfcomm
            else if (*it == "-ch")
            {
                ++it;
                if (it != argsEnd)
                    channel = stoi(*it);
                else
                    return false;
            }
            else if(*it == "-h")
            {
                cout << "BreathMeasure v1.0" << endl;
                cout << "Usage:" << endl;
                cout << "BreathMeasure [[[-tcp <port>] | -notcp] [-bt [-adr <address>] [-ch <channel>]]] | [-h]" << endl;
                exit(0);
            }
            //nieznany argument
            else
                return false;
            ++it;
        }
        return true;
    }

    void App::SetDetector(Detector& detector)
    {
        det = &detector;
    }

    void App::StartSample()    {
        lock_guard<mutex> lck(appMut);
        if(sampleFlag == false)
        {
            sampleThread = thread([this] { det->Sample(); });
            sampleFlag = true;
        }
    }

    void App::StopSample()
    {
        lock_guard<mutex> lck(appMut);
        if(sampleFlag == true)
        {
            det->StopSample();
            sampleThread.join();
            sampleFlag = false;
        }
    }
