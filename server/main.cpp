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

#define MAX_LEN 1024
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
    else if(buffer[13] == ID_TESTE) rslt=ID_TESTE;
    else rslt = -1; // Identificador nao reconhecido 
    return rslt;
}
//------------------------------------------------------------------------------
int parseReport(unsigned char *buffer, int len, TPacketReport *tmp){
    int rslt = 0;
    unsigned short crcRecebido=0;
    memcpy(&crcRecebido, &buffer[len-2], 2);
    unsigned short crcCalculado = calccrc16(&buffer[2], len-4); 
    if(crcRecebido == crcCalculado){
        memcpy(tmp, buffer, sizeof(TPacketReport));
        rslt=0;
    }
    else rslt = -1; // CRC nao bateu
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
        if(r == true && lenBufferRcvData > MAX_LEN){
            log_error("Server received datagram from %s (%s) and droped %d bytes, max=", 
                        serverUDP->GetClientName(), 
                        serverUDP->GetClientAddress(),
                        lenBufferRcvData,
                        MAX_LEN); 
        }
        else if(r == true){
            pr=waitpid(-1, &status, WNOHANG);
            if(pr == -1) log_debug("No forked child, waitpid=%d", pr);
            p=fork();
            if(p == -1){
                log_error("Fork fails, exiting...");
                exit(EXIT_FAILURE);
            }
            if(p == 0){
                // Child process forked
                unsigned long int token=getToken();
                if(bufferRcv[0] == STX){
                    if(bufferRcv[1] == PLAINTEXT || bufferRcv[1] == CIPHERTEXT){
                        log_debug("Child process started token=%ul, pidParent=%d, pid=%d, server received datagram from %s (%s) %d bytes, buffer=%s ", 
                                    token,
                                    getppid(),
                                    getpid(),
                                    serverUDP->GetClientName(), 
                                    serverUDP->GetClientAddress(), 
                                    lenBufferRcvData,
                                    ucharByteArray2charHexArray(bufferRcv, lenBufferRcvData));
                        
                        if(bufferRcv[1] == CIPHERTEXT){
                            tiger.Decrypt(&bufferRcv[2], lenBufferRcvData-2);
                            log_debug("Token=%d, pidParent=%d, pid=%d, buffer decrypted=%s ", 
                                        token,
                                        getppid(),
                                        getpid(),
                                        ucharByteArray2charHexArray(bufferRcv, lenBufferRcvData));
                        }
                        
                        if(memcmp(&bufferRcv[2], "assinatura", LEN_SIGN) != 0){
                            log_error("Token=%d, pidParent=%d, pid=%d, assinatura nao bateu (%.10s)", token, getppid(), getpid(), (char *)bufferRcv[2]);
                        }
                    }
                    else{
                        log_error("Modo nao bateu (%d) token=%ul, pidParent=%d, pid=%d", bufferRcv[1], token, getppid(), getpid());
                        exit(EXIT_FAILURE);
                    }
                }
                else{
                    log_error("Iniciador nao bateu (%d) token=%ul, pidParent=%d, pid=%d", bufferRcv[0], token, getppid(), getpid());
                    exit(EXIT_FAILURE);
                }                
                tipoPacote=idPacketRcv(bufferRcv, sizeof(bufferRcv));
                if(tipoPacote == ID_REPORT){
                    TPacketReport pkt;
                    r=parseReport(bufferRcv, sizeof(TPacketReport), &pkt);
                    if(r == 0){
                        if(setConfigurationsChild(pkt.pid.sn) == true){
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
                            mysql.Init("localhost:6033", "petfeeder", "fatec2019!");
                            mysql.Connect();
                            mysql.SwitchDb("petfeeder");                            
                            log_debug("Mysql database connected? buffer:%s",mysql.BufferError);
                            mysql.Prepare("INSERT INTO incoming(pid, seq, st, uptime, sn, nivel_alm, peso, horario_alm, qtde_alm, token, datahora) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, NOW())");
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
                            mysql.ExecuteUpdate();
                            log_debug("Pacote recebido e tentado inserir no banco: %s", mysql.BufferError);
                            mysql.CloseCon();                      
                        }                        
                    }
                    else{
                        log_error("parseReport=%d",r);
                    }
                }
                /* SELECT ZUADO! https://github.com/original-work/MySQL-Connector-Cpp-Wrapper-Class/blob/master/main.cpp */
                else if(tipoPacote == ID_TESTE){
                    printf("ID_TESTE received token=%d, pidParent=%d, pid=%d, server received datagram from %s (%s) %d bytes", token, getppid(), getpid(), serverUDP->GetClientName(), serverUDP->GetClientAddress(), lenBufferRcvData);
                    TMySqlWrapper mysql;
                    mysql.Init("localhost:6033", "petfeeder", "fatec2019!");
                    mysql.Connect();
                    mysql.SwitchDb("petfeeder");                            
                    log_debug("Mysql database connected? buffer:%s",mysql.BufferError);
                    mysql.ExecuteQuery("select * from device");
                    mysql.CloseCon();
                    while(mysql.Fetch()){
                        printf("serial=%s\n", mysql.GetString("sn").c_str());
                        printf("id_usuario=%d\n", mysql.GetInt("id_usuario"));
                        printf("datahora=%s\n", mysql.GetString("datahora").c_str()); 
                    }                                                                  
                }
                else{
                    log_error("Pacote recebido nao reconhecido=%d", tipoPacote);
                }
                    
                //serverUDP->Send((unsigned char *)"OK", 2);
                
                exit(EXIT_SUCCESS);
            }
            else{
                // Parent process
                continue;
            }            
        } 
    }
}