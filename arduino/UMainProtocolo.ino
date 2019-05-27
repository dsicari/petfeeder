//---------------------------------------------------------------------------
bool verificaBufferRecebidoClient(unsigned char *bufferRcv, int lenBufferRcvData, bool log)
{
    bool rslt=true;

    Serial.print("len=");Serial.println(lenBufferRcvData);
    if(bufferRcv[0] == STX)
    {
        if(bufferRcv[1] == PLAINTEXT){
            if(log==true) Serial.println(F("Cliente recebeu pacote plaintext"));
        }
        else if(bufferRcv[1] == CIPHERTEXT){
#ifdef CRYPTO_TIGER
            tiger.Decrypt(&bufferRcv[2], lenBufferRcvData-2);
#endif
            if(log==true) Serial.println(F("Cliente recebeu pacote encriptado"));
        }
        else{
            if(log==true) Serial.println(F("Modo nao bateu"));
            rslt=false;
        }

        unsigned short crcRecebido, crcCalculado;
        memcpy(&crcRecebido, &bufferRcv[lenBufferRcvData-2], 2);
        crcCalculado = calccrc16(&bufferRcv[2], lenBufferRcvData-4);
        if(crcRecebido!=crcCalculado)
        {
            if(log==true){
              Serial.println(F("CRC pacote recebido nao bateu"));        
            }
            rslt=false;
        }
        else{
            if(memcmp(&bufferRcv[2], "assinatura", LEN_SIGN) != 0){
                if(log==true) Serial.println(F("Assinatura nao bateu"));
                rslt=false;
            }
        }
        
        if(log==true){
          Serial.print(F("crcRecebido:"));Serial.println(crcRecebido);
          Serial.print(F("crcCalculado:"));Serial.println(crcCalculado);
        }
    }
    else{
        if(log==true) Serial.println(F("Iniciador pacote invalido"));
        rslt=false;
    }
    return rslt;
}
//---------------------------------------------------------------------------
void montaReport(TPacketReport *pkt, bool log=false)
{
    // iniciador
    pkt->init.iniciador=STX;
    pkt->init.modo=PLAINTEXT;
    // pacote id
    memcpy(pkt->pid.sign, sign, 10);
    pkt->pid.seq=++seq;
    gravarSeqEEPROM(seq);
    pkt->pid.st=stPetFeeder;
    pkt->pid.id=ID_REPORT;
    memcpy(pkt->pid.sn, sn, 10);
    // pacote report
    pkt->uptime=uptime;
    pkt->nivel_alm=nivelComida;
    pkt->peso=pesoPote;
    memcpy(pkt->horario_alm, horario_alm, 23);
    pkt->qtde_alm=100; 
    pkt->crc=0;
    
    if(log==false) return;    
    Serial.print(rtc.getDateStr());Serial.print(" -- ");Serial.print(rtc.getTimeStr());Serial.println(F(" ----- Monta pacote=ID_REPORT -----"));    
    Serial.print(F("iniciador="));Serial.println(pkt->init.iniciador);
    Serial.print(F("modo="));Serial.println(pkt->init.modo);
    Serial.print(F("assinatura="));
    for(int i=0; i<sizeof(pkt->pid.sign); i++) Serial.print(pkt->pid.sign[i]);
    Serial.println();
    Serial.print(F("identificador="));Serial.println(pkt->pid.id);
    Serial.print(F("sequencia="));Serial.println((long)pkt->pid.seq);
    Serial.print(F("status="));Serial.println(pkt->pid.st);
    Serial.print(F("sn="));
    for(int i=0; i<sizeof(pkt->pid.sn); i++) Serial.print(pkt->pid.sn[i]);
    Serial.println();
    Serial.print(F("uptime="));Serial.println((long)pkt->uptime);
    Serial.print(F("nivel alimento="));Serial.println(pkt->nivel_alm);
    Serial.print(F("peso="));Serial.println(pkt->peso);
    Serial.print(F("horario_alimentacao="));
    for(int i=0; i<sizeof(pkt->horario_alm); i++) Serial.print(pkt->horario_alm[i]);
    Serial.println();
    Serial.print(F("quantidade alimento="));Serial.println(pkt->qtde_alm);    
}
//---------------------------------------------------------------------------
bool sendRcv(unsigned char *buff, int *lenBuff)
{
  bool rslt=false;
  
  if (wifi.registerUDP(HOST_NAME, HOST_PORT)) Serial.println(F("register udp ok"));
  else Serial.println(F("register udp error"));

  Serial.print(F("Enviando... "));Serial.print(*lenBuff);Serial.print(F(" bytes="));
  for(uint32_t i = 0; i < *lenBuff; i++) {
    Serial.print("{");Serial.print(i);Serial.print(",");Serial.print((unsigned char)buff[i], HEX);Serial.print("}");
  }
  Serial.print("\r\n");
  
  if(wifi.send((const uint8_t*)buff, *lenBuff) == true)
  { 
    Serial.println(F("Pacote enviado..."));
    memset(buff, 0, BUFFER_MAX);
    *lenBuff=0;    
    *lenBuff = wifi.recv((uint8_t*)buff, BUFFER_MAX);
    if (*lenBuff > 0) {
      Serial.print(F("Received bytes="));Serial.println(*lenBuff);      
      Serial.println(F("bytes="));
      for(uint32_t i = 0; i < *lenBuff; i++) {
        Serial.print("{");Serial.print(i);Serial.print(",");Serial.print((unsigned char)buff[i], HEX);Serial.print("}");
      }
      Serial.print("\r\n");      
      rslt=verificaBufferRecebidoClient(buff, *lenBuff, true);   
    }
    else{
      Serial.println(F("Nenhum pacote recebido"));
    }
  } 
  
  if (wifi.unregisterUDP()) Serial.println(F("unregister udp ok"));
  else Serial.println(F("unregister udp error"));  
  
  return rslt;
}
//---------------------------------------------------------------------------
void reportar()
{
  //----------------------------------
  //       MONTA PACOTE REPORT
  //----------------------------------
  
  TPacketReport pkt;
  montaReport(&pkt, true); 
  unsigned short crc=0;
  //unsigned char buff[sizeof(TPacketReport)];
  
  // Buffer para recepcao
  //unsigned char buffRcv[BUFFER_MAX];
  //int lenBuffRcv=0;
  bool r=false;

  memset(bufferUDP, 0, sizeof(bufferUDP));
  memcpy(bufferUDP, (unsigned char*)&pkt, sizeof(TPacketReport));
  crc=calccrc16(&bufferUDP[2], sizeof(TPacketReport) - 4);
  memcpy(&bufferUDP[sizeof(TPacketReport) - 2], &crc, 2);
#ifdef CRYPTO_TIGER
  if(pkt.init.modo == CIPHERTEXT) criptografiaBuffer(bufferUDP, sizeof(bufferUDP), &tiger, pkt.init.modo);
#endif
  //memset(buffRcv, 0, sizeof(buffRcv));  
  //r=sendRcv(buff, sizeof(buff), buffRcv, &lenBuffRcv, 2000);
  int lenBuff=sizeof(TPacketReport);
  r=sendRcv(bufferUDP, &lenBuff);
  Serial.print("SendRcv=");Serial.println(r);
  
  //----------------------------------
  //       PARSE PACOTE
  //----------------------------------
  
  if(r==true)
  {
    int tipoPacote=idPacketRcv(bufferUDP, sizeof(bufferUDP));
    if(tipoPacote == RESPONSE)
    {
      TPacketResponse pktResp;
      memcpy(&pktResp, bufferUDP, sizeof(TPacketResponse));
      Serial.println(F("## RESPONSE recebido, TPacketResponse:"));
      Serial.print(F("iniciador="));Serial.println(pktResp.init.iniciador);
      Serial.print(F("modo="));Serial.println(pktResp.init.modo);
      Serial.print(F("assinatura="));
      for(int i=0; i<sizeof(pktResp.pid.sign); i++) Serial.print(pktResp.pid.sign[i]);
      Serial.println();
      Serial.print(F("identificador="));Serial.println(pktResp.pid.id);
      Serial.print(F("sequencia="));Serial.println((long)pktResp.pid.seq);
      Serial.print(F("status="));Serial.println(pktResp.pid.st);
      Serial.print(F("sn="));
      for(int i=0; i<sizeof(pktResp.pid.sn); i++) Serial.print(pktResp.pid.sn[i]);
      Serial.println(); 
      Serial.print(F("idCmd="));Serial.println(pktResp.idCmd);
      Serial.print(F("cmd="));Serial.println(pktResp.cmd);
      Serial.print(F("tamanho="));Serial.println(pktResp.tamanho);
      Serial.print(F("valor cmd="));
      for(int i=0; i<sizeof(pktResp.valor); i++) Serial.print(pktResp.valor[i]);
      Serial.println();
      Serial.print(F("sizeof(valor)="));Serial.println(sizeof(pktResp.valor));  
      //--------------------------
      //      REALIZA COMANDO
      //--------------------------
      if(pktResp.cmd==NAO_HA_CMD)
      {
        Serial.println(F("Nao ha cmd para executar..."));
      } 
      else if(pktResp.cmd==ALTERAR_HORARIO_COMIDA)
      {
        /*
        int pos=0;
        for(int i=0; i<sizeof(pktResp.valor); i++){
          if(pktResp.valor[i] == 0x3B){
            Serial.print(F("pos (;)="));Serial.println(i);
            pos=i;
          }
        }
        char tmp_valor[pktResp.tamanho];
        memcpy((char)pktResp.valor, tmp_valor, pktResp.tamanho);
        char tmp[4];
        memcpy(&tmp_valor[pos+1], tmp, sizeof(tmp-1));
        tmp[4]='\0';
        int nova_qtde_alm=atoi(tmp);
        Serial.print(F("Nova qtde char="));Serial.println(tmp);
        Serial.print(F("Nova qtde int="));Serial.println(nova_qtde_alm);
        char tmp2[pos];
              
        Serial.println(F("Cmd ALTERAR_HORARIO_COMIDA recebido"));  
        String horarios_atuais(horario_alm);        
        String valores_novos(pktResp.valor);       
        Serial.print(F("Novos Valores="));Serial.println(valores_novos);
        Serial.print(F("Qtde alimento="));Serial.print(qtde_alm);Serial.print(">>");
        qtde_alm=(unsigned short)valores_novos.substring(valores_novos.length()-3, valores_novos.length()).toInt();
        gravarQtdeAlmEEPROM();
        lerQtdeAlmEEPROM();
        Serial.println(qtde_alm);        
        String horarios_novos=valores_novos.substring(0, valores_novos.length()-4);
        Serial.print(F("Horarios atuais["));Serial.print(horarios_atuais);Serial.print("]>>");Serial.print(F("Horarios novos["));Serial.print(horarios_novos);Serial.println("]");                
        memcpy(horario_alm, horarios_novos.c_str(), horarios_novos.length()); 
        gravarHorEEPROM();
        lerHorEEPROM();
        Serial.print(F("Horarios atuais modificados["));Serial.print(horario_alm);Serial.println("]");
        */

        char tmp_valor[sizeof(pktResp.valor)+1];
        memcpy(tmp_valor, (char*)pktResp.valor, sizeof(pktResp.valor));
        tmp_valor[pktResp.tamanho+1]='\0';
        Serial.print(F("Novos Valores="));Serial.println(tmp_valor);
        
        char tmp_qtde_alm[4];
        memcpy(tmp_qtde_alm, &tmp_valor[pktResp.tamanho-4], sizeof(tmp_qtde_alm));
        tmp_qtde_alm[4]='\0';
        int nova_qtde_alm=atoi(tmp_qtde_alm);
        Serial.print(F("Nova qtde char="));Serial.println(tmp_qtde_alm);
        qtde_alm=(unsigned short)nova_qtde_alm;
        gravarQtdeAlmEEPROM();
        lerQtdeAlmEEPROM();
        Serial.print(F("Nova qtde int="));Serial.println(nova_qtde_alm);

        int pos=0;
        for(int i=0; i<sizeof(pktResp.valor); i++){
          if(pktResp.valor[i] == 0x3B){
            Serial.print(F("pos (;)="));Serial.println(i);
            pos=i;
          }
        }
        char tmp_horarios[pos+2];
        memcpy(tmp_horarios, tmp_valor, pos);
        tmp_horarios[pos]='\0';
        Serial.print(F("Novos horarios="));Serial.println(tmp_horarios);
        memcpy(horario_alm, "21:59,22:01,22:05,22:10\0", sizeof(horario_alm));
        
        TPacketCmdResponse pktCmdResponse;                        
        pktCmdResponse.init.iniciador=STX;
        pktCmdResponse.init.modo=pktResp.init.modo;
        memcpy(pktCmdResponse.pid.sign, sign, sizeof(sign));
        pktCmdResponse.pid.id=CMD_RESP;
        pktCmdResponse.pid.seq=pktResp.pid.seq;
        pktCmdResponse.pid.st=stPetFeeder;                        
        memcpy(pktCmdResponse.pid.sn, pktResp.pid.sn, sizeof(pktResp.pid.sn));   
        pktCmdResponse.idCmd=pktResp.idCmd;
        pktCmdResponse.cmd=pktResp.cmd;
        pktCmdResponse.cmdStatus=REALIZADO; 
        memset(bufferUDP, 0, sizeof(bufferUDP));       
        memcpy(bufferUDP, (unsigned char*)&pktCmdResponse, sizeof(TPacketCmdResponse));
        crc=calccrc16(&bufferUDP[2], sizeof(TPacketCmdResponse) - 4);
        memcpy(&bufferUDP[sizeof(TPacketCmdResponse) - 2], &crc, 2);                    
        //r=sendRcv(bufferUDP, sizeof(TPacketCmdResponse));
        Serial.print(F("SendRcv="));Serial.println(r);
      }
      else if(pktResp.cmd==COLOCAR_COMIDA)
      {
        //...
      }else Serial.println(F("Cmd desconhecido"));                  
    }else Serial.println(F("Tipo pacote desconhecido"));  
  }else Serial.println(F("Nenhum pacote recebido!"));
}  
