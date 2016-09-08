#include "BtClient.h"

using namespace std;

BtClient::BtClient(const string& serverAddress, int rfCommChan)
    :   serverAdrStr(serverAddress), channel(rfCommChan)
{
    addres = { 0 };
	//ustawianie parametrow polaczenia
    mySocket = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    addres.rc_family = AF_BLUETOOTH;
    addres.rc_channel = (uint8_t) channel;
    str2ba(serverAdrStr.c_str(), &addres.rc_bdaddr);
}

BtClient::~BtClient()
{

}

int BtClient::Connect()
{
	//laczenie UWAGA: mozliwe jest polaczenie sie do servera na zlym kanale
	//bez informacji o bledzie
    return connect(mySocket, (struct sockaddr *)&addres, sizeof(addres));
}

void BtClient::Send(const string &msg)
{
	//przesylanie wiadomosci
    write(mySocket, msg.data(), msg.size());
}

void BtClient::Disconnect()
{
	//zamykanie polaczenia
    close(mySocket);
}
