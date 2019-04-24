/*
    robin verton, dec 2015
    implementation of the RC4 algo
*/

#include "rc4.h"

void swap(unsigned char *a, unsigned char *b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

int KSA(char *key, unsigned char *S) {

    int len = strlen(key);
    int j = 0;

    for(int i = 0; i < N; i++)
        S[i] = i;

    for(int i = 0; i < N; i++) {
        j = (j + S[i] + key[i % len]) % N;

        swap(&S[i], &S[j]);
    }

    return 0;
}

int PRGA(unsigned char *S, char *plaintext, unsigned char *ciphertext) {

    int i = 0;
    int j = 0;

    for(size_t n = 0, len = strlen(plaintext); n < len; n++) {
        i = (i + 1) % N;
        j = (j + S[i]) % N;

        swap(&S[i], &S[j]);
        int rnd = S[(S[i] + S[j]) % N];

        ciphertext[n] = rnd ^ plaintext[n];

    }

    return 0;
}

int RC4(char *key, char *plaintext, unsigned char *ciphertext) {

    unsigned char S[N];
    KSA(key, S);

    PRGA(S, plaintext, ciphertext);

    return 0;
}
/*
int main() {

    char key[5];
    memcpy(key, "12345", 5);

    char plaintext[4];
    memcpy(plaintext, "1", 4);

    unsigned char *ciphertext = (unsigned char *)malloc(sizeof(int) * strlen(plaintext));    

    RC4(key, plaintext, ciphertext);
    for(size_t i = 0, len = strlen(plaintext); i < len; i++)
        printf("%02hhX", ciphertext[i]);
    printf("\n");

     RC4(key, (char*)ciphertext, (unsigned char*)plaintext);
    for(size_t i = 0, len = strlen(plaintext); i < len; i++)
        printf("%02hhX", plaintext[i]);
    printf("\n");
    
    return 0;
}*/