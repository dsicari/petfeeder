//---------------------------------------------------------------------------
void verifica5min()
{
  if ((millis() - tempo_5min) > 300000 && stPetFeeder!=OPERACAO_LOCAL) 
  {
    tempo_5min = millis();   
    if(menuAcessado==true){
      constroiDisplayMain();
      menuAcessado=false;
    }

    //reportar(); 
     
  }
}
