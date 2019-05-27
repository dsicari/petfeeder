//---------------------------------------------------------------------------
void verifica1min()
{
  if ((millis() - tempo_1min) > 60000 && stPetFeeder!=OPERACAO_LOCAL) 
  {
    tempo_1min = millis();
    if(menuAcessado==true){
      constroiDisplayMain();
      menuAcessado=false;
    }

    constroiDisplayMain();

    reportar();
    
  } // fim  millis
}// fim verifica1min
