#include "UCrytiger.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//------------------------------------------------------------------------------
TCrytiger::TCrytiger() 
{
}
//------------------------------------------------------------------------------
TCrytiger::~TCrytiger() 
{
}
//------------------------------------------------------------------------------
void TCrytiger::InitTiger(unsigned char *key, int lenKey){
    int index=0;
    memset(XorKey, 0, LEN_XORKEY);

    while(index < LEN_XORKEY){
        if(LEN_XORKEY-index > lenKey){
            memcpy(&XorKey[index], key, lenKey);
            index+=lenKey;  
        }
        else{
            memcpy(&XorKey[index], key, LEN_XORKEY-index);  
            break;  
        }
    } 
}
//------------------------------------------------------------------------------
void TCrytiger::EncryptDecrypt(unsigned char *inpString, int len) 
{   
    int index=0;
    for (int i = 0; i < len; i++) 
    { 
        if(index >= LEN_XORKEY) index=0;
        inpString[i] = inpString[i] ^ XorKey[index]; 
        //printf("%02X",inpString[i]);
        index++; 
    } 
} 
//------------------------------------------------------------------------------
void TCrytiger::Encrypt(unsigned char *inpString, int len)
{
    EncryptDecrypt(inpString, len);
}
//------------------------------------------------------------------------------
void TCrytiger::Decrypt(unsigned char *inpString, int len)
{
    EncryptDecrypt(inpString, len); 
}
