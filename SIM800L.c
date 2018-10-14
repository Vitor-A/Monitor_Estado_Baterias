#include"Serial.h"

extern int1 resposta_SIM;
extern comando_disponivel_UART;
int32 timeout = 7000000;

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
      fputc(0x1A,SIM800L_SERIAL);
      fprintf(SIM800L_SERIAL,"\r\n"); 
    }                                                                                        
    while(!comando_disponivel_UART && --timeout > 0);                         // Aguarda o SIM800L responder por ~23 segundos caso nao responda retorna 0
                         
    if(comando_disponivel_UART){                                              // Caso receba a resposta 
                       
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
  
  char numero_envio[30];

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