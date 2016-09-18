#include "App.h"

using namespace std;

bool App::parseArgs(
        const list<string>& args,
        unsigned int& tcpPort,
        bool& bt,
        string& serverAdr,
        int& channel)
    {
        //sprawdzanie i zmiana argumentow
        auto it = args.begin();
        auto argsEnd = args.end();
        while (it != argsEnd)
        {
            if(*it == "-tcp")
            {
                ++it;
                if (it != argsEnd)
                    tcpPort = stoul(*it);
                else
                    return false;
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
                cout << "brdet v1.0" << endl;
                cout << "Usage:" << endl;
                cout << "\tBreathMeasure [-tcp <port>] [-bt [-adr <address>] [-ch <channel>]] | [-h]" << endl;
                exit(0);
            }
            //nieznany argument
            else
                return false;
            ++it;
        }
        return true;
    }

    //uruchomienie watka probkujacego
    void App::StartSample(Detector& det)
    {
        //sampleThread = thread([&det]{ det.Sample(); });
    }

    void App::StopSample(Detector& det)
    {
        //det.StopSample();
        //sampleThread.join();
    }
