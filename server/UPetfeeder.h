#ifndef PETFEEDER_H
#define PETFEEDER_H

//  DEFINES PROTOCOLO

#define     STX             0x02
#define     ACK             0x06
#define     NAK             0x15

//  DEFINES IDENTIFICADOR (PID) PROTOCOLO

#define     ID_REPORT       0x01
#define     ID_TESTE        0x02

// ESTRUTURAS
#pragma pack(push, 1) // nao faz alinhamento memoria 
typedef struct{
    unsigned char iniciador;            // 1 byte
    unsigned char pid;                  // 1 byte
    unsigned long seq;                  // 8 bytes
    unsigned char st;                   // 1 byte    
    unsigned long uptime;               // 8 bytes
    char sn[5];                         // 1*5 bytes
    unsigned short nivel_alm;           // 2 bytes
    unsigned short peso;                // 2 bytes
    char horario_alm[24];               // 1*24 byte
    unsigned short qtde_alm;            // 2 bytes
    unsigned short crc;                 // 2 bytes
}TPacketReport;
#pragma pack(pop) 


#endif