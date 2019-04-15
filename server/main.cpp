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
#include "UMySqlConnector.h"
//#include "rc4.h"

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
    if(buffer[0] == STX){
        if(buffer[1] == ID_REPORT) rslt=ID_REPORT;
        else if(buffer[1] == ID_TESTE) rslt=ID_TESTE;
        else rslt = -1; // Identificador nao reconhecido
    }  
    return rslt;
}
//------------------------------------------------------------------------------
int parseReport(unsigned char *buffer, int len, TPacketReport *tmp){
    int rslt = 0;
    unsigned short crcRecebido=0;
    memcpy(&crcRecebido, &buffer[len-2], 2);
    unsigned short crcCalculado = calccrc16(buffer, len-2); 
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
                log_error("Fork fails, exiting");
                exit(EXIT_FAILURE);
            }
            if(p == 0){
                // Child process forked
                unsigned long int token=getToken();
                log_debug("Child process started, token=%d, pidParent=%d, pid=%d, server received datagram from %s (%s) %d bytes, %s ", 
                            token,
                            getppid(),
                            getpid(),
                            serverUDP->GetClientName(), 
                            serverUDP->GetClientAddress(), 
                            lenBufferRcvData,
                            ucharByteArray2charHexArray(bufferRcv, lenBufferRcvData));                 
                tipoPacote=idPacketRcv(bufferRcv, sizeof(bufferRcv));
                if(tipoPacote == ID_REPORT){
                    TPacketReport packetReport;
                    r=parseReport(bufferRcv, sizeof(TPacketReport), &packetReport);
                    if(r == 0){
                        if(setConfigurationsChild(packetReport.sn) == true){
                            log_debug("ID_REPORT received, token=%d, pidParent=%d, pid=%d, server received datagram from %s (%s) %d bytes", token, getppid(), getpid(), serverUDP->GetClientName(), serverUDP->GetClientAddress(), lenBufferRcvData);
                            log_debug("identificador=0x%02X", packetReport.pid);
                            log_debug("sequencia=%ul", packetReport.seq);
                            log_debug("status=0x%02X", packetReport.st);
                            log_debug("uptime=%ul", packetReport.uptime);
                            log_debug("serial=%s", packetReport.sn);
                            log_debug("nivel alimento=%i", packetReport.nivel_alm);
                            log_debug("peso=%i", packetReport.peso);
                            log_debug("horario_alimentacao=%s", packetReport.horario_alm);
                            log_debug("quantidade alimento=%i", packetReport.qtde_alm);
                            log_debug("crc=0x%02X", packetReport.crc);
                            TMySqlConnector *mysql=new TMySqlConnector("localhost:6033", "petfeeder", "petfeeder", "fatec2019!");
                            if(mysql->ConnectDB() == true){
                                log_debug("Mysql database connected...");
                                mysql->Prepare("INSERT INTO incoming(pid, seq, st, uptime, sn, nivel_alm, peso, horario_alm, qtde_alm, token, datahora) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, NOW())");
                                mysql->SetInt(1, (int)packetReport.pid);
                                mysql->SetULongInt(2, packetReport.seq);
                                mysql->SetInt(3, (int)packetReport.st);
                                mysql->SetULongInt(4, packetReport.uptime);
                                mysql->SetString(5, packetReport.sn);
                                mysql->SetInt(6, (int)packetReport.nivel_alm);
                                mysql->SetInt(7, (int)packetReport.peso);
                                mysql->SetString(8, packetReport.horario_alm);
                                mysql->SetInt(9, (int)packetReport.qtde_alm);  
                                mysql->SetULongInt(10, token);
                                if(mysql->ExecuteUpdate("") == true) log_debug("Pacote recebido e inserido no banco");
                                else log_error("Pacote nao inserido no banco, %s", mysql->BufferError);
                                mysql->CloseCon();
                            }
                            else{
                                log_error("Mysql database connect error: %s", mysql->BufferError);
                            }
                            log_debug("ending child...");
                            mysql=NULL;
                            delete mysql;                         
                        }                        
                    }
                    else{
                        log_error("parseReport=%d",r);
                    }
                }
                /* SELECT ZUADO! https://github.com/original-work/MySQL-Connector-Cpp-Wrapper-Class/blob/master/main.cpp */
                else if(tipoPacote == ID_TESTE){
                    printf("ID_TESTE received, token=%d, pidParent=%d, pid=%d, server received datagram from %s (%s) %d bytes", token, getppid(), getpid(), serverUDP->GetClientName(), serverUDP->GetClientAddress(), lenBufferRcvData);
                    sleep(1);
                    TMySqlConnector *mysql=new TMySqlConnector("localhost:6033", "petfeeder", "petfeeder", "fatec2019!");
                    if(mysql->ConnectDB() == true){
                        if(mysql->ExecuteQuery("select * from device") == true){
                            mysql->CloseCon();
                            while(mysql->Fetch()){
                                printf("OK");
                            }                            
                            //printf("serial=%s\n", mysql->GetString("sn").c_str());
                            //printf("id_usuario=%d\n", mysql->GetInt("id_usuario"));
                            //printf("datahora=%s\n", mysql->GetString("datahora").c_str());
                        }
                        else{
                            printf("erro: %s", mysql->BufferError);   
                        }                        
                        mysql->CloseCon(); 
                        log_debug("ending child...");
                        mysql=NULL;
                        delete mysql;
                    }
                    else{
                        log_error("Mysql database connect error: %s", mysql->BufferError);
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