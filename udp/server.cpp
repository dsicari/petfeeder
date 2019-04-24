/*#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>*/

#include "calls.h"

/*void error(char *msg) {
  perror(msg);
  exit(1);
}*/

int main(int argc, char **argv) {
  //int sockfd; /* socket */
  //int portno; /* port to listen on */
  //socklen_t clientlen; /* byte size of client's address */
  //struct sockaddr_in serveraddr; /* server's addr */
  //struct sockaddr_in clientaddr; /* client addr */
  //struct hostent *hostp; /* client host info */
  //unsigned char buf[1024]; /* message buf */
  //char *hostaddrp; /* dotted decimal host addr string */
  //int optval; /* flag value for setsockopt */
  //int n; /* message byte size */

  //if (argc != 2) {
  //  fprintf(stderr, "usage: %s <port>\n", argv[0]);
  //  exit(1);
  //}
  //portno = atoi(argv[1]);

  //sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  //if (sockfd < 0) 
  //  error("ERROR opening socket");

  /* setsockopt: Handy debugging trick that lets 
   * us rerun the server immediately after we kill it; 
   * otherwise we have to wait about 20 secs. 
   * Eliminates "ERROR on binding: Address already in use" error. 
   */
  //optval = 1;
  //setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));

  /*
   * build the server's Internet address
   */
  //bzero((char *) &serveraddr, sizeof(serveraddr));
  //serveraddr.sin_family = AF_INET;
  //serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
  //serveraddr.sin_port = htons((unsigned short)portno);

  /* 
   * bind: associate the parent socket with a port 
   */
  //if (bind(sockfd, (struct sockaddr *) &serveraddr, 
  //     sizeof(serveraddr)) < 0) 
  //  error("ERROR on binding");

  /* 
   * main loop: wait for a datagram, then echo it
   */
  //clientlen = sizeof(clientaddr);

  TNetworkServerUDP serverUDP = TNetworkServerUDP("127.0.0.1", 8080);
  if(serverUDP.CreateSocket() == false) return 0;
  if(serverUDP.CreateServer() == false) return 0;
  printf("listening...\n");
  while (1) {
    
    //bzero(buf, 1024);
    //n = recvfrom(sockfd, buf, 1024, 0, (struct sockaddr *) &clientaddr, &clientlen);
    //if (n < 0)
    //  error("ERROR in recvfrom");

    /* 
     * gethostbyaddr: determine who sent the datagram
     */
    //hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr, sizeof(clientaddr.sin_addr.s_addr), AF_INET);
    
    //if (hostp == NULL){
    //    error("ERROR on gethostbyaddr");
    //}
    
    //hostaddrp = inet_ntoa(clientaddr.sin_addr);
    
    //if (hostaddrp == NULL){
    //    error("ERROR on inet_ntoa\n");
    //}
    unsigned char bufferRcv[sizeof(TPacketReport)];
    int lenBufferRcvData=0;
    bool r=serverUDP.Rcv(bufferRcv, sizeof(bufferRcv), &lenBufferRcvData);
    if(r == true){
      printf("server received datagram from %s (%s)\n", serverUDP.GetHostName(), serverUDP.GetHostAddress());    
      printf("server received %d bytes\n", lenBufferRcvData);

      printf("Buffer serialized\n");
      for (int i = 0; i < sizeof(bufferRcv); i++){
          printf("%02X", bufferRcv[i]);
      }
      // deserializa struct
      TPacketReport tmp; //Re-make the struct
      memcpy(&tmp, bufferRcv, sizeof(TPacketReport));
      printf("\n...");
      unsigned short c = calccrc16((unsigned char*)&tmp, (sizeof(TPacketReport) - 2));
      printf("crc calculado=%d bytes=0x%02X", c, c);
      
      
      printf("\ncrc=%i", tmp.crc);
      printf("\nEstrutura TAlimentador deserializado\nserial=%s", tmp.serial);
      printf("\nnivel alimento=%i", tmp.nivel_alimento);
      printf("\npeso=%i", tmp.peso);
      printf("\nhorario_alimentacao=%s", tmp.horario_alimentacao);
      printf("\nquantidade alimento=%i", tmp.quantidade_alimento);
      printf("\ncrc=%i", tmp.crc); 
    } 
    else{
      printf("fails rcv\n");
    }

    /* 
     * sendto: echo the input back to the client 
     */
    //n = sendto(sockfd, (const unsigned char*)buf, sizeof(buf), 0, (struct sockaddr *) &clientaddr, clientlen);
    //if (n < 0)  
    //  error("ERROR in sendto");
  }
}