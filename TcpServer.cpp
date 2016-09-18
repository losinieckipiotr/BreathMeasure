#include "TcpServer.h"
#include <future>
#include <iostream>
#include <exception>

using namespace std;
using namespace boost::asio::ip;

TcpServer::TcpServer(App& app, unsigned int port) :
    connected(false),
    myApp(app),
    myService(),
    myAcceptor(myService,
    boost::asio::ip::tcp::endpoint(tcp::v4(), (unsigned short)port), true),
    mySocket(myService)

{
    Accept();
    serviceTh = thread([this](){ myService.run(); });
}

TcpServer::~TcpServer()
{

}

void TcpServer::Accept()
{
    mySocket = tcp::socket(myService);
    myAcceptor.async_accept(mySocket,
    [this](const boost::system::error_code& ec)
    {
        if(!ec)
        {
            myApp.StopSample();
            myApp.StartSample();
            cout << "Connected" << endl;
            cout << "Restarting measure..." << endl;
            boost::asio::ip::tcp::no_delay option(true);
            mySocket.set_option(option);
            connected = true;
            Recive();
        }
        else
        {
            cout << "Accept Error!" << endl;
            connected = false;
        }
    });
}

void TcpServer::Recive()
{
    char data[1];
    mySocket.async_read_some(boost::asio::buffer(data, 1),
    [this](const boost::system::error_code, std::size_t)
    {
        Send("-1");
        connected = false;
        mySocket.close();
        cout << "Disconneted" << endl;
        Accept();
    });
}

void TcpServer::Send(const string& msg)
{
    if(connected)
    {
        boost::asio::async_write(mySocket, boost::asio::buffer(msg.data(), msg.size()),
        [this](boost::system::error_code ec, std::size_t)
        {
            if(ec)
            {
                if(connected)
                {
                    connected = false;
                    cout << "Disconneted" << endl;
                    Accept();
                }
            }
        });
    }
}

void TcpServer::Shutdown()
{
    connected = false;
    mySocket.close();
    myService.stop();
    serviceTh.join();
}
