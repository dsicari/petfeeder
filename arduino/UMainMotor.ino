//---------------------------------------------------------------------------
void pararMotor()
{
    digitalWrite(IN1, 0);
    digitalWrite(IN2, 0);
    digitalWrite(IN3, 0);
    digitalWrite(IN4, 0);
}
//---------------------------------------------------------------------------
void motorHorario(int passos, int tempo)
{
  for(int i=0; i<passos; i++){
    // Passo 1
    digitalWrite(IN1, 1);
    digitalWrite(IN2, 0);
    digitalWrite(IN3, 0);
    digitalWrite(IN4, 1);
    delay(tempo);  
  
    // Passo 2
    digitalWrite(IN1, 0);
    digitalWrite(IN2, 1);
    digitalWrite(IN3, 0);
    digitalWrite(IN4, 1);
    delay(tempo);
  
    // Passo 3
    digitalWrite(IN1, 0);
    digitalWrite(IN2, 1);
    digitalWrite(IN3, 1);
    digitalWrite(IN4, 0);
    delay(tempo);
  
    // Passo 4
    digitalWrite(IN1, 1);
    digitalWrite(IN2, 0);
    digitalWrite(IN3, 1);
    digitalWrite(IN4, 0);
    delay(tempo);
  }
}
//---------------------------------------------------------------------------
void motorAntiHorario(int passos, int tempo)
{
  for(int i=0; i<passos; i++){
    // Passo 1
    digitalWrite(IN1, 1);
    digitalWrite(IN2, 0);
    digitalWrite(IN3, 1);
    digitalWrite(IN4, 0);
    delay(tempo);
    // Passo 3
    digitalWrite(IN1, 0);
    digitalWrite(IN2, 1);
    digitalWrite(IN3, 1);
    digitalWrite(IN4, 0);
    delay(tempo);
    // Passo 2
    digitalWrite(IN1, 0);
    digitalWrite(IN2, 1);
    digitalWrite(IN3, 0);
    digitalWrite(IN4, 1);
    delay(tempo);
    // Passo 4
    digitalWrite(IN1, 1);
    digitalWrite(IN2, 0);
    digitalWrite(IN3, 0);
    digitalWrite(IN4, 1);
    delay(tempo);  
  }
}
//---------------------------------------------------------------------------
