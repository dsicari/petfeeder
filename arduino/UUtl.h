#ifndef UTL_H
#define UTL_H
//------------------------------------------------------------------------------
/**
 * bitRead
 * @description Le bit especifico
 * @param value Valor/Variavel a ser lido
 * @param bit Bit a ser lido, iniciando em 0, LSB (rightmost)
 * @return 0 ou 1
 */
#define bitRead(value, bit) (((value) >> (bit)) & 0x01)

//------------------------------------------------------------------------------
/**
 * bitSet
 * @description Escreve 1 em um bit espefico
 * @param value Valor/Variavel a ser modificado
 * @param bit Bit a ser modificado, iniciando em 0, LSB (rightmost)
 * @return Nenhum
 */
#define bitSet(value, bit) ((value) |= (1UL << (bit)))

//------------------------------------------------------------------------------
/**
 * bitClear
 * @description Escreve 0 em um bit especifico
 * @param value Valor/Variavel a ser modificado
 * @param bit Bit a ser modificado, iniciando em 0, LSB (rightmost)
 * @return Nenhum
 */
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))

//------------------------------------------------------------------------------
/**
 * bitWrite
 * @description Escreve em um bit especifico
 * @param value Valor/Variavel a ser modificado
 * @param bit Bit a ser modificado, iniciando em 0, LSB (rightmost)
 * @param bitvalue Valor do bit a ser modificado 0 ou 1
 * @return Nenhum
 */
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))

//------------------------------------------------------------------------------
/**
 * lowByte
 * @description Extrai o byte menos significativo (rightmost) de uma word
 * @return byte
 */
#define lowByte(w) ((uint8_t) ((w) & 0xff))

//------------------------------------------------------------------------------
/**
 * highByte
 * @description Extrai o byte mais significativo (leftmost) de uma word ou o segundo menos significativo de um tipo longo
 * @return byte
 */
#define highByte(w) ((uint8_t) ((w) >> 8))

unsigned short calccrc16(const void* void_data, unsigned data_size);

char * ucharByteArray2charHexArray(unsigned char *buffer, int len);

char * appendChar2CharArray(char* array, int len, char c);

#ifdef CRYPTO_TIGER
  void criptografiaBuffer(unsigned char *buff, int len, TCrytiger *tiger, unsigned char modo);
#endif

int idPacketRcv(unsigned char *buffer, int len);

bool checkCRC(unsigned char *buff, int len, unsigned short *crcRecebido, unsigned short *crcCalculado);

//bool verificaBufferRecebidoClient(unsigned char *bufferRcv, int lenBufferRcvData, TCrytiger *tiger, bool log);

#endif
