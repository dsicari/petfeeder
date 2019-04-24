/*
 *          C L I E N T
 */

#include "../server/UPetfeeder.h"
#include "../server/UUtil.h"
#include "../server/UNetwork.h"
#include "../server/ULog.h"
#include "../server/USimpleIni.h"
#include "../server/UCrytiger.h"

int main()
{ 
    TPacketReport pkt; 
    // iniciador
    pkt.init.iniciador=STX;
    pkt.init.modo=CIPHERTEXT;
    // pacote id
    memcpy(pkt.pid.sign, "assinatura", 10);
    pkt.pid.seq=1;
    pkt.pid.st=0;
    pkt.pid.id=ID_REPORT;
    memcpy(pkt.pid.sn, "1234567890", 10);
    // pacote report
    pkt.uptime=1000;
    pkt.nivel_alm=50;
    pkt.peso=100;
    memcpy(pkt.horario_alm, "08:50;10:40;18:50;22:30", 24);
    pkt.qtde_alm=100; 

    // Calculo CRC = pkt.pid + pkt - pkt.crc - pkt.init
    pkt.crc=calccrc16((unsigned char*)&pkt + 2, sizeof(TPacketReport) - 4);
    printf("crc calculado=%d byte=0x%02X", pkt.crc, pkt.crc);    
    printf("\nsize TPacketReport=%i\n", sizeof(TPacketReport));
    printf("horario_alimentacao=%s\n", pkt.horario_alm);
    
    // serializa struct
    unsigned char buff[sizeof(TPacketReport)];
    memcpy(buff, (unsigned char*)&pkt, sizeof(TPacketReport));
    
    printf("Plaintext=");
    for (int i = 0; i < sizeof(buff); i++){
        printf("%02X", buff[i]);
    }
    
    if(pkt.init.modo == CIPHERTEXT){
        TCrytiger tiger;
        unsigned char tigerKey[10];
        memcpy(tigerKey, "Fatec2019!", 10);
        // ciphertext = pkt.pid + pkt + pkt.crc - pkt.init
        tiger.InitTiger(tigerKey, 10);
        tiger.Encrypt(&buff[2], sizeof(TPacketReport) - 2);
        printf("\nCiphertext=");
        for (int i = 0; i < sizeof(buff); i++){
            printf("%02X", buff[i]);
        }
        printf("\n");
    }

    TNetworkClientUDP clientUDP = TNetworkClientUDP("68.183.138.223",30685);
    if(clientUDP.CreateSocket() != false){
        unsigned char bufferRcv[sizeof(TPacketReport)];
        int lenBufferRcvData=0;
        bool r=false;

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
    else{
        printf("fail create socket");
    }
    printf("\nexiting...");
    return 0; 
} 