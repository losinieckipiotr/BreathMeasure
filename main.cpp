#include <iostream>
#include <string>
#include <list>
#include <exception>
#include <functional>
#include <thread>

#include "BtClient.h"
#include "Detector.h"
#include "TcpServer.h"

using namespace std;

bool parseArgs(
    const list<string>& args,
    string& serverAdr,
    int& channel,
    bool& bt,
    unsigned int& tcpPort)
{
    //sprawdzanie i zmiana argumentow
    auto it = args.begin();
    auto argsEnd = args.end();
    while (it != argsEnd)
    {
        //adres mac telefonu
        if (*it == "-adr")
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
        //bez bluetooth
        else if(*it == "-nobt")
        {
            bt = false;
        }
        else if(*it == "-tcp")
        {
            ++it;
            if (it != argsEnd)
                tcpPort = stoul(*it);
            else
                return false;
        }
        else if(*it == "-h")
        {
            cout << "brdet v1.0" << endl;
            cout << "Usage:" << endl;
            cout << "\tbrdet [-adr <address>] [-ch <channel>] [-nobt] [-tcp <port>] | [-h]" << endl;
            exit(0);
        }
        //nieznany argument
        else
            return false;
        ++it;
    }
    return true;
}

int main(int argc, char **argv)
{
    try
    {
        //domyslne wartosci
        string serverAdr = "58:3F:54:51:C3:6F";
        int channel = 5;
        bool bt = true;
        unsigned int tcpPort = 0;

        //tworznie listy argumentow
		list<string> args;
		for (int i = 1; i < argc; i++)
		{
			args.push_back(argv[i]);
		}
		//parsowanie
		if(parseArgs(args, serverAdr, channel, bt, tcpPort) == false)
            throw runtime_error("Bad arguments!");

        TcpServer* tcpServer = nullptr;
        if(tcpPort != 0)
        {
            tcpServer = new TcpServer(tcpPort);
            cout << "Starting TCP server..." << endl;
        }

        BtClient* btClient = nullptr;
        if(bt == true)
        {
            btClient = new BtClient(serverAdr, channel);
            cout << "Connecting to rfcomm server..." << endl;
            int s = btClient->Connect();
            if(s != 0)
                throw runtime_error("Cannot conect to rfcomm server!");
        }

        function<void(double)> handler = [tcpServer, btClient](double val)
        {
            cout << val << endl;
            if(btClient != nullptr)
                btClient->Send(to_string(val));
            if(tcpServer != nullptr)
                tcpServer->Send(to_string(val));
        };

        Detector det(handler);
        det.Init();
        cout << "Starting measure..." << endl;
        cout << "Press Enter to exit." << endl;

        //uruchomienie watka probkujacego
        thread sampleTh = thread([&det]{ det.StartSample(); });

        //wcisniecie klawisz Enter powoduje zamkniecie programu
        cin.get();
        //zatrzymanie probkowania
        det.StopSample();
        //synchronizacja
        sampleTh.join();
        //zamkniecie polaczanie BT
        if(btClient != nullptr)
            btClient->Disconnect();
        if(tcpServer != nullptr)
            tcpServer->Shutdown();
	}
	catch(exception& e)
	{
        cout << e.what() << endl;
        return 1;
	}

    return 0;
}
