//---------------------------------------------------------------------------
void verifica1s()
{
  if ((millis() - tempo_1s) > 1000 && stPetFeeder!=OPERACAO_LOCAL) 
  {
    tempo_1s = millis();
    uptime++;
    if(menuAcessado==true){
      constroiDisplayMain();
      menuAcessado=false;
    }  
    static bool minBloq=false;
    static int _nivelComida=nivelComida;
    static int _pesoPote=pesoPote;
    if(_nivelComida!=nivelComida || _pesoPote!=pesoPote){
      _nivelComida=nivelComida;
      _pesoPote=pesoPote;
      lcd.setCursor(0, 1);lcd.print("                ");    
      lcd.setCursor(0, 1);lcd.print(nivelComida);lcd.print('%');
      lcd.setCursor(11, 1);lcd.print(pesoPote);lcd.print('g'); 
    }         

    String hora(rtc.getTime().hour);
    String minuto(rtc.getTime().min);
    if(hora.length()==1){
      String tmp(hora);
      hora="0";
      hora.concat(tmp);
    }
    if(minuto.length()==1){
      String tmp(minuto);
      minuto="0";
      minuto.concat(tmp);
    }
    String horarios(horario_alm);
    String horario="";    
    horario.concat(hora);horario.concat(":");horario.concat(minuto);
    Serial.print("[");Serial.print(horario);Serial.print("]");Serial.print("[");Serial.print(horarios);Serial.print("]");Serial.print(F(",st="));Serial.print(stPetFeeder);Serial.print(F(",minBloq="));Serial.println(minBloq);
    int pos=horarios.indexOf(horario);
    if(pos != -1 && minuto != lastMinute && stPetFeeder==OK && minBloq==false ){
      Serial.println(F("Evento encontrado, minBloq=1"));
      lastMinute=minuto;
      minBloq=true;
      colocarComida(qtde_alm);
    }
    if(minuto != lastMinute && minBloq==true){
      Serial.println(F("minBloq=0"));
      minuto="";
      minBloq=false;
    } 

    
  }
}
