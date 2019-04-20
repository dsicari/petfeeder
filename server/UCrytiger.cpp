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

/*
int main() 
{
	unsigned char sampleString[] = {'1', '2', '3'}; 

	unsigned char key[] = { 0xF1, 0xF2, 0xF3, 0xFF};

	init(key, sizeof(key));
	printf("Chave expandida: ");
	for (int i = 0; i < LEN_XORKEY; i++){ 
		printf("%02X", xorKey[i]); 
	} 
	printf("\n"); 

	printf("Freetext: (%ld bytes): ", sizeof(sampleString));
	for (int i = 0; i < sizeof(sampleString); i++){ 
		printf("{'%c':0x%02X}", sampleString[i],sampleString[i]); 
	} 
	printf("\n");

	// Encrypt
	printf("Encrypted String (%ld bytes): ", sizeof(sampleString)); 
	encrypt(sampleString, sizeof(sampleString)); 
	printf("\n"); 

	// Decrypt
	printf("Decrypted String: (%ld bytes): ", sizeof(sampleString)); 
	encryptDecrypt(sampleString, sizeof(sampleString));
	memset(&sampleString[sizeof(sampleString)], '\0', 1);
	printf(":%s", sampleString); 
	printf("\n"); 

	return 0; 
} */

