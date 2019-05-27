//---------------------------------------------------------------------------
void gravarResetEEPROM(){
  EEPROM.write(ADDR_RES, resetEEPROM); 
}
//---------------------------------------------------------------------------
void lerResetEEPROM(){
  resetEEPROM=(bool)EEPROM.read(ADDR_RES); 
}
//---------------------------------------------------------------------------
void gravarHorEEPROM(){
  //EEPROM.put(ADDR_HOR, horario_alm); 
  for (int i=0; i<sizeof(horario_alm); i++){
    EEPROM.write(ADDR_HOR+i, horario_alm[i]);
  }
}
//---------------------------------------------------------------------------
void lerHorEEPROM(){
  //EEPROM.get(ADDR_HOR, horario_alm); 
  for (int i=0; i<sizeof(horario_alm); i++){
    horario_alm[i]=(char)EEPROM.read(ADDR_HOR+i);
  }
}
//---------------------------------------------------------------------------
void gravarQtdeAlmEEPROM(){
  //EEPROM.put(ADDR_QTD, qtde_alm); 
  byte two = (qtde_alm & 0xFF);
  byte one = ((qtde_alm >> 8) & 0xFF);
  
  EEPROM.write(ADDR_QTD, two);
  EEPROM.write(ADDR_QTD + 1, one);
}
//---------------------------------------------------------------------------
void lerQtdeAlmEEPROM(){
  //EEPROM.get(ADDR_QTD, qtde_alm);
  byte two = EEPROM.read(ADDR_QTD);
  byte one = EEPROM.read(ADDR_QTD + 1);
 
  qtde_alm = ((two << 0) & 0xFF) + ((one << 8) & 0xFF00); 
}
//---------------------------------------------------------------------------
void gravarSeqEEPROM(unsigned long int seq){
  //EEPROM.put(ADDR_SEQ, seq); 
  byte four = (seq & 0xFF);
  byte three = ((seq >> 8) & 0xFF);
  byte two = ((seq >> 16) & 0xFF);
  byte one = ((seq >> 24) & 0xFF);
  EEPROM.write(ADDR_SEQ, four);
  EEPROM.write(ADDR_SEQ + 1, three);
  EEPROM.write(ADDR_SEQ + 2, two);
  EEPROM.write(ADDR_SEQ + 3, one);
}
//---------------------------------------------------------------------------
unsigned long int lerSeqEEPROM(){
  long four = EEPROM.read(ADDR_SEQ);
  long three = EEPROM.read(ADDR_SEQ + 1);
  long two = EEPROM.read(ADDR_SEQ + 2);
  long one = EEPROM.read(ADDR_SEQ + 3);
  return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
}
