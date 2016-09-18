#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <string>
#include <thread>

#include <boost/asio.hpp>

#include "App.h"

class TcpServer
{
public:
	TcpServer(App& app, unsigned int port);
	~TcpServer();

	void Send(const std::string& msg);
	void Shutdown();

private:
    void Accept();
    void Recive();

    bool connected;

    App& myApp;

    boost::asio::io_service myService;
    boost::asio::ip::tcp::acceptor myAcceptor;
    boost::asio::ip::tcp::socket mySocket;

    std::thread serviceTh;

};

#endif // !TCP_SERVER_h
