#ifndef BT_CLIENT_H
#define BT_CLIENT_H

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

#include <string>

class BtClient
{
public:
	BtClient(const std::string& serverAddress, int channel);
	~BtClient();

	int Connect();
	void Send(const std::string& msg);
	void Disconnect();

private:
    const std::string serverAdrStr;
    struct sockaddr_rc addres;
    int mySocket, channel;
};

#endif // !BT_CLIENT_H
