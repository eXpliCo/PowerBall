#include "ServerConnection.h"
using namespace std;
ServerConnection::ServerConnection()
{
	WSADATA wsaData = {0};
	WSAStartup(MAKEWORD(2,2), &wsaData);
	this->mPort = 10000;
	this->mIp = "127.0.0.1";
	this->mServerSocket = new Connection(socket( AF_INET, SOCK_STREAM, IPPROTO_IP ));

    //u_long noBlocking = 1;
    //ioctlsocket(this->mServerSocket->sock, FIONBIO, &noBlocking);

	this->mServer = false;
	this->mNumClients = 0;
}
ServerConnection::~ServerConnection()
{
	closesocket(this->mServerSocket->sock);
	if(this->mServer)
	{
		DWORD exitCode;
		GetExitCodeThread(this->mServerSocket->handle, &exitCode);
		ExitThread(exitCode);
		CloseHandle(this->mServerSocket->handle);
		for(int i = 0; i < this->mNumClients; i++)
		{
			GetExitCodeThread(this->mClientSocket[i]->handle, &exitCode);
			ExitThread(exitCode);
			CloseHandle(this->mClientSocket[i]->handle);
			closesocket(this->mClientSocket[i]->sock);
		}
	}
	WSACleanup();
}
void ServerConnection::SetIP(char ip[])
{
	this->mIp = ip;
}
void ServerConnection::SetPort(int port)
{
	this->mPort = port;
}
void ServerConnection::InitializeConnection()
{
	sockaddr_in saServer = {0};
	saServer.sin_family = AF_INET;
	saServer.sin_port = htons(this->mPort);
	saServer.sin_addr.s_addr = inet_addr(this->mIp);
	connect(this->mServerSocket->sock, (sockaddr*)&saServer, sizeof( sockaddr ));
	int a = ::WSAGetLastError();
	bool err = WSAEWOULDBLOCK == a;
	fd_set write;
	
	FD_ZERO(&write);
    FD_SET(this->mServerSocket->sock, &write);

    TIMEVAL timeOut;
	timeOut.tv_sec = 10;
	
	int res = select(0,  NULL, &write, NULL, &timeOut);

	if(res != 0)
	{
		this->mServer = false;
		if(FD_ISSET(this->mServerSocket->sock, &write))
        {
			DWORD n;
			//CreateThread(0, 0, &TalkToServer, (void*) this->mServerSocket, 0, &n) ;
        }

	}
	else
	{
		closesocket(this->mServerSocket->sock);
		this->mServerSocket->sock = socket( AF_INET, SOCK_STREAM, IPPROTO_IP );
		//u_long noBlocking = 1;
		//ioctlsocket(this->mServerSocket->sock, FIONBIO, &noBlocking);
		this->mServer = true;
		sockaddr_in saListen = {0};
		saListen.sin_family = AF_INET;
		saListen.sin_port = htons(this->mPort);
		saListen.sin_addr.s_addr = htonl(INADDR_ANY);
		bind(this->mServerSocket->sock, (sockaddr*)&saListen, sizeof(sockaddr_in));
		listen(this->mServerSocket->sock, SOMAXCONN);

        DWORD n;
		this->mServerSocket->handle = CreateThread(0, 0, &ListenForClient, (void*) this, 0, &n);
	}
}
DWORD WINAPI ServerConnection::ListenForClient(void* param)
{
	while(true)
	{
		ServerConnection* sc = (ServerConnection*) param;
		sockaddr_in saClient = {0};
		int nSALen = sizeof(sockaddr);
		int ret = 0;

		SOCKET sock = accept(sc->mServerSocket->sock, (sockaddr*)&saClient, &nSALen );

		if(sock != INVALID_SOCKET)
		{
			ret = 1;
			Connection* client = new Connection(sock);
			sc->mClientSocket.push_back(client);
			sc->mNumClients++;

			//u_long noBlocking = 1;
			//ioctlsocket(sock, FIONBIO, &noBlocking);

		
			DWORD n;
			client->handle = CreateThread(0, 0, &TalkToClient, (void*) client, 0, &n) ;
		}
	}
	return 0;
}
void ServerConnection::Read()
{
	if(this->mServer)
	{
		for(int i = 0; i < this->mNumClients; i++)
		{
			int numBytes = recv(this->mClientSocket[i]->sock, this->mClientSocket[i]->buf + this->mClientSocket[i]->numCharsInBuf, BUFFER_SIZE - this->mClientSocket[i]->numCharsInBuf, 0);
			this->mClientSocket[i]->numCharsInBuf += numBytes;
		}
	}
	else
	{
		int numBytes = recv(this->mServerSocket->sock, this->mServerSocket->buf + this->mServerSocket->numCharsInBuf, BUFFER_SIZE - this->mServerSocket->numCharsInBuf, 0);
		this->mServerSocket->numCharsInBuf += numBytes;
	}
}
void ServerConnection::Write()
{
	if(this->mServer)
	{
		for(int i = 0; i < this->mNumClients; i++)
		{
			int numBytes = send(this->mClientSocket[i]->sock, this->mClientSocket[i]->bufW, this->mClientSocket[i]->numCharsInBufW, 0 );
			if(numBytes != this->mClientSocket[i]->numCharsInBufW) 
			{        
				this->mClientSocket[i]->numCharsInBufW -= numBytes;
				memmove(this->mClientSocket[i]->bufW, this->mClientSocket[i]->bufW + numBytes, this->mClientSocket[i]->numCharsInBufW);
			}
			else this->mClientSocket[i]->numCharsInBufW = 0;
		}
	}
	else
	{
		int numBytes = send(this->mServerSocket->sock, this->mServerSocket->bufW, this->mServerSocket->numCharsInBufW, 0 );
		if(numBytes != this->mServerSocket->numCharsInBufW) 
		{        
			this->mServerSocket->numCharsInBufW -= numBytes;
			memmove(this->mServerSocket->bufW, this->mServerSocket->bufW + numBytes, this->mServerSocket->numCharsInBufW);
		}
		else this->mServerSocket->numCharsInBufW = 0;
	}
}

void ServerConnection::SetupFDSets(fd_set& ReadFDs, fd_set& WriteFDs, fd_set& ExceptFDs) 
{
    FD_ZERO(&ReadFDs);
    FD_ZERO(&WriteFDs);
    FD_ZERO(&ExceptFDs);

    
	if(this->mServer)
	{
		//FD_SET(this->mServerSocket->sock, &ReadFDs);
		//FD_SET(this->mServerSocket->sock, &ExceptFDs);

		for(int i = 0; i < this->mNumClients; i++)
		{
			if (this->mClientSocket[i]->numCharsInBuf < BUFFER_SIZE) 
			{
				FD_SET(this->mClientSocket[i]->sock, &ReadFDs);
			}

			if (this->mClientSocket[i]->numCharsInBufW > 0) 
			{
				FD_SET(this->mClientSocket[i]->sock, &WriteFDs);
			}

			FD_SET(this->mClientSocket[i]->sock, &ExceptFDs);
		}
	}
	else
	{
		
		if (this->mServerSocket->numCharsInBuf < BUFFER_SIZE) 
		{
			FD_SET(this->mServerSocket->sock, &ReadFDs);
		}

		if (this->mServerSocket->numCharsInBufW > 0) 
		{
			FD_SET(this->mServerSocket->sock, &WriteFDs);
		}

		FD_SET(this->mServerSocket->sock, &ExceptFDs);
	}

}

void ServerConnection::Update()
{
	
	fd_set ReadFDs, WriteFDs, ExceptFDs;
	SetupFDSets(ReadFDs, WriteFDs, ExceptFDs);
	if(this->mServer)
	{
		
		TIMEVAL timeOut;
		timeOut.tv_sec = 0;
		if (select(0, &ReadFDs, &WriteFDs, NULL, &timeOut) > 0) 
		{
			if (FD_ISSET(this->mServerSocket->sock, &ReadFDs)) 
			{
				//this->ListenForClient();
			}
		
		
			for(int i = 0; i < this->mNumClients; i++)
			{

				if (FD_ISSET(this->mClientSocket[i]->sock, &ReadFDs)) 
				{
					Read();
					FD_CLR(this->mClientSocket[i]->sock, &ReadFDs);
				}
				if (FD_ISSET(this->mClientSocket[i]->sock, &WriteFDs)) 
				{
					Write();
					FD_CLR(this->mClientSocket[i]->sock, &WriteFDs);
				}
			}
		}
	}
	else
	{
		TIMEVAL timeOut;
		timeOut.tv_sec = 0;
		if (select(0, &ReadFDs, &WriteFDs, NULL, 0) > 0) 
		{
			if (FD_ISSET(this->mServerSocket->sock, &ReadFDs)) 
			{
				Read();
				FD_CLR(this->mServerSocket->sock, &ReadFDs);
			}
			if (FD_ISSET(this->mServerSocket->sock, &WriteFDs)) 
			{
				Write();
				FD_CLR(this->mServerSocket->sock, &WriteFDs);
			}
		}
		
	}

}

bool ServerConnection::GetReadBuffer(char* bufOut, int size, int clientIndex)
{
	bool somethingToRead = false;
	if(this->mServer)
	{
		if(clientIndex >= 0 && clientIndex < this->mNumClients)
		{
			if(this->mClientSocket[clientIndex]->numCharsInBuf > 0)
			{
				for(int i = 0; i < size; i++)
					bufOut[i] = this->mClientSocket[clientIndex]->buf[i];
				this->mClientSocket[clientIndex]->numCharsInBuf = 0;
				somethingToRead = true;
			}
		}
	}
	else
	{
		if(this->mServerSocket->numCharsInBuf > 0)
		{
			for(int i = 0; i < size; i++)
				bufOut[i] = this->mServerSocket->buf[i];
			this->mServerSocket->numCharsInBuf = 0;
			somethingToRead = true;
		}
	}
	return somethingToRead;
}
void ServerConnection::SetWriteBuffer(char* buf, int size, int clientIndex)
{
	if(this->mServer)
	{
		if(clientIndex >= 0 && clientIndex < this->mNumClients)
		{
			for(int i = 0; i < size; i++)
				this->mClientSocket[clientIndex]->bufW[i] = buf[i];
			this->mClientSocket[clientIndex]->numCharsInBufW = size;
		}
	}
	else
	{
		for(int i = 0; i < size; i++)
			this->mServerSocket->bufW[i] = buf[i];
		this->mServerSocket->numCharsInBufW = size;
	}
}
DWORD WINAPI ServerConnection::TalkToClient(void* param)
{
	while(true)
	{
		Connection* conn = (Connection*)param;
		

		int numBytes = recv(conn->sock, conn->buf + conn->numCharsInBuf, BUFFER_SIZE - conn->numCharsInBuf, 0);
		conn->numCharsInBuf += numBytes;
		
		numBytes = 0;

		if(conn->numCharsInBufW > 0)
		{
			numBytes = send(conn->sock, conn->bufW, conn->numCharsInBufW, 0 );
			if(numBytes != conn->numCharsInBufW) 
			{        
				conn->numCharsInBufW -= numBytes;
				memmove(conn->bufW, conn->bufW + numBytes, conn->numCharsInBufW);
			}
			else conn->numCharsInBufW = 0;
		}
			
	}

	return 0;
}
DWORD WINAPI ServerConnection::TalkToServer(void* param)
{
	while(true)
	{
		Connection* conn = (Connection*)param;
	
		int numBytes = recv(conn->sock, conn->buf + conn->numCharsInBuf, BUFFER_SIZE - conn->numCharsInBuf, 0);
		conn->numCharsInBuf += numBytes;

		if(conn->numCharsInBufW > 0)
		{
			numBytes = 0;
		
			numBytes = send(conn->sock, conn->bufW, conn->numCharsInBufW, 0 );
			if(numBytes != conn->numCharsInBufW) 
			{        
				conn->numCharsInBufW -= numBytes;
				memmove(conn->bufW, conn->bufW + numBytes, conn->numCharsInBufW);
			}
			else conn->numCharsInBufW = 0;
		}
		
	}
	return 0;
}