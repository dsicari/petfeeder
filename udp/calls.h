#ifndef DATA_H
#define DATA_H



#include "utl.h"
#include "rc4.h"
#include "UNetwork.h"

//  DEFINES PROTOCOLO

#define     STX                     0x02
#define     ACK                     0x06
#define     NAK                     0x15

//  DEFINES COMANDOS PROTOCOLO

#define     CMD_REPORT           0x01

// ESTRUTURAS
#pragma pack(push, 1) // nao faz alinhamento memoria 
typedef struct{
    unsigned char iniciador;            // 1 byte
    unsigned long sequencia;            // 8 bytes
    unsigned char status;               // 1 byte    
    unsigned long uptime;               // 8 bytes
    char serial[5];                     // 1*5 bytes
    unsigned short nivel_alimento;      // 2 bytes
    unsigned short peso;                // 2 bytes
    char horario_alimentacao[24];       // 1*24 byte
    unsigned short quantidade_alimento; // 2 bytes
    unsigned short crc;                 // 2 bytes
}TPacketReport;
#pragma pack(pop) 

#endif