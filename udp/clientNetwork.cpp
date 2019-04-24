#include "UNetwork.h"
#include "calls.h"

// Driver code 
int main() { 
    TPacketReport pcktReport;
    pcktReport.iniciador=STX;
    pcktReport.sequencia=1;
    pcktReport.status=CMD_REPORT;
    pcktReport.uptime=1000;
    memcpy(pcktReport.serial, "AAAA\0", 24);
    pcktReport.nivel_alimento=50;
    pcktReport.peso=100;
    memcpy(pcktReport.horario_alimentacao, "08:50;10:40;18:50;22:30\0", 24);
    pcktReport.quantidade_alimento=100; 

    pcktReport.crc=calccrc16((unsigned char*)&pcktReport, sizeof(TPacketReport) - 2);
    printf("crc calculado=%d byte=0x%02X", pcktReport.crc, pcktReport.crc);    
    printf("\nsize TPacketReport=%i\n", sizeof(TPacketReport));
    printf("horario_alimentacao=%s\n", pcktReport.horario_alimentacao);

    // serializa struct
    unsigned char buff[sizeof(TPacketReport)];
    memcpy(buff, (unsigned char*)&pcktReport, sizeof(TPacketReport));

    TNetworkClientUDP clientUDP = TNetworkClientUDP("0.0.0.0", 8080);
    if(clientUDP.CreateSocket() != false){
        unsigned char bufferRcv[sizeof(TPacketReport)];
        int lenBufferRcvData=0;
        bool r=false;

        printf("Sending...");
        for (int i = 0; i < sizeof(buff); i++){
            printf("%02X", buff[i]);
        }
        
        r = clientUDP.SendRcv(buff, sizeof(buff), bufferRcv, sizeof(bufferRcv), &lenBufferRcvData);
        if(r == false){
            printf("SendRcv fails...\n");
        }
        else{
            printf("Server response bytes(%d)\n", lenBufferRcvData); 
            for (int i = 0; i < lenBufferRcvData; i++){
                printf("%02X", bufferRcv[i]);
            }
            printf("\n");
        }
        
    }

    clientUDP.~TNetworkClientUDP();
    return 0; 
} 