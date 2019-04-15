#ifndef NETWORK_H
#define NETWORK_H

#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 

#include <sys/time.h>

#include <netdb.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 

#define LEN_HOST 15
//------------------------------------------------------------------------------
class TNetworkClientUDP{
    private:
        char Host[LEN_HOST];
        unsigned int Port;   
        struct sockaddr_in ServerAddr;   
        int Sock;      
         
    public:
        TNetworkClientUDP(const char *host, unsigned int port);
        ~TNetworkClientUDP();
        bool CreateSocket();
        bool Send(unsigned char *bufferSend, int lenBufferSend);
        bool SendRcv(unsigned char *bufferSend, int lenBufferSend, unsigned char *bufferRcv, int lenBufferRcv, int *lenBufferRcvData, int timeout=5);
};
//------------------------------------------------------------------------------
class TNetworkServerUDP{
    private:           
        socklen_t ClientLen; /* byte size of client's address */
        struct sockaddr_in ClientAddr; /* client addr */
        struct sockaddr_in ServerAddr; /* server's addr */
        struct hostent *pHost; /* client host info */
        char *pHostAddr; /* dotted decimal host addr string */
        char Bind[LEN_HOST];
        unsigned int Port;  
        int Sock;  
        bool Bind2Any;
         
    public:
        TNetworkServerUDP(unsigned int port);
        TNetworkServerUDP(const char *host, unsigned int port);
        ~TNetworkServerUDP();
        bool CreateSocket();
        bool CreateServer();
        bool Rcv(unsigned char *bufferRcv, int lenBufferRcv, int *lenBufferRcvData);
        bool Send(unsigned char *bufferSend, int lenBufferSend);
        char *GetClientName();
        char *GetClientAddress();
};
//------------------------------------------------------------------------------
#endif