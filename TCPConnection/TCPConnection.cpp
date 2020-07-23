#include "TCPConnection.h"
#pragma once

/**
<summary>
Function to print out time in HH:MM:SS:MMM format in local time.
</summary>
<returns></returns>
*/
void TCPConnection::printTime()
{
	SYSTEMTIME currTime;
	GetLocalTime(&currTime);
	printf("%02d:%02d:%02d:%03d", currTime.wHour, currTime.wMinute, currTime.wSecond, currTime.wMilliseconds);
	DWORD end = GetTickCount();
	printf(" (%03d ms): ", end - _lastEvent);
	_lastEvent = end;
}

/**
<summary>
Function to print out chars that can't be printed inherently.
</summary>
<returns></returns>
<param name = "c">Char to be printed</param>
*/
void TCPConnection::printChar(char c)
{
	if (c == char(0))
	{
		printf("<NUL>");
	}
	else if (c == char(1))
	{
		printf("<SOH>");
	}
	else if (c == char(2))
	{
		printf("<STX>");
	}
	else if (c == char(3))
	{
		printf("<ETX>");
	}
	else if (c == char(4))
	{
		printf("<EOT>");
	}
	else if (c == char(5))
	{
		printf("<ENQ>");
	}
	else if (c == char(6))
	{
		printf("<ACK>");
	}
	else if (c == char(21))
	{
		printf("<NAK>");
	}
	else if (c == char(10))
	{
		printf("<LF>");
	}
	else if (c == char(13))
	{
		printf("<CR>");
	}
	else {
		printf("%c", c);
	}
}

/**
<summary>
Print the message sent/received char by char using printChar() method above.
</summary>
<returns></returns>
<param name = "msg">Complete message to print out.</param>
*/
void TCPConnection::printMessage(string msg)
{
	for (unsigned int i = 0; i < msg.length(); i++)
	{
		printChar(msg[i]);
	}
	cout << endl;
}

/**
<summary>
Timer to delay sending a message
</summary>
<returns></returns>
<param name = "time">Amount of time to delay </param>
*/
void TCPConnection::timeDelay(double time)
{
	Sleep(time * 1000);
}


/**
<summary>
Initialize Winsock for the client--print error if failure on startup
or getting addressinfo.
</summary>
<returns></returns>
*/
void TCPConnection::initWinsock()
{
	_works = WSAStartup(MAKEWORD(2, 2), &_wsaData); // Same as client
	if (_works != 0)
	{
		cout << "Error on WSAStartup" << endl;
		return; // Indicates error encountered
	}
	ZeroMemory(&_hints, sizeof(_hints)); // Fills block of memory w/ 0's
	_hints.ai_family = AF_INET;
	_hints.ai_socktype = SOCK_STREAM;
	_hints.ai_protocol = IPPROTO_TCP;

	/* Getting local address/port */
	//_works = getaddrinfo(NULL, DEFAULT_PORT, &_hints, &_result);
	_works = getaddrinfo(NULL, "1787", &_hints, &_result);
	if (_works != 0)
	{
		cout << "Error on getaddrinfo" << endl;
		WSACleanup();
		return;
	}
}

/**
<summary>
Creates a connecting socket and connects to the server from a client.
Prints an error message if error occurs during socket creation.
</summary>
<returns> -1 if an error occurs, 0 if no error occurs </returns>
*/
int TCPConnection::findClientMessager()
{
	_messager = INVALID_SOCKET;
	for (_ptr = _result; _ptr != NULL; _ptr = _ptr->ai_next)
	{

		/* Create the connecting socket */
		_messager = socket(_ptr->ai_family, _ptr->ai_socktype, _ptr->ai_protocol); // Create socket
		if (_messager == INVALID_SOCKET)
		{
			cout << "Error during socket creation" << endl;
			WSACleanup();
			return -1;
		}
		printTime();
		cout << ": Created connecting socket" << endl;
		/* Connecting to the server */
		_works = connect(_messager, _ptr->ai_addr, (int)_ptr->ai_addrlen);
		if (_works == SOCKET_ERROR)
		{
			closesocket(_messager);
			_messager = INVALID_SOCKET;
			continue;
		}
		else
		{
			printTime();
			cout << ": Connected to the server" << endl;
		}
		break;
	}
	freeaddrinfo(_result);
	if (_messager == INVALID_SOCKET)
	{
		cout << "Error during server connection" << endl;
		WSACleanup();
		return -1;
	}
	printTime();
	cout << "Success: client initiated!" << endl;
	return 0;
}


/**
<summary>
Runs initWinsock() and findClientMessager()
</summary>
<return></return>
*/
void TCPConnection::startClient()
{
	initWinsock();
	findClientMessager();
}

/**
<summary>
Initialize Winsock. Then, create, bind, and start listening from a listener
socket, printing any errors if encountered along the way.
</summary>
<return></return>
*/
void TCPConnection::startServer()
{
	/* Initialize Winsock */
	int start;
	sockaddr_in SERVER;
	SERVER.sin_family = AF_INET;
	SERVER.sin_addr.s_addr = INADDR_ANY;
	//SERVER.sin_port = htons(stoi(DEFAULT_PORT));
	SERVER.sin_port = htons(1787);
	start = WSAStartup(MAKEWORD(2, 2), &_wsaData);
	if (start != 0)
	{
		cout << "Error on WSAStartup: " << start << endl;
	}
	/* Create socket that will connect to server */
	_listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (_listener == INVALID_SOCKET)
	{
		cout << "Error creating socket to connect to server: " << WSAGetLastError() << endl;
		WSACleanup();
	}

	/* Bind the socket */
	start = bind(_listener, (sockaddr*)&SERVER, sizeof(SERVER));
	if (start == SOCKET_ERROR)
	{
		cout << "Error on bind:" << WSAGetLastError() << endl;
		closesocket(_listener);
		WSACleanup();
	}
	/* Create the listener socket */
	start = listen(_listener, SOMAXCONN);
	if (start == SOCKET_ERROR)
	{
		cout << "Error on entering the listening state: " << start << endl;
		closesocket(_listener);
		WSACleanup();
	}
	printTime();
	cout << "Server entered listening state" << endl;

	/* Create the thread */
	DWORD threadID;
	sockaddr_in client;
	int clientSize = sizeof(client);
	while (true)
	{
		_messager = accept(_listener, (struct sockaddr*)&client, &clientSize);
		printTime();
		cout << "Client Connection success!" << endl;
		CreateThread(NULL, 0, redirect, (LPVOID)this, 0, &threadID);
	}
	printTime();
	cout << "SUCCESS: Server initiated!" << endl;
}



/**
<summary>
Account for the desired time delay of message. Send the message specified to the desired
socket for both client/server. Print the time message was sent.
</summary>
<returns> Returns -1 if error encountered, return length of message if successful </returns>
<param name = "messager">The socket through which to send a message</param>
<param name = "message">Message to be sent</param>
*/
int TCPConnection::sendMessage(SOCKET messager, string message)
{
	timeDelay(_sendDelay);
	_works = send(messager, message.c_str(), message.length(), 0);
	printTime();

	if (_works == SOCKET_ERROR)
	{
		cout << "Error on sending message" << endl;
		closesocket(messager);
		WSACleanup();
		return -1;
	}
	else
	{
		if (_isServer)
		{
			cout << "SERVER sends to client " << messager << " : ";
		}
		else
		{
			cout << "CLIENT sends to server: ";
		}
		printMessage(message);
	}
	_sender = false;
	_sendCounter++;
	return _works;
}

/**
<summary>
Use sendMessage() to send EOT character as a message.
</summary>
<return> Returns -1 if error encountered, return 1 if successful </return>
<param name = "messager">The socket through which to send EOT message</param>
*/
int TCPConnection::sendEOT(SOCKET messager)
{
	return sendMessage(messager, EOT);
}

/**
<summary>
Use sendMessage() to send ENQ character as a message.
</summary>
<return> Returns -1 if error encountered, return 1 if successful </return>
<param name = "messager">The socket through which to send ENQ message</param>
*/
int TCPConnection::sendENQ(SOCKET messager)
{
	return sendMessage(messager, ENQ);
}

/**
<summary>
Use sendMessage() to send ACK character as a message.
</summary>
<return> Returns -1 if error encountered, return 1 if successful </return>
<param name = "messager">The socket through which to send ACK message</param>
*/
int TCPConnection::sendACK(SOCKET messager)
{
	return sendMessage(messager, ACK);
}

/**
<summary>
Client/Server attempts to receive a message. Print time and message if successful.
If received 0 bytes, print/indicate a disconnection.
</summary>
<return> Returns -1 if error encountered, return 1 if successful </return>
<param name = "messager">The socket through which to receive a message</param>
*/
int TCPConnection::receiveMessage(SOCKET messager)
{
	char receiveMsg[DEFAULT_LENGTH];
	int rWorks = recv(messager, receiveMsg, (int)strlen(receiveMsg), 0);
	if (rWorks > 0)
	{
		printTime();
		if (_isServer)
		{
			cout << "SERVER received from client " << messager << " : ";
		}
		else
		{
			cout << "CLIENT received from server: ";
		}
		string msg;
		msg.assign(receiveMsg, rWorks);
		printMessage(msg);
		_sender = true;
		return rWorks;
	}
	else if (rWorks == 0)
	{
		printTime();
		if (_isServer)
		{
			cout << "Client " << messager << " disconnected" << endl;
		}
		else
		{
			cout << "SERVER disconnected." << endl;
		}
		return 0;
	}
	return -1;
}


/**
<summary>
Takes in an instance of TCPConnection class as a static method to call exchange()
as an instance method.
</summary>
<return> Returns the result of exchange() </return>
<param name = "lpParam">Provides a pseudonym of THIS</param>
*/
DWORD WINAPI TCPConnection::redirect(LPVOID lpParam)
{
	return ((TCPConnection*)lpParam)->exchange();
}

/**
<summary>
Server's send/receive method: continually send/receive data through messager
socket while client is available.
</summary>
<return> ExitsThread(0) if client closed or error on send/receive </return>
*/
DWORD TCPConnection::exchange()
{
	SOCKET messager = (SOCKET)_messager;
	//char receiver[DEFAULT_LENGTH];
	while (true)
	{
		int bytesReceived = receiveMessage(messager);
		if (bytesReceived == 0)
		{
			closesocket(messager);
			ExitThread(0);
		}
		else if (bytesReceived > 0)
		{
			int bytesSent = sendMessage(messager, _msg1);
			if (bytesSent == -1)
			{
				closesocket(messager);
				ExitThread(0);
			}
		}
		else
		{
			cout << "Error on receive from socket " << messager << endl;
			closesocket(messager);
			ExitThread(0);
		}
	}
}
/**
<summary>
Create a new client in the _clients table, based on the number
of clients already present in the table and the SOCKET ID.
</summary>
<returns></returns>
<param name = "x">The position in _clients table </param>
<param name = "s">The socket ID</param>
*/
void TCPConnection::makeClient(int x, SOCKET s)
{
	setOneClientClient(x, s);
	setOneClientConnected(x, true);
	setOneClientSendACK(x, false);
	setOneClientReceiveACK(x, false);
	if (_isServer)
	{
		_numClients++;
		_numClientsConnected++;
		printTime();
		cout << "New client: " << s << endl;
	}
}

/**
<summary>
Function to shutdown a client, raising error if one is encoutnered.
</summary>
<returns>-1 if error encountered, 0 otherwise </returns>
*/
int TCPConnection::clientShutdown()
{
	/* Shutdown connection for sending */
	_works = shutdown(getMessager(), SD_SEND);
	if (_works == SOCKET_ERROR) {
		cout << "Error on shutdown: " << WSAGetLastError() << endl;
		closesocket(getMessager());
		WSACleanup();
		return 1;
	}

	/* Shutdown and cleanup */
	closesocket(getMessager());
	WSACleanup();
	printTime();
	cout << ": Closed and shutdown" << endl;

	/* Safe exit */
	printf("Enter any number to exit: ");
	int ok;
	cin >> ok;
	return 0;
}