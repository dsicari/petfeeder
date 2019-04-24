#include <stdio.h>

int main(){
    unsigned char buffer[1024];
    // warning: format ‘%i’ expects argument of type ‘int’, but argument 2 has type ‘long unsigned int’ [-Wformat=]
    // trocou-se %i para %zu
    printf("\nA Char is %zu bytes", sizeof(char ));
    printf("\nAn int is %zu bytes", sizeof(int ));
    printf("\nA short is %zu bytes", sizeof(short ));
    printf("\nA long is %zu bytes", sizeof(long ));
    printf("\nA long long is %zu bytes\n", sizeof(long long));
    printf("\nAn unsigned Char is %zu bytes", sizeof(unsigned char));
    printf("\nAn unsigned int is %zu bytes", sizeof(unsigned int));
    printf("\nAn unsigne short is %zu bytes", sizeof(unsigned short));
    printf("\nAn unsigned long is %zu bytes", sizeof(unsigned long));
    printf("\nAn unsigned long long is %zu bytes\n", sizeof(unsigned long long));
    printf("\nfloat is %zu bytes", sizeof(float));
    printf("\nA double is %zu bytes\n", sizeof(double));
    printf("\nA long double is %zu bytes\n", sizeof(long double));
    printf("\nbuffer is %zu bytes\n", sizeof(buffer));
    return 0;
}