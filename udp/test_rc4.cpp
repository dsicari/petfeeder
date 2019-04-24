
#include "calls.h"

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

    char key[5];
    memcpy(key, "12345", 5);

    //char plaintext[4];
    //memcpy(plaintext, "123\0", 4);

    printf("Tamamnho estrutura TPacketReport(%zu) - CRC(%zu) = %zu\n", sizeof(TPacketReport), sizeof(pcktReport.crc), (sizeof(TPacketReport)-sizeof(pcktReport.crc)));
    unsigned char buffer[sizeof(TPacketReport)];
    memcpy(buffer, (unsigned char*)&pcktReport, sizeof(TPacketReport));    
    unsigned char *ciphertext = (unsigned char *)malloc(sizeof(int) * sizeof(TPacketReport)*2);    

    printf("RAW Buffer serialized\n");
    for (int i = 0; i < sizeof(TPacketReport); i++){
        printf("%02X", buffer[i]);
    }

    printf("\nSTRING Buffer serialized\n");
    char plaintext[sizeof(TPacketReport)*2+1];
    for (int i = 0; i < (sizeof(TPacketReport)); i++){
        sprintf(&plaintext[i*2], "%02X", buffer[i]);
    }
    memset(&plaintext[sizeof(TPacketReport)*2], '\0', 1);

    for (int i = 0; i < sizeof(TPacketReport)*2; i++){
        printf("%02X", plaintext[i]);
    }
    printf("\n[2]STRING Buffer serialized\n%s\n", plaintext);

    
    printf("[HEX] RC4 encrypted Buffer serialized POINTER\n");
    RC4(key, plaintext, ciphertext);
    for(size_t i = 0; i < sizeof(TPacketReport)*2; i++)
        printf("%02X", ciphertext[i]);
    
    printf("\n[HEX]RC4 encrypted Buffer serialized\n");
    RC4(key, plaintext, ciphertext);
    for(size_t i = 0; i < sizeof(TPacketReport)*2; i++)
        printf("%02X", ciphertext[i]);
    
    printf("\n[STRING]RC4 encrypted Buffer serialized\n%s\n", ciphertext);

    printf("\n[HEX]RC4 decripted Buffer serialized\n");
    RC4(key, (char*)ciphertext, (unsigned char*)plaintext);
    for(size_t i = 0; i < sizeof(TPacketReport)*2; i++)
        printf("%02hhX", plaintext[i]);
    
    printf("\n[STRING]RC4 decripted Buffer serialized\n%s\n", plaintext);
    
    printf("\n%02hhX", plaintext[0]);
    printf("\n");
    
    return 0;
}