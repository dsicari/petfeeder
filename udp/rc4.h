#ifndef RC4_H
#define RC4_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <cstring>

#define N 256   // 2^8

void swap(unsigned char *a, unsigned char *b);

int KSA(char *key, unsigned char *S);

int PRGA(unsigned char *S, char *plaintext, unsigned char *ciphertext);

int RC4(char *key, char *plaintext, unsigned char *ciphertext);



#endif