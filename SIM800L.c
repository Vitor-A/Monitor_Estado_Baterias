#include"Serial.h"
#include <string.h>

extern int1 resposta_SIM;
extern comando_disponivel_UART;
int32 timeout = 7000000;
char coordenada[40];

int1 envia_SIM800L(char *send, char *recive){

  int8 tentativas = 2;

  disable_interrupts(INT_RTCC); 
  enable_interrupts(INT_RDA);                                              
  enable_interrupts(GLOBAL);
  
  do{
    timeout = 7000000;  
    comando_disponivel_UART = 0;

    fprintf(SIM800L_SERIAL,send);                                             // Envia comando para o SIM800L 
    if(strstr(recive,"+CMGS")!=0){
      delay_ms(150);
      fputc(0x1A,SIM800L_SERIAL);
    }                                                                                        
    while(!comando_disponivel_UART && --timeout > 0);                         // Aguarda o SIM800L responder por ~23 segundos caso nao responda retorna 0
                         
    if(comando_disponivel_UART){                                              // Caso receba a resposta 
      comando_disponivel_UART = 0;                  
      posicao_valor_comando = strstr(comando_recibido_UART,recive);           // Procura a palavra recive da funcao na resposta recebida
                
      if(posicao_valor_comando!= 0){                                          // Caso encontre a resposta esperada
        enable_interrupts(INT_RDA);
        enable_interrupts(INT_RTCC);
        enable_interrupts(GLOBAL);
        return 1;
      }
    }
  }while(--tentativas>0);                
  
  enable_interrupts(GLOBAL);
  enable_interrupts(INT_RDA);
  enable_interrupts(INT_RTCC);

  return 0;
}

int1 Send_SMS(char *numero,char *mensagem){
  
  char numero_envio[35];

  sprintf(numero_envio,"AT+CMGS=\"%s\"\r\n",numero);

  if(envia_SIM800L("AT+CMGF=1\r\n","OK"))
    if(envia_SIM800L("AT+CSMP=17,255,0,0\r\n","OK"))
      if(envia_SIM800L(numero_envio, ">"))
        if(envia_SIM800L(mensagem, "+CMGS:"))
          return 1;
  return 0;
}

int1 Read_SMS(){
  
  if(envia_SIM800L("AT+CMGF=1\r\n","OK")){
    if(envia_SIM800L("AT+CMGL=\"REC UNREAD\"\r\n", "")){
      resposta_SIM = true;
      return 1;    
    }  
  }
  return 0;
}

int1 Get_Locate(char *numero){
  
  if(envia_SIM800L("AT+CMGF=1\r\n","OK"))
    if(envia_SIM800L("AT+CGATT=1\r\n","OK"))
      if(envia_SIM800L("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\r\n","OK"))
        if(envia_SIM800L("AT+SAPBR=3,1,\"APN\",\"CMNET\"\r\n","OK"))
          if(envia_SIM800L("AT+SAPBR=1,1\r\n","OK"))
            if(envia_SIM800L("AT+SAPBR=2,1\r\n","+SAPBR:"))
              if(envia_SIM800L("AT+CLBSCFG=0,1\r\n","+CLBSCFG:")){                
                if(envia_SIM800L("AT+CLBS=1,1\r\n","+CLBS:")){
                  Get_Coordenadas();
                  Send_SMS(numero,coordenada); 
                  if(envia_SIM800L("AT+SAPBR=0,1\r\n","OK"))
                    return 1; 
                }
              }
 
  envia_SIM800L("AT+SAPBR=0,1\r\n","OK");
  return 0;   
}

void Get_Coordenadas(){
 
 int8 index=0;
 int8 index2=0;

  memset (coordenada, 0x00, sizeof(coordenada));
  
  coordenada[0] ='L';
  coordenada[1] ='O';
  coordenada[2] ='C';
  coordenada[3] ='A';
  coordenada[4] ='T';
  coordenada[5] ='E';
  coordenada[6] =':';
  coordenada[7] =' ';
 
  index2 = 8;
  for(index=22;index<33;index++){ //Latitude
   coordenada[index2] = comando_recibido_UART[index];
   index2++;   
  }

  for(index=11;index<22;index++){ //Longitude
   coordenada[index2] = comando_recibido_UART[index];
   index2++;   
  }
  
  coordenada[index2++] =' ';
  coordenada[index2++] ='R';
  coordenada[index2++] ='=';

  for(index=33;index<36;index++){ //Precisão
   coordenada[index2] = comando_recibido_UART[index];
   index2++;   
  }
  
  coordenada[index2++] ='m';
  coordenada[index2] = 0;

}