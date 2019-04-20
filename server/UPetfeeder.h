#ifndef PETFEEDER_H
#define PETFEEDER_H

//  DEFINES PROTOCOLO

#define     ACK             0x06
#define     NAK             0x15

//  DEFINES INIT PROTOCOLO

#define     STX             0x02
#define     PLAINTEXT       0x00
#define     CIPHERTEXT      0x01
#define     LEN_SIGN        10

//  DEFINES IDENTIFICADOR (PID) PROTOCOLO

#define     ID_REPORT       0x01
#define     ID_TESTE        0x02

// ESTRUTURAS
#pragma pack(push, 1) // nao faz alinhamento memoria 

typedef struct{
    unsigned char iniciador;            // 1 byte
    unsigned char modo;                 // 1 byte
}TPacketInit;

typedef struct{
    unsigned char sign[10];             // 1*10 bytes
    unsigned char id;                   // 1 byte
    unsigned long seq;                  // 8 bytes
    unsigned char st;                   // 1 byte
    char sn[10];                        // 1*10 bytes
}TPacketID;

typedef struct{  
    TPacketInit init;                   // 2 bytes
    TPacketID pid;                      // 30 bytes
    unsigned long uptime;               // 8 bytes    
    unsigned short nivel_alm;           // 2 bytes
    unsigned short peso;                // 2 bytes
    char horario_alm[24];               // 1*24 byte
    unsigned short qtde_alm;            // 2 bytes
    unsigned short crc;                 // 2 bytes
}TPacketReport;

#pragma pack(pop) 

#endif