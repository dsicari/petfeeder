/*
 *          S E R V E R
 */

#include <unistd.h> // fork()
#include <signal.h>
#include <sys/types.h> // waitpid()
#include <sys/wait.h>
#include <sys/stat.h> // mkdir/stat

#include "UPetfeeder.h"
#include "UUtil.h"
#include "UNetwork.h"
#include "ULog.h"
#include "USimpleIni.h"
#include "UMySqlWrapper.h"
#include "UCrytiger.h"

#define MAX_LEN         1024
#define LOG_ENABLED     true
//------------------------------------------------------------------------------
using namespace std;
const char *iniFilePath="server.ini";
char logPathDevices[128]={0};
//------------------------------------------------------------------------------
bool setConfigurations(unsigned int *port){
    bool rslt=true;
    TSimpleIni ini;
    memset(logPathDevices, 0, sizeof(logPathDevices));
    if(ini.Load(iniFilePath) == false){
        rslt=false;
    }
    else{
        // Config Server port
        *port = (unsigned int)ini.GetValue<int>("SERVER", "PORT", 50685);
        // Config Log path, modo quiet
        string strLogPathDevices=ini.GetValue<string>("DEFAULT", "LOG_PATH_DEVICES", "/var/log/petfeeder/devices/");
        strcpy(logPathDevices, strLogPathDevices.c_str());   
        string log_path_filename = ini.GetValue<string>("DEFAULT", "LOG_PATH_FILENAME", "/var/log/petfeeder/log");        
        FILE *filePointer = fopen((const char*)log_path_filename.c_str(), "a+");
        if(filePointer != NULL){
            log_set_fp(filePointer);
            log_set_quiet(ini.GetValue<int>("DEFAULT", "LOG_QUIET", 1));
        }
        else{
            rslt=false;
        }
    }
    return rslt;   
}
//------------------------------------------------------------------------------
bool setConfigurationsChild(char *serial){
    bool rslt=true;
    struct stat st = {0};
    char logPathFilenameSerial[128];
    strcpy(logPathFilenameSerial, logPathDevices);
    strcat(logPathFilenameSerial, serial);
    //if (stat(logPathFilenameSerial, &st) == -1) {
    //    if(mkdir(logPathFilenameSerial, 0700) != 0){
    //        rslt=false;
    //    }
    //}
    FILE *filePointer = fopen(logPathFilenameSerial, "a+");
    if(filePointer != NULL){
        log_set_fp(filePointer);
    }
    else{
        rslt=false;
    }    
    return rslt;
}
//------------------------------------------------------------------------------
unsigned long int getToken(){
    TSimpleIni ini;
    unsigned long int token=0;    
    if(ini.Load(iniFilePath) == false) return 0;    
    token=ini.GetValue<unsigned long int>("PACKET", "TOKEN", 0);
    token++;    
    ini.SetValue<unsigned long int>("PACKET", "TOKEN", token);
    if(!ini.SaveAs(iniFilePath)) token=0;
    return token;  
}
//------------------------------------------------------------------------------
int idPacketRcv(unsigned char *buffer, int len){
    int rslt = 0;
    if(buffer[13] == ID_REPORT) rslt=ID_REPORT;
    else if(buffer[13] == CMD_RESP) rslt=CMD_RESP;
    else rslt = -1; // Identificador nao reconhecido 
    return rslt;
}
//------------------------------------------------------------------------------
bool checkCRC(unsigned char *buffer, int len)
{
    bool rslt=false;
    unsigned short crcRecebido=0;
    memcpy(&crcRecebido, &buffer[len-2], 2);
    unsigned short crcCalculado = calccrc16(&buffer[2], len-4); 
    if(crcRecebido == crcCalculado){
        rslt=true;
    }
    return rslt;
}
//------------------------------------------------------------------------------
void sendUDP(unsigned char *buffer, int len, int modo, TCrytiger *tiger, TNetworkServerUDP *serverUDP)
{
    if(modo==CIPHERTEXT){
        tiger->Encrypt(&buffer[2], len-4);
    }
    serverUDP->Send((unsigned char *)&buffer, len);
}//-----------------------------------------------------------------------------
bool conectaBDPetFeeder(TMySqlWrapper *mysql, bool log=false)
{
    bool rslt=false;
    mysql->Init("localhost:6033", "petfeeder", "Fatec2019!");
    if(mysql->Connect() == true)
    {
        if(log==true) log_debug("Conectado ao mysql, token=%ul, pidParent=%d, pid=%d", getppid(), getpid());
        if(mysql->SwitchDb("petfeeder") == true)
        {
            if(log==true) log_debug("SwitchDb OK, token=%ul, pidParent=%d, pid=%d", getppid(), getpid());
            rslt=true;
        }else if(log==true) log_error("Mysql database SwitchDb():%s",mysql->BufferError);
    }else if(log==true) log_error("Mysql database Connect():%s",mysql->BufferError);
    return rslt;
}
//------------------------------------------------------------------------------
bool verificaMsgRet(char *sn, int len, TPacketResponse *pktResponse)
{
    bool rslt=false;
    try
    {
        TMySqlWrapper mysql;
        if(conectaBDPetFeeder(&mysql, LOG_ENABLED) == true)
        {
            if(mysql.Prepare("select * from outgoing where sn_device=? and realizado = 0") == true)
            {
                string str="";
                str.assign(sn, len);
                mysql.SetString(1, str);  
                if(mysql.ExecuteQuery() == true)
                {                
                    mysql.CloseCon();
                    while(mysql.Fetch()){
                        pktResponse->idCmd=mysql.GetULongInt("id");
                        pktResponse->cmd=mysql.GetInt("cmd");
                        memcpy(&pktResponse->valor, mysql.GetString("valor").c_str(), mysql.GetString("valor").length());
                    }
                    rslt=true;
                }
            }
        }
    }       
    catch(exception& e)
    {
        rslt=false;
    }
    
    return rslt;
}
//------------------------------------------------------------------------------
bool updateCmdOutgoing(char *sn, int len, TPacketCmdResponse *pktCmdResponse)
{
    bool rslt=false;
    try
    {
        TMySqlWrapper mysql;
        if(conectaBDPetFeeder(&mysql, LOG_ENABLED) == true)
        {
            if(mysql.Prepare("update outgoing set realizado = ? where id=? and sn_device=?") == true)
            {
                mysql.SetInt(1, pktCmdResponse->cmdStatus);
                mysql.SetULongInt(2, pktCmdResponse->idCmd);
                string str="";
                str.assign(sn, len);
                mysql.SetString(3, str);  
                if(mysql.ExecuteUpdate() == true)
                {                
                    rslt=true;
                }
                mysql.CloseCon();
            }
        }
    }       
    catch(exception& e)
    {
        rslt=false;
    }
    
    return rslt;
}
//------------------------------------------------------------------------------
int main() {
    bool r=false; 
    unsigned int port=0;
    
    r=setConfigurations(&port);
    if(r == false)  exit(EXIT_FAILURE);
    
    TCrytiger tiger;
    unsigned char tigerKey[10];
    memcpy(tigerKey, "Fatec2019!", 10);
    tiger.InitTiger(tigerKey, 10);

    log_info("Server started, pid=%d", getpid());     
    TNetworkServerUDP *serverUDP = new TNetworkServerUDP(port);     
    r=serverUDP->CreateSocket();
    if(r == false){
        log_error("Create Socket fails");
        exit(EXIT_FAILURE);
    }
    
    r=serverUDP->CreateServer();
    if(r == false){
        log_error("Create Server fails");
        exit(EXIT_FAILURE);
    }

    log_info("Server created socket, created server, now start listening...");

    while(1){    
        unsigned char bufferRcv[MAX_LEN];
        int lenBufferRcvData=0;
        pid_t p, pr;
	int status;
        int tipoPacote=0;
        int r=0;
      
        r=serverUDP->Rcv(bufferRcv, sizeof(bufferRcv), &lenBufferRcvData);
        if(r == true && lenBufferRcvData > MAX_LEN)
        {
            // Erro: Pacote recebido excedeu MAX_LEN
            log_error("Server recebeu pacote de %s (%s) e negou %d bytes, max=", serverUDP->GetClientName(), serverUDP->GetClientAddress(), lenBufferRcvData, MAX_LEN); 
        }
        else if(r == true)
        {
            bool erroBuffer=false;
            
            /**** Verifica buffer recebido ****/           
            if(bufferRcv[0] == STX)
            {
                if(bufferRcv[1] == PLAINTEXT){
                    log_debug("Server recebeu pacote de %s (%s) %d bytes, buffer=%s ", serverUDP->GetClientName(), serverUDP->GetClientAddress(), lenBufferRcvData, ucharByteArray2charHexArray(bufferRcv, lenBufferRcvData));
                }
                else if(bufferRcv[1] == CIPHERTEXT){
                    tiger.Decrypt(&bufferRcv[2], lenBufferRcvData-2);
                    log_debug("Server recebeu pacote encriptado de %s (%s) %d bytes, buffer plaintext=%s ", serverUDP->GetClientName(), serverUDP->GetClientAddress(), lenBufferRcvData, ucharByteArray2charHexArray(bufferRcv, lenBufferRcvData));
                }
                else{
                    log_error("Modo nao bateu=%d",bufferRcv[1]);
                    erroBuffer=true;
                }
                
                if(!erroBuffer && checkCRC(bufferRcv, lenBufferRcvData) == false){
                    log_error("CRC pacote recebido nao bateu");
                    erroBuffer=true;
                }
                else{
                    if(memcmp(&bufferRcv[2], "assinatura", LEN_SIGN) != 0){
                        log_error("Assinatura nao bateu (%.10s)", (char *)bufferRcv[2]);
                        erroBuffer=true;
                    }
                }
            }
            else{
                log_error("Iniciador pacote invalido");
                erroBuffer=true;
            }

            if(erroBuffer == true) continue;
                        
            pr=waitpid(-1, &status, WNOHANG);
            if(pr == -1) log_debug("No forked child, waitpid=%d", pr);
            
            p=fork();
            if(p == -1) log_error("Fork fails!!");
            else if(p == 0){
                // Child process forked
                bool task=false;
                unsigned long int token=getToken();
                log_debug("Child process started token=%ul, pidParent=%d, pid=%d", token, getppid(), getpid());                                   
                tipoPacote=idPacketRcv(bufferRcv, sizeof(bufferRcv));
                if(tipoPacote == ID_REPORT){
                    TPacketReport pkt;
                    memcpy(&pkt, bufferRcv, sizeof(TPacketReport));
                    if(setConfigurationsChild(pkt.pid.sn) == true)
                    {
                        log_debug("ID_REPORT received token=%d, pidParent=%d, pid=%d, server received datagram from %s (%s) %d bytes", token, getppid(), getpid(), serverUDP->GetClientName(), serverUDP->GetClientAddress(), lenBufferRcvData);
                        log_debug("identificador=0x%02X", pkt.pid.id);
                        log_debug("sequencia=%ul", pkt.pid.seq);
                        log_debug("status=0x%02X", pkt.pid.st);
                        log_debug("uptime=%ul", pkt.uptime);
                        log_debug("serial=%s", pkt.pid.sn);
                        log_debug("nivel alimento=%i", pkt.nivel_alm);
                        log_debug("peso=%i", pkt.peso);
                        log_debug("horario_alimentacao=%s", pkt.horario_alm);
                        log_debug("quantidade alimento=%i", pkt.qtde_alm);
                        log_debug("crc=0x%02X", pkt.crc);
                        TMySqlWrapper mysql;
                        if(conectaBDPetFeeder(&mysql, LOG_ENABLED) == true)
                        {
                            if(mysql.Prepare("INSERT INTO incoming(pid, seq, st, uptime, sn, nivel_alm, peso, horario_alm, qtde_alm, token, datahora) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, NOW())") == true)
                            { 
                                mysql.SetInt(1, (int)pkt.pid.id);
                                mysql.SetULongInt(2, pkt.pid.seq);
                                mysql.SetInt(3, (int)pkt.pid.st);
                                mysql.SetULongInt(4, pkt.uptime);
                                mysql.SetString(5, pkt.pid.sn);
                                mysql.SetInt(6, (int)pkt.nivel_alm);
                                mysql.SetInt(7, (int)pkt.peso);
                                mysql.SetString(8, pkt.horario_alm);
                                mysql.SetInt(9, (int)pkt.qtde_alm);  
                                mysql.SetULongInt(10, token);
                                if(mysql.ExecuteUpdate() == true)
                                {
                                    log_debug("ExecuteUpdate OK, token=%ul, pidParent=%d, pid=%d", token, getppid(), getpid());
                                    task=true;
                                    mysql.CloseCon();
                                }else log_error("Mysql database ExecuteUpdate():%s",mysql.BufferError);    
                                
                                // Verifica se ha mensagem de retorno  
                                TPacketResponse pktResponse;
                                if(verificaMsgRet(pkt.pid.sn, sizeof(pkt.pid.sn), &pktResponse) == true)
                                {
                                    log_debug("SN %s possui cmd %d valor %s", appendChar2CharArray(pkt.pid.sn, sizeof(pkt.pid.sn), '\0'), pktResponse.cmd, pktResponse.valor);
                                    pktResponse.init.iniciador=STX;
                                    pktResponse.init.modo=pkt.init.modo;
                                    pktResponse.pid.id=pkt.pid.id;
                                    pktResponse.pid.seq=pkt.pid.seq;
                                    memcpy(pktResponse.pid.sign, pkt.pid.sign, sizeof(pkt.pid.sign));
                                    memcpy(pktResponse.pid.sn, pkt.pid.sn, sizeof(pkt.pid.sn));
                                    pktResponse.pid.st=OK;
                                    pktResponse.crc = calccrc16(&pktResponse + 2, sizeof(pktResponse)-4);
                                    sendUDP((unsigned char *)&pktResponse, sizeof(TPacketResponse), pktResponse.init.modo, &tiger, serverUDP);
                                }else log_error("select() retornou false para msg_ret, buffer mysql=%s",mysql.BufferError);
                            }else log_error("Mysql database Prepare():%s",mysql.BufferError);
                            if(task==false) mysql.CloseCon(); // Se nao completou tarefa de insert no banco, fechar conexao
                        }                         
                    }else log_error("setConfigurationsChild()");
                }
                else if(tipoPacote == CMD_RESP){
                    TPacketCmdResponse pktCmdResp;
                    memcpy(&pktCmdResp, bufferRcv, sizeof(TPacketCmdResponse));
                    if(setConfigurationsChild(pktCmdResp.pid.sn) == true)
                    {
                        log_debug("CMD_RESP received token=%d, pidParent=%d, pid=%d, server received datagram from %s (%s) %d bytes", token, getppid(), getpid(), serverUDP->GetClientName(), serverUDP->GetClientAddress(), lenBufferRcvData);
                        log_debug("identificador=0x%02X", pktCmdResp.pid.id);
                        log_debug("sequencia=%ul", pktCmdResp.pid.seq);
                        log_debug("status=0x%02X", pktCmdResp.pid.st);
                        log_debug("serial=%s", pktCmdResp.pid.sn);
                        log_debug("idCmd=%d", pktCmdResp.idCmd);
                        log_debug("cmd=%d", pktCmdResp.cmd);
                        log_debug("cmdStatus=%d", pktCmdResp.cmdStatus);
                        log_debug("crc=0x%02X", pktCmdResp.crc);    
                    }else log_error("setConfigurationsChild()");
                }
                else log_error("Pacote recebido nao reconhecido=%d", tipoPacote);
                   
                break; // encerra child
            }
            else{
                // Parent process
                continue;
            }            
        } 
    }
    serverUDP=NULL;
    delete serverUDP;
}