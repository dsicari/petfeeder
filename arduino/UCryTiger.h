#ifndef CRYTIGER_H
#define CRYTIGER_H

#define LEN_XORKEY 32

class TCrytiger {
private:
    bool XorKeyIsSet;
    unsigned char XorKey[LEN_XORKEY];
    
    void EncryptDecrypt(unsigned char *inpString, int len);
    
public:
    TCrytiger();
    ~TCrytiger();
    void InitTiger(unsigned char *key, int lenKey);
    void Encrypt(unsigned char *inpString, int len);
    void Decrypt(unsigned char *inpString, int len);
};

#endif /* CRYTIGER_H */
