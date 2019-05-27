#include "UPetFeeder.h"
#ifdef CRYPTO_TIGER
  #include "UCrytiger.h"
#endif
#include "UUtl.h"
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <DS1307.h>
#include <HCSR04.h>
#include "HX711.h"
#include "SoftwareSerial.h"
#include "ESP8266.h"
#include <EEPROM.h>
#include <string.h>
//---------------------------------------------------------------------------
// ESP
#define SSID        "dsicari"
#define PASSWORD    "rootdir99!"
#define HOST_NAME   "68.183.138.223"
#define HOST_PORT   (50685)
#define BUFFER_MAX  80

// Celula de Carga
#define DOUT  A3
#define CLK  A2

// Botoes
#define BTN_3 8
#define BTN_2 9
#define BTN_1 10

// Motor
#define IN1 4 
#define IN2 5
#define IN3 6
#define IN4 7

// Enderecos
#define ADDR_RES    0
#define ADDR_SEQ    10
#define ADDR_HOR    20
#define ADDR_QTD    50

#define NIVEL_COMIDA_MIN 57 
#define NIVEL_COMIDA_MAX 10
//---------------------------------------------------------------------------
DS1307 rtc(A0, A1);
Time  t;
LiquidCrystal_I2C lcd(0x27, 16, 2);
UltraSonicDistanceSensor distanceSensor(11,12);
HX711 scale(DOUT, CLK);
#ifdef CRYPTO_TIGER
  TCrytiger tiger;
#endif
SoftwareSerial espSerial(2,3); // RX, TX
ESP8266 wifi(espSerial);
//---------------------------------------------------------------------------
bool resetEEPROM;
char horario_alm[24];       // armazenado na EEPROM
unsigned short qtde_alm;    // armazenado na EEPROM
char sn[10];
const unsigned char sign[10];
bool menuAcessado=false;
unsigned long int tempo_10ms;
unsigned long int tempo_1s;
unsigned long int tempo_1min;
unsigned long int tempo_5min;
unsigned long int uptime; 
unsigned long int seq;    // armazenado na EEPROM
int nivelComida;
int pesoPote;
float calibration_factor = -383650;
String lastMinute="";
unsigned char bufferUDP[BUFFER_MAX];
unsigned char stPetFeeder;
//---------------------------------------------------------------------------
void verifica10ms();
void verifica1s();
void verifica1min();
void verifica5min();
void constroiDisplayMain();
void colocarComida(int qtde);
void gravarResetEEPROM();
void lerResetEEPROM();
void gravarHorEEPROM();
void lerHorEEPROM();
void gravarQtdeAlmEEPROM();
void lerQtdeAlmEEPROM();
void gravarSeqEEPROM(unsigned long int seq);
unsigned long int lerSeqEEPROM();
void pararMotor();
void motorHorario(int passos, int tempo);
void motorAntiHorario(int passos, int tempo);
void montaReport(TPacketReport *pkt, bool log);
bool verificaBufferRecebidoClient(unsigned char *bufferRcv, int lenBufferRcvData, bool log);
bool sendRcv(unsigned char *buff, int *lenBuff);
void reportar();
//---------------------------------------------------------------------------
void setup()
{
  // Status PetFeeder
  stPetFeeder=OK;
  
  // Serial hardware
  Serial.begin(9600);
  Serial.println(F("Petfeeder iniciando..."));

  // LCD 
  lcd.begin();
  lcd.backlight();
  lcd.clear(); 
  lcd.setCursor(0, 0);
  lcd.print(F(" SEU  PETFEEDER "));
  lcd.setCursor(0, 1);
  lcd.print(F(" ESTA INICIANDO "));
  
  // Inicia serial number
  memset(sn, 0, sizeof(sn));
  memcpy(sn, "1234567890", sizeof(sn));
  
  // Puxa informacoes eeprom  
  //resetEEPROM=true;       // apenas feito 1 vez para iniciar default alimentador
  //gravarResetEEPROM();  
  if(resetEEPROM==true){
    lerResetEEPROM();
    Serial.println(F("Reset EEPROM = true"));
    memcpy(horario_alm, "21:59,22:01,22:05,22:10\0", sizeof(horario_alm));  
    //memcpy(horario_alm, "sssssssssssssssssssssss\0", sizeof(horario_alm)); 
    qtde_alm=100;
    gravarSeqEEPROM(0);
    gravarHorEEPROM();
    gravarQtdeAlmEEPROM();
    resetEEPROM=false;
    gravarResetEEPROM();
  }
  lerHorEEPROM();
  lerQtdeAlmEEPROM();
  seq = lerSeqEEPROM();    
  Serial.print(F("Sequencia pacotes="));Serial.println((long)seq);
  Serial.print(F("Horarios="));
  for(int i=0; i<sizeof(horario_alm); i++) Serial.print(horario_alm[i]);
  Serial.println();
  Serial.print(F("Qtde Alimento="));Serial.println(qtde_alm);
    
  // Tipos
  Serial.print(F("TPacketInit size="));Serial.println(sizeof(TPacketInit));
  Serial.print(F("TPacketID size="));Serial.println(sizeof(TPacketID));
  Serial.print(F("TPacketReport size="));Serial.println(sizeof(TPacketReport));
  Serial.print(F("TPacketResponse size="));Serial.println(sizeof(TPacketResponse));
  Serial.print(F("TPacketCmdResponse size="));Serial.println(sizeof(TPacketCmdResponse));
  
  // Inicia ESP  
  bool rst=wifi.restart();
  Serial.print(F("Reiniciando esp="));Serial.println(rst);
  rst=wifi.leaveAP();
  Serial.print(F("Desconectando wifi esp="));Serial.println(rst);
  if (wifi.setOprToStationSoftAP()) Serial.println(F("to station + softap ok"));
  else Serial.println(F("to station + softap error"));

  if (wifi.joinAP(SSID, PASSWORD)) {
      Serial.println(F("Join AP success"));
      Serial.print(F("IP: "));
      Serial.println(wifi.getLocalIP().c_str());       
  } 
  else Serial.println(F("Join AP failure"));
  
  if (wifi.disableMUX()) Serial.println(F("single ok"));
  else Serial.println(F("single error"));

  // Botoes
  pinMode(BTN_1, INPUT);
  pinMode(BTN_2, INPUT);
  pinMode(BTN_3, INPUT);

  // Motor
  pinMode(IN1,OUTPUT);
  pinMode(IN2,OUTPUT);
  pinMode(IN3,OUTPUT);
  pinMode(IN4,OUTPUT);
  pararMotor();

  // RTC
  //rtc.setDOW(TUESDAY);      //Define o dia da semana
  //rtc.setTime(20, 33, 0);     //Define o horario
  //rtc.setDate(7, 5, 2019);   //Define o dia, mes e ano
  rtc.halt(false);
  rtc.setSQWRate(SQW_RATE_1);
  rtc.enableSQW(true);  

  // Celula carga
  scale.set_scale();
  //scale.tare(); // Zera tara
  //long zero_factor = scale.read_average();
  scale.set_scale(calibration_factor);
  pesoPote=0;
  
  nivelComida=0;
  uptime=0;
  
#ifdef CRYPTO_TIGER
  // Criptografia
  unsigned char tigerKey[10];
  memcpy(tigerKey, "Fatec2019!", 10);
  tiger.InitTiger(tigerKey, 10);
#endif

  // assinatura pacote
  memset(sign, 0, sizeof(sign));
  memcpy(sign, "assinatura", sizeof(sign));
  
  //inicia variaveis de contagem de tempo
  tempo_10ms = millis();
  tempo_1s = millis();
  tempo_1min = millis();
  tempo_5min = millis();

  // Zera buffer utilizado para comunicacao UDP
  memset(bufferUDP, 0, sizeof(bufferUDP));
  
  constroiDisplayMain();
}
//---------------------------------------------------------------------------
void loop()
{
  verifica10ms();
  verifica1s();
  verifica1min();
  verifica5min();   
}
//---------------------------------------------------------------------------
void constroiDisplayMain(){
  lcd.clear();
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
  lcd.setCursor(0, 0);
  lcd.print(hora);lcd.print(':');lcd.print(minuto);
  lcd.setCursor(0, 1);lcd.print("                ");    
  lcd.setCursor(0, 1);lcd.print(nivelComida);lcd.print('%');
  lcd.setCursor(11, 1);lcd.print(pesoPote);lcd.print('g'); 
}
//---------------------------------------------------------------------------
void colocarComida(int qtde)
{
  stPetFeeder=OPERACAO_LOCAL;    
  unsigned long int t=millis();
  float pesoAtual=(float)pesoPote;
  float target=(float)qtde-pesoAtual;
  int count=0;
  
  if(target < 0){
    Serial.println(F("Ainda ha comida, colocarComida() nao sera executado"));
    stPetFeeder=OK; 
    return;
  }
  
  Serial.print(F("Executando acao colocar comida, qtde atual="));Serial.print(pesoPote);Serial.print(F(" target="));Serial.println(target);
  lcd.setCursor(0, 0);lcd.print(F("                "));
  lcd.setCursor(0, 0);lcd.print(F("COLOCANDO COMIDA"));  
  bool rotacaoMotor=true;
  
  while(scale.get_units()*1000 < target){
    Serial.print("Peso=");Serial.println(scale.get_units(), 3);
    if(rotacaoMotor==true) motorHorario(3, 5);
    else motorAntiHorario(3, 5);
    if(count>10)  
    {
      count=0;
      rotacaoMotor=!rotacaoMotor;    
    }
    count++;
    if((millis() - t) > 60000)
    {
      stPetFeeder=ERRO_MOTOR;
      constroiDisplayMain();
      return;
    }
  }   
  stPetFeeder=OK; 
}
//---------------------------------------------------------------------------
