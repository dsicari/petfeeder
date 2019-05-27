void verifica10ms() 
{
  if ((millis() - tempo_10ms) > 10 ) 
  {
    tempo_10ms = millis();    
     
    bool btn1=digitalRead(BTN_1);
    bool btn2=digitalRead(BTN_2);
    bool btn3=digitalRead(BTN_3);
    static bool btn1_lido=false;
    static bool btn2_lido=false;
    static bool btn3_lido=false;

    static int countZerarTara=0;
    static int countGirarHorario=0;
    static int countGirarAntiHorario=0;
    
    static float distanciaAVG=0;
    static float pesoAVG=0;
    static int count=0;    

    // BOTAO
    if(btn1_lido!=btn1){
      btn1_lido=btn1;
      Serial.print(F("BTN_1=")); Serial.println(btn1, HEX);
    }
    
    if(btn2_lido!=btn2){
      btn2_lido=btn2;
      Serial.print(F("BTN_2=")); Serial.println(btn2, HEX);
    }
    
    if(btn3_lido!=btn3){
      btn3_lido=btn3;     
      Serial.print(F("BTN_3=")); Serial.println(btn3, HEX);       
    }    

    // SENSOR DISTANCIA E CELULA CARGA
    // scale.get_units() atrasa menuAcessado!!
    if(count<10 && stPetFeeder==OK){
      distanciaAVG = distanciaAVG + distanceSensor.measureDistanceCm();
      pesoAVG = pesoAVG + (scale.get_units()*1000);
      count++;
    }
    else{      
      int nivel=(int)(distanciaAVG/10);
      if(nivel<NIVEL_COMIDA_MAX) nivel=NIVEL_COMIDA_MAX;
      else if(nivel>NIVEL_COMIDA_MIN) nivel=NIVEL_COMIDA_MIN;
      nivelComida=map(nivel, NIVEL_COMIDA_MIN, NIVEL_COMIDA_MAX, 0, 100);
      pesoPote=(int)(pesoAVG/10);      
      count=0;
      distanciaAVG=0;
      pesoAVG=0;
    }

    if(btn1_lido==false && btn2_lido==false && btn3_lido==false){
      countGirarHorario=0;
      countGirarAntiHorario=0;
      countZerarTara=0;       
      pararMotor();
    }
    else if(btn1_lido==true && btn2_lido==true && btn3_lido==false){
      // MENU ZERAR TARA
      stPetFeeder=OPERACAO_LOCAL;
      menuAcessado=true;
      if(countZerarTara==0){
        lcd.clear();
        lcd.setCursor(0, 0);lcd.print(F("ZERAR TARA?"));    
        lcd.setCursor(0, 1);lcd.print(F("ZERANDO EM 3s"));
        Serial.println(F("Operacao local, zerar tara, msg=ZERANDO EM 3s"));
      }
      else if(countZerarTara==100){
        lcd.setCursor(0, 1);lcd.print(F("ZERANDO EM 2s"));
        Serial.println(F("Operacao local, zerar tara, msg=ZERANDO EM 2s"));
      }
      else if(countZerarTara==200){
        lcd.setCursor(0, 1);lcd.print(F("ZERANDO EM 1s"));
        Serial.println(F("Operacao local, zerar tara, msg=ZERANDO EM 1s"));
      }
      else if(countZerarTara==300){
        lcd.clear();
        lcd.setCursor(0, 1);lcd.print(F("ZERANDO TARA"));
        Serial.println(F("Operacao local, zerar tara, msg=ZERANDO TARA"));
        scale.tare();
        scale.set_scale(calibration_factor);
        //countZerarTara=0;  // zerar quando menuZerarTara = false 
        //operacaoLocal=false;
        lcd.clear();
        lcd.setCursor(0, 0);lcd.print(F("TARA ZERADA"));
        Serial.println(F("Operacao local, zerar tara, msg=TARA ZERADA"));
      }
      else if(countZerarTara==500){
        lcd.setCursor(0, 1);lcd.print(F("SOLTAR BOTAO"));
        Serial.println(F("Operacao local, zerar tara, msg=SOLTAR BOTAO"));
        stPetFeeder=OK;
      }
      countZerarTara++;      
    }
    else if(btn1_lido==true && btn2_lido==false && btn3_lido==false){
      // MENU GIRAR HORARIO
      stPetFeeder=OPERACAO_LOCAL;
      menuAcessado=true;
      if(countGirarHorario==0){
        lcd.clear();
        lcd.setCursor(0, 0);lcd.print(F("GIRANDO MOTOR H"));
      }
      motorHorario(3, 5); 
      countGirarHorario++;
      stPetFeeder=OK;
    }
    else if(btn1_lido==false && btn2_lido==true && btn3_lido==false){
      // MENU GIRAR ANTIHORARIO
      stPetFeeder=OPERACAO_LOCAL;
      menuAcessado=true;
      if(countGirarAntiHorario==0){
        lcd.clear();
        lcd.setCursor(0, 0);lcd.print(F("GIRANDO MOTOR AH"));
      }
      motorAntiHorario(3, 5); 
      countGirarAntiHorario++;
      stPetFeeder=OK;
    }  

  }
}
