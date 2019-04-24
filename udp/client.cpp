// Client side implementation of UDP client-server model 
#include "calls.h"

// Driver code 
int main() { 
    char Host[LEN_HOST];
    memcpy(Host, "127.0.0.1\0", LEN_HOST);
    int sockfd; 
    char buffer[LEN_BUFFER]; 
    char *hello = "Hello from client"; 
    struct sockaddr_in servaddr;

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
    printf("crc calculado=%d bytes=%02X", pcktReport.crc, pcktReport.crc);
    
    printf("\nsize TPacketReport=%i\n", sizeof(TPacketReport));
    printf("horario_alimentacao=%s\n", pcktReport.horario_alimentacao);

    // serializa struct
    unsigned char buff[sizeof(TPacketReport)];
    memcpy(buff, (unsigned char*)&pcktReport, sizeof(TPacketReport));

    printf("Buffer serialized\n");
    for (int i = 0; i < sizeof(TPacketReport); i++){
        printf("%02X", buff[i]);
    }

    char key[5];
    memcpy(key, "12345", 5);

    unsigned char *ciphertext = (unsigned char *)malloc(sizeof(int) * sizeof(TPacketReport)); 

    RC4(key, (char*)buff, ciphertext);
    printf("\nRC4 Buffer serialized\n");
    for(size_t i = 0, len = sizeof(TPacketReport); i < len; i++)
        printf("%02hhX", ciphertext[i]);
    printf("\n");

    // deserializa struct
    TPacketReport tmp; //Re-make the struct
    memcpy(&tmp, buff, sizeof(TPacketReport));
    printf("\nDeserializado = horario_alimentacao=%s\n", tmp.horario_alimentacao);
         return 0;   
    // Creating socket file descriptor 
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
  
    memset(&servaddr, 0, sizeof(servaddr)); 
      
    // Filling server information 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(PORT); 
    servaddr.sin_addr.s_addr = inet_addr("68.183.138.223"); 
      
    int n=0;
    socklen_t len=0;
      
    sendto(sockfd, (const char *)buff, sizeof(TPacketReport), MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr)); 
    printf("Message sent.\n"); 
          
    n = recvfrom(sockfd, (char *)buffer, LEN_BUFFER, MSG_WAITALL, (struct sockaddr *) &servaddr, &len); 
    buffer[n] = '\0'; 
    printf("Server response: %s\n", buffer); 
  
    close(sockfd); 
    return 0; 
} 