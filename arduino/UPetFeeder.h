#ifndef PETFEEDER_H
#define PETFEEDER_H

// Definir arduino para unsigned long int = 8bytes
#define     IS_ARDUINO
// Definir crypto tiger para utilizar encriptacao
// nao esta definido pois estava com bug durante recepcao
//#define     CRYPTO_TIGER

//  DEFINES PROTOCOLO

#define     ACK             0x06
#define     NAK             0x15
    

// DEFINES ST (STATUS) PROTOCOLO

#define     OK                      0x00
#define     BLOQUEADO               0x01
#define     OPERACAO_LOCAL          0x02
#define     ERRO_MOTOR              0x03
#define     ERRO_CELULA_CARGA       0x04
#define     ERRO_SENSOR_DISTANCIA   0x05
#define     ERRO_BOTAO_PRESSIONADO  0x06
#define     ERRO_NETWORK            0x07

//  DEFINES INIT PROTOCOLO

#define     STX             0x02
#define     PLAINTEXT       0x00
#define     CIPHERTEXT      0x01
#define     LEN_SIGN        10

//  DEFINES IDENTIFICADOR (PID) PROTOCOLO

#define     ID_REPORT       0x01
#define     CMD_RESP        0x02
#define     RESPONSE        0x03

//  DEFINES COMANDOS PROTOCOLO

#define     NAO_HA_CMD                  0x00
#define     ALTERAR_HORARIO_COMIDA      0x01
#define     COLOCAR_COMIDA              0x02
#define     BLOQUEIO                    0x03

//  DEFINES STATUS COMANDOS PROTOCOLO

#define     EM_FILA         0x00
#define     REALIZADO       0x01
#define     NAO_REALIZADO   0x02

// ESTRUTURAS
#ifdef IS_ARDUINO // Arduino tem 4 bytes para long, eqto PC tem 4 bytes para int
#pragma pack(push, 1) // nao faz alinhamento memoria 

typedef struct{
    unsigned char iniciador;            // 1 byte
    unsigned char modo;                 // 1 byte
}TPacketInit;

typedef struct{
    char sign[10];                      // 1*10 bytes
    unsigned char id;                   // 1 byte
    unsigned long int seq;              // 4 bytes
    unsigned char st;                   // 1 byte
    char sn[10];                        // 1*10 bytes
}TPacketID;

typedef struct{  
    TPacketInit init;                   // 2 bytes
    TPacketID pid;                      // 30 bytes
    unsigned long int uptime;           // 4 bytes    
    unsigned short nivel_alm;           // 2 bytes
    unsigned short peso;                // 2 bytes
    char horario_alm[23];               // 1*23 byte
    unsigned short qtde_alm;            // 2 bytes
    unsigned short crc;                 // 2 bytes
}TPacketReport;

typedef struct{
    TPacketInit init;                   // 2 bytes
    TPacketID pid;                      // 30 bytes
    unsigned long int idCmd;            // 4 bytes
    long cmd;                           // 4 bytes
    long tamanho;                       // 4 bytes
    char valor[30];                     // 30 bytes 
    unsigned short crc;                 // 2 bytes
}TPacketResponse;

typedef struct{
    TPacketInit init;                   // 2 bytes
    TPacketID pid;                      // 30 bytes
    unsigned long int idCmd;            // 4 bytes
    long cmd;                           // 4 bytes
    long cmdStatus;                     // 4 bytes 
    unsigned short crc;                 // 2 bytes
}TPacketCmdResponse;
#pragma pack(pop) 
#endif

#ifndef IS_ARDUINO
#pragma pack(push, 1) // nao faz alinhamento memoria 

typedef struct{
    unsigned char iniciador;            // 1 byte
    unsigned char modo;                 // 1 byte
}TPacketInit;

typedef struct{
    char sign[10];                      // 1*10 bytes
    unsigned char id;                   // 1 byte
    unsigned int seq;                   // 4 bytes
    unsigned char st;                   // 1 byte
    char sn[10];                        // 1*10 bytes
}TPacketID;

typedef struct{  
    TPacketInit init;                   // 2 bytes
    TPacketID pid;                      // 30 bytes
    unsigned int uptime;                // 4 bytes    
    unsigned short nivel_alm;           // 2 bytes
    unsigned short peso;                // 2 bytes
    char horario_alm[23];               // 1*23 byte
    unsigned short qtde_alm;            // 2 bytes
    unsigned short crc;                 // 2 bytes
}TPacketReport;

typedef struct{
    TPacketInit init;                   // 2 bytes
    TPacketID pid;                      // 30 bytes
    unsigned int idCmd;                 // 4 bytes
    int cmd;                            // 4 bytes
    int tamanho;                        // 4 bytes
    char valor[30];                     // 30 bytes 
    unsigned short crc;                 // 2 bytes
}TPacketResponse;

typedef struct{
    TPacketInit init;                   // 2 bytes
    TPacketID pid;                      // 30 bytes
    unsigned int idCmd;                 // 4 bytes
    int cmd;                            // 4 bytes
    int cmdStatus;                      // 4 bytes 
    unsigned short crc;                 // 2 bytes
}TPacketCmdResponse;
#pragma pack(pop) 
#endif

#endif
