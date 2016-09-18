#include <iostream>
#include <functional>
#include <exception>
#include <stdexcept>

#include "App.h"
#include "BtClient.h"
#include "Detector.h"
#include "TcpServer.h"

using namespace std;

int main(int argc, char **argv)
{
    try
    {
        //domyslne wartosci
        string serverAdr = "58:3F:54:51:C3:6F";
        int channel = 5;
        bool bt = false;
        int tcpPort = 2016;
        App myApp;

        //tworznie listy argumentow
		list<string> args;
		for (int i = 1; i < argc; i++)
		{
			args.push_back(argv[i]);
		}

		//parsowanie
		if(myApp.parseArgs(args, tcpPort, bt, serverAdr, channel) == false)
            throw runtime_error("Bad arguments!");

        TcpServer* tcpServer = nullptr;
        if(tcpPort > 0)
        {
            tcpServer = new TcpServer(myApp, tcpPort);
            cout << "Starting TCP server on port: " << tcpPort << endl;
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

        //przekazanie handlera
        Detector det(handler);
        //inicjalizacja detektora
        det.Init();
        cout << "Starting measure..." << endl;
        cout << "Press Enter to exit." << endl;

        myApp.SetDetector(det);
        //uruchomienie watka probkujacego
        myApp.StartSample();
        //wcisniecie klawisz Enter powoduje zamkniecie programu
        cin.get();
        //zatrzymanie probkowania
        myApp.StopSample();
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
