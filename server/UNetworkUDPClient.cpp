#include "UNetwork.h"

//------------------------------------------------------------------------------
TNetworkClientUDP::TNetworkClientUDP(const char *host, unsigned int port){
    memcpy(Host, host, sizeof(host));
    Port=port;
    memset(&ServerAddr, 0, sizeof(ServerAddr));       

    ServerAddr.sin_family = AF_INET; 
    ServerAddr.sin_port = htons(Port); 
    ServerAddr.sin_addr.s_addr = inet_addr(Host); 
}
//------------------------------------------------------------------------------
TNetworkClientUDP::~TNetworkClientUDP(){
    close(Sock); 
}
//------------------------------------------------------------------------------
bool TNetworkClientUDP::CreateSocket(){
    // Creating socket file descriptor for UDP
    bool rslt=false;
    Sock = socket(AF_INET, SOCK_DGRAM, 0); 
    if(Sock < 0) { 
        rslt=false;
    }
    else{
        rslt=true;
    }    
    return rslt;
}
//------------------------------------------------------------------------------
bool TNetworkClientUDP::Send(unsigned char *bufferSend, int lenBufferSend){
    bool rslt=false;

    if(Sock < 0){
        // socket nao foi criado, retornar
        return rslt;
    }

    int r = sendto(Sock, (const unsigned char *)bufferSend, lenBufferSend, MSG_CONFIRM, (const struct sockaddr *) &ServerAddr, sizeof(ServerAddr)); 
    if(r >= 0){
        rslt=true;              
    }

    return rslt;
}
//------------------------------------------------------------------------------
bool TNetworkClientUDP::SendRcv(unsigned char *bufferSend, int lenBufferSend, unsigned char *bufferRcv, int lenBufferRcv, int *lenBufferRcvData, int timeout){
    bool rslt=false;

    if(Sock < 0){
        // socket nao foi criado, retornar
        return rslt;
    }

    int r = sendto(Sock, (const unsigned char *)bufferSend, lenBufferSend, MSG_CONFIRM, (const struct sockaddr *) &ServerAddr, sizeof(ServerAddr)); 
    if(r >= 0){
        // Enviou bytes, entao aguardar resposta
        socklen_t len_servaddr=0;
        // timeout recepcao
        struct timeval tv;
        tv.tv_sec = timeout;
        tv.tv_usec = 0;
        setsockopt(Sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
        r = recvfrom(Sock, (unsigned char *)bufferRcv, lenBufferRcv, MSG_WAITALL, (struct sockaddr *) &ServerAddr, &len_servaddr); 
        if(r >= 0){
            if(r > lenBufferRcv){
                // recebeu mais do que o buffer permite
                *lenBufferRcvData=lenBufferRcv;
                rslt=false;
            }
            else{
                *lenBufferRcvData=r;
                rslt=true;
            }                  
        }        
    }

    return rslt;
}
//------------------------------------------------------------------------------