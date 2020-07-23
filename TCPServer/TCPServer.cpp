#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <TCPConnection.h>

using namespace std;

/** <summary>
Create and start server; calls startServer() method to listen for new clients and communicate
with existing clients. Print and raise issues if encountered.
</summary>
*/
int main()
{
	bool isServer = true;
	string msg1 = string(1, char(6)), msg2 = "another default message";
	double sendDelay = 1.0;
	TCPConnection testServer(isServer, sendDelay, msg1, msg2);
	testServer.startServer();

	/* Close the server */
	closesocket(testServer.getListener());
	WSACleanup();
	return 0;
}