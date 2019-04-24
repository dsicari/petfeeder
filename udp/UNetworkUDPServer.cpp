#include "UNetwork.h"

//------------------------------------------------------------------------------
TNetworkServerUDP::TNetworkServerUDP(const char *bind, unsigned int port)
{
    memcpy(Bind, bind, sizeof(bind));
    Port=port;
    memset(&ServerAddr, 0, sizeof(ServerAddr));     
}
//------------------------------------------------------------------------------
TNetworkServerUDP::~TNetworkServerUDP()
{
    close(Sock); 
}
//------------------------------------------------------------------------------
bool TNetworkServerUDP::CreateSocket()
{
    // Creating socket file descriptor for UDP
    bool rslt=false;
    Sock = socket(AF_INET, SOCK_DGRAM, 0); 
    if(Sock < 0) { 
        rslt=false;
    }
    else{
        int optval = 1;
        setsockopt(Sock, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));
        rslt=true;
    }   
        printf("01");   
    return rslt;
}
//------------------------------------------------------------------------------
bool TNetworkServerUDP::CreateServer()
{
    bool rslt=false;
    if(Sock < 0) { 
        rslt=false;
    }
    else{
        bzero((char *) &ServerAddr, sizeof(ServerAddr));
        ServerAddr.sin_family = AF_INET;
        ServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        ServerAddr.sin_port = htons((unsigned short)Port);
        if (bind(Sock, (struct sockaddr *) &ServerAddr, sizeof(ServerAddr)) < 0){
            rslt=false;
        }
        else{
            ClientLen = sizeof(ClientAddr);
            rslt=true;
        }
    }     
    return rslt;
}
//------------------------------------------------------------------------------
bool TNetworkServerUDP::Rcv(unsigned char *bufferRcv, int lenBufferRcv, int *lenBufferRcvData)
{
    bool rslt=false;

    if(Sock < 0){
        // socket nao foi criado, retornar
        return rslt;
    }
    printf("1");
    int r = recvfrom(Sock, bufferRcv, lenBufferRcv, 0, (struct sockaddr *) &ClientAddr, &ClientLen);
    if(r >= 0){
        printf("2");
        // Recebeu bytes
        if(r > lenBufferRcv){
            // Bytes recebido maior que buffer
            *lenBufferRcvData=lenBufferRcv;
            rslt=false;
        }
        else{
            *lenBufferRcvData=r;
            pHost = gethostbyaddr((const char *)&ClientAddr.sin_addr.s_addr, sizeof(ClientAddr.sin_addr.s_addr), AF_INET);
            pHostAddr = inet_ntoa(ClientAddr.sin_addr);
            rslt=true;
        }         
    }
    return rslt;
}
//------------------------------------------------------------------------------
char * TNetworkServerUDP::GetHostName()
{
    if(pHost == NULL) return NULL;
    else return pHost->h_name;   
}
//------------------------------------------------------------------------------
char * TNetworkServerUDP::GetHostAddress()
{
    if(pHostAddr == NULL) return NULL;
    else return pHostAddr; 
}
