#include "UPetFeeder.h"
#include "UCrytiger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//---------------------------------------------------------------------------
static unsigned short _incrCrcCalc( unsigned char ch, unsigned short currentCrc )
{
  unsigned short newCrc;
  unsigned short ch_loc;
  static const unsigned short int crctbl[16]  = {
            0x0000,0xcc01,0xd801,0x1400,0xf001,0x3c00,0x2800,0xe401,
            0xa001,0x6c00,0x7800,0xb401,0x5000,0x9c01,0x8801,0x4400 };
  newCrc=currentCrc;
  ch_loc=(unsigned short)ch;
    newCrc = ( newCrc >> 4 ) ^ crctbl[ ( ch_loc ^ newCrc ) & 0x0f ];
    newCrc = ( newCrc >> 4 ) ^ crctbl[ ( ( ch_loc >> 4 ) ^ newCrc ) & 0x0f ];
   return newCrc;
}
//---------------------------------------------------------------------------
unsigned short calccrc16(const void* void_data, unsigned data_size)
{
   unsigned i;
   unsigned char* data;
   unsigned short crc;
   crc=0xAA55;
   crc+=(unsigned short)data_size;
   data=(unsigned char*)void_data;
   for ( i=0 ; i < data_size ; i++ )  {
      crc=_incrCrcCalc(data[i],crc);
   }
   return crc;
}
//---------------------------------------------------------------------------
/*char * ucharByteArray2charHexArray(unsigned char *buffer, int len){
    char *temp=new char[len*2+1];
    for(int i=0; i < len; i++){
        sprintf(&temp[i*2], "%02X", buffer[i]);
    }
    memset(&temp[len*2], '\0', 1);
    char *returnTemp=temp;
    temp=NULL;
    delete temp;
    return returnTemp;
}*/
//---------------------------------------------------------------------------
/*char *appendChar2CharArray(char* array, int len, char c)
{
    char *ret = new char[len+1];
    memcpy(ret, array, len);
    ret[len+1]=c;
    return ret;
}*/
//------------------------------------------------------------------------------
void criptografiaBuffer(unsigned char *buff, int len, TCrytiger *tiger, unsigned char modo)
{
    if(modo == CIPHERTEXT){
        // ciphertext = pkt.pid + pkt + pkt.crc - pkt.init
        tiger->Encrypt(&buff[2], len - 2);
    }   
}
//------------------------------------------------------------------------------
int idPacketRcv(unsigned char *buffer, int len){
    int rslt = 0;
    if(buffer[12] == ID_REPORT) rslt=ID_REPORT;
    else if(buffer[12] == CMD_RESP) rslt=CMD_RESP;
    else if(buffer[12] == RESPONSE) rslt=RESPONSE;
    else rslt = -1; // Identificador nao reconhecido 
    return rslt;
}
//---------------------------------------------------------------------------
bool checkCRC(unsigned char *buff, int len, unsigned short *crcRecebido, unsigned short *crcCalculado)
{
    bool rslt=false;
    unsigned short _crcRecebido=0;
    memcpy(&crcRecebido, &buff[len-2], 2);
    unsigned short _crcCalculado = calccrc16(&buff[2], len-4);
    *crcRecebido=_crcRecebido;
    *crcCalculado=_crcCalculado;
    if(crcRecebido == crcCalculado){
        rslt=true;
    }
    return rslt;
}
