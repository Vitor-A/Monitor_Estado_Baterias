#include"SIM800L.h"
#include"TAI_Bateria.h"
#include"numeros.h"
#include "Memory.h"

#define   UART_BUFFER_SIZE         150

char comando_recibido_UART[UART_BUFFER_SIZE];
char comando_recibido_BUFF [UART_BUFFER_SIZE];
int *posicao_valor_comando = 0;
int16 timeout_trama_UART = 100000;
extern int1 comando_disponivel_UART;
extern int32 corrente_limite;
extern int32 tempo_entre_alertas;
extern float aux_corrente;
extern float aux_tensao;
extern float zero_set;
extern float zero_set_aux;

#INT_RDA 
void recepcao_UART()
{
 int i = 1;
 int8 caracterRx;
   if(kbhit(SIM800L_SERIAL) && comando_disponivel_UART == 0) {
      caracterRx = getc(SIM800L_SERIAL);
      if(caracterRx == '>'||caracterRx == '+' ||caracterRx =='\n'||caracterRx =='\r') {
        comando_recibido_UART[0] = caracterRx;
        while(i < (UART_BUFFER_SIZE-2) && timeout_trama_UART > 0) {
           if(kbhit(SIM800L_SERIAL)) {
             caracterRx = getc(SIM800L_SERIAL);
             comando_recibido_UART[i] = caracterRx;
             if(caracterRx == ';') { 
               i++;
               break;
             }
             i++;
             timeout_trama_UART = 100000;
           }
           timeout_trama_UART--;
        }
        comando_disponivel_UART = 1;
        timeout_trama_UART = 100000;
        comando_recibido_UART[i] = 0;
      }
   }
  return;
}

char Get_Comando()
{
  char CMD[15];
  strcpy (comando_recibido_BUFF, comando_recibido_UART);                           
  strcpy (CMD, "+CMTI"); posicao_valor_comando = strstr(comando_recibido_BUFF,CMD); if(posicao_valor_comando!= 0)return(SMS_COMMAND);
  strcpy (CMD, "+CMGL"); posicao_valor_comando = strstr(comando_recibido_BUFF,CMD); if(posicao_valor_comando!= 0)return(SMS_READ_COMMAND);

  return(0);
}


void Executa_Comando(char comando){

  char CMD[15];
  char numero[20];

  disable_interrupts(GLOBAL);
  
  switch(comando){
     
    case SMS_COMMAND:       //+CMTI: "SM",10<CR><LF>
      Read_SMS();
    break;

    case SMS_READ_COMMAND:  //+CMGL: 1,"REC UNREAD","+5531995822739","","18/10/13,21:57:54-12"<CR><LF>

      strcpy (CMD, "OITAI");posicao_valor_comando = strstr(comando_recibido_BUFF,CMD);
      if(posicao_valor_comando!= 0){

        get_numero(14,25,numero);
        Send_SMS(numero,"HEARTBEAT()");
        break;
      }

      strcpy (CMD, "+NUMADD");posicao_valor_comando = strstr(comando_recibido_BUFF,CMD); //+CMGL: 1,"REC UNREAD","+5531995822739","","18/10/14,00:28:46-12"nr+NUMADD+5531995422738<CR><LF>
      if(posicao_valor_comando!= 0){
        
        get_numero(14,75,numero);
        insere_numero(numero);
        write_config(ADDR_qtd_numeros,&qtd_numeros,1);
        write_config(ADDR_vector_numeros,&numeros,50);
        get_numero(14,25,numero);
        Send_SMS(numero,numeros);
        break;
      }

      strcpy (CMD, "+NUMDEL");posicao_valor_comando = strstr(comando_recibido_BUFF,CMD); //+NUMDEL
      if(posicao_valor_comando!= 0){
         
        limpa_numeros(); 
        write_config(ADDR_qtd_numeros,&qtd_numeros,1);
        write_config(ADDR_vector_numeros,&numeros,50);
        get_numero(14,25,numero);
        Send_SMS(numero,"OK CLEAR ALL");
        break;
      }

      strcpy (CMD, "+GSMLOCATE");posicao_valor_comando = strstr(comando_recibido_BUFF,CMD); //+GSMLOCATE
      if(posicao_valor_comando!= 0){
         
        get_numero(14,25,numero); 
        Get_Locate(numero); 
        break;
      }

      strcpy (CMD, "+CURLIM");posicao_valor_comando = strstr(comando_recibido_BUFF,CMD); //rn+CMGL: 1,"REC UNREAD","+5531995822739","","18/11/10,18:12:21-08"rn+CURLIM000001rn

      if(posicao_valor_comando!= 0){
         
        get_numero(14,25,numero);
        corrente_limite = get_value(posicao_valor_comando,6,7); //+CURALM001<CR><LF>
        write_config(ADDR_corrente_limite,&corrente_limite,4);
        memset (comando_recibido_BUFF, 0x00, sizeof(comando_recibido_BUFF));
        sprintf(comando_recibido_BUFF, "Corrente Limite = %Lu mA",corrente_limite);
        Send_SMS(numero,comando_recibido_BUFF);
        break;
      }

      strcpy (CMD, "+CURALM");posicao_valor_comando = strstr(comando_recibido_BUFF,CMD); //rn+CMGL: 1,"REC UNREAD","+5531995822739","","18/11/10,18:09:52-08"rn+CURALM001rn
      if(posicao_valor_comando!= 0){
         
        get_numero(14,25,numero);
        tempo_entre_alertas = get_value(posicao_valor_comando,3,7); //+CURLIM000001<CR><LF>
        write_config(ADDR_tempo_entre_alertas,&tempo_entre_alertas,4);
        memset (comando_recibido_BUFF, 0x00, sizeof(comando_recibido_BUFF));
        sprintf(comando_recibido_BUFF, "Tempo Entre Alertas = %Lu min",tempo_entre_alertas);
        Send_SMS(numero,comando_recibido_BUFF);
        break;
      }

      strcpy (CMD, "+CSTATS");posicao_valor_comando = strstr(comando_recibido_BUFF,CMD); //+CSTATS
      if(posicao_valor_comando!= 0){
         
        get_numero(14,25,numero);
        memset (comando_recibido_BUFF, 0x00, sizeof(comando_recibido_BUFF));
        sprintf(comando_recibido_BUFF, "Tempo Entre Alertas = %Lu min / Corrente Limite = %Lu mA / Corrente Atual: %6.0f mA / Tensao Atual: %2.2f V / Qtd Telefones: %u",tempo_entre_alertas,corrente_limite,aux_corrente,aux_tensao,qtd_numeros);
        Send_SMS(numero,comando_recibido_BUFF);
        if(qtd_numeros>0) Send_SMS(numero,numeros);     
           
        break;
      }

      strcpy (CMD, "+SETZERO");posicao_valor_comando = strstr(comando_recibido_BUFF,CMD); //+SETZERO
      if(posicao_valor_comando!= 0){
         
        get_numero(14,25,numero);
        zero_set = zero_set_aux; 
        write_config(ADDR_zero_set,&zero_set,4);
        Send_SMS(numero,"ZERO SETED");

        break;
      }
         
    break;


  }

  return;

}

int32 get_value(char *pointer_to_string, int nro_caracteres,int posicao) //get_value(posicao_valor_comando,3,6);
{
  int32 valor = 0;
  int i = 0;
  int32 aux = 1;
  int potencia = 0;

  while(nro_caracteres > 0)
  {
    if( pointer_to_string[posicao+i] != '.' )
    {
      nro_caracteres--;
      potencia = nro_caracteres;
      aux = 1;
      while(potencia > 0)
      {
        aux = aux * 10;
        potencia--;
      }
      valor = valor + ((int32)(pointer_to_string[posicao+i]-48)*aux);
    }
    i++;
  }

  return(valor);
}

void get_numero(int nro_caracteres, int posicao, char *destino){

  int8 index = 0;

  for(index = 0;index< nro_caracteres;index++){

    destino[index] = comando_recibido_BUFF[index+posicao];

  }

  destino[index] = '\0';

  return;
}

void clear_command(){

  delay_ms(200);
  memset (comando_recibido_UART, 0x00, sizeof(comando_recibido_UART));
  memset (comando_recibido_BUFF, 0x00, sizeof(comando_recibido_BUFF));

  comando_disponivel_UART = FALSE;
  enable_interrupts(GLOBAL);
  enable_interrupts(INT_RTCC);
  enable_interrupts(INT_RDA);
  
  return;
}