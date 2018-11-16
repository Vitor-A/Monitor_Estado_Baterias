// Projeto Monitor do Estado de Baterias - TAI 
// Data: Setembro de 2018


//******************* Interrupção do Timer 0 / RTCC **************************
//  
//  Cristal externo = 16 MHz - Uso de PLL*4 -> Frequencia de trabalho = 16MHz 
//  Prescaler = 1:16
//  Tout =(4*prescaler* (256-TMR0))/fclk  
//  Para interrup��o a cada 1 ms
//  TMR0 = 6 
// 
//****************************************************************************

#include <18F4550.h>

#fuses NOWDT,NOPROTECT,NOLVP,MCLR,HSPLL,PLL4
#device PASS_STRINGS=IN_RAM,adc=10
#use delay(clock=16000000)
#use rs232(uart1,baud=115200, xmit=PIN_C6, rcv=PIN_C7, stream = SIM800L_SERIAL, ERRORS)
#use rs232(baud=9600, xmit=PIN_B2, rcv=PIN_B3, stream = MONITOR_SERIAL)
#use fast_io (ALL)
#priority ext,rda, rtcc,

#include <stdlib.h>
#include "TAI_Bateria.h"
#include "SIM800L.h"
#include "Memory.h"
#include "numeros.h"

// Declaração de Variáveis

int1 comando_disponivel_UART = FALSE;
int1 One_Second = FALSE;
int1 One_Minute = FALSE;
int16 Miliseconds = 0;
int16 seconds = 0;
char comando = 0;
int1 resposta_SIM = false;
int8 leitura_tensao_partida[400];
int1 partida_iniciada = FALSE;
int1 aquisicao_tensao_partida = FALSE;
int1 veiculo_ligado = FALSE;
int16 index = 0;
unsigned int8 index_1 = 0;
unsigned int8 index_2 = 0;
int8 V1 = 255;
int8 V2 = 255;
int1 sel_ad = FALSE;
unsigned int16 leitura_corrente[100];
unsigned int16 offset_current_ref[100];
unsigned int8 leitura_tensao[100];
unsigned int16 aux_leitura[100];
float aux_offset_current_ref = 0;
float aux_corrente = 0;
float aux_tensao = 0;
float zero_set = 0;
float zero_set_aux = 0;
int32 corrente_limite = 3000;
int8 qtd_numeros=0;
int8 tempo_corrente_verif = 0;
int8 tempo_corrente_verif_low = 0; 
int8 index_envio = 0;
int32 tempo_entre_alertas = 1;
int32 tempo_ultimo_alerta = 1;
int1 detect_high_current = false;
char numero[20];
//Fim declaração de variáveis

#INT_RTCC
void Timer_0(){

  set_timer0(6);  
  
  if(partida_iniciada){
    
    output_high(PIN_D0);
    if(index<400){ 
        
      leitura_tensao_partida[index] = (read_adc()>>2);
      index++;
    } 
    else{
      index = 0;
      partida_iniciada = FALSE;
      aquisicao_tensao_partida = TRUE;
      output_low(PIN_D0);
      enable_interrupts(INT_RDA);
      enable_interrupts(INT_EXT);
    }
  }
  else{
    Miliseconds++;

    if(Miliseconds % 5 == 0){
      sel_ad = !sel_ad;
   
      if(sel_ad){
        leitura_corrente[index_1] = read_adc();
        set_adc_channel(3);
        delay_us(10);
        offset_current_ref[index_1] = read_adc();
        set_adc_channel(1);
        index_1++;
      }
      else{
        leitura_tensao[index_2] = (read_adc()>>2);
        set_adc_channel(2);
        index_2++;
      } 
    }

    if(Miliseconds == 1000){
      seconds++;
      Miliseconds = 0;
      One_Second = TRUE;
      index_1 = 0;
      index_2 = 0;
      output_toggle(PIN_A0);
      if(seconds==60){
        seconds = 0;
        One_Minute = TRUE;
      }
    }
  }
  return;
}

#INT_EXT
void Interrupcao_Externa(){
  
  set_adc_channel(1);
  disable_interrupts(INT_RDA);
  disable_interrupts(INT_EXT);
  partida_iniciada = TRUE;
  veiculo_ligado = TRUE;
  aquisicao_tensao_partida = FALSE;
  index = 0;

  return;
}

void main()
{
  set_tris_a (0b00001110);                                                     //Ra7-Ra6-Ra5-Ra4-Ra3-Ra2-Ra1-Ra0
  set_tris_b (0b00001001);                                                     //Rb7-Rb6-Rb5-Rb4-Rb3-Rb2-Rb1-Rb0
  set_tris_c (0b10000000);                                                     //Rc7-Rc6-Rc5-Rc4-Rc3-Rc2-Rc1-Rc0
  set_tris_d (0b00000000); 
  
  setup_adc_ports(AN0_TO_AN3);
  setup_adc(ADC_CLOCK_DIV_2);
  set_adc_channel(1);
  setup_wdt(WDT_OFF);
  setup_timer_0(RTCC_INTERNAL | RTCC_DIV_16 | RTCC_8_BIT);
  set_timer0(6);
  ext_int_edge(H_TO_L);

  output_low(PIN_D0);
  enable_interrupts(INT_RTCC);
  enable_interrupts(INT_RDA);
  enable_interrupts(GLOBAL);

  le_EEPROM(); 
  output_low(PIN_D2);
  delay_ms(2000);
  output_high(PIN_D2);
  delay_ms(15000);
  Send_SMS("031995822739","INICIANDO...");

  enable_interrupts(INT_EXT_H2L);

  while(TRUE){

    if(One_Second){

      One_Second = FALSE;
      Executar_Cada_Segundo();

    }

    if(One_Minute){

      One_Minute = FALSE;
      Executar_Cada_Minuto();

    }

  }

}

void le_EEPROM(void){

  read_config(ADDR_tempo_entre_alertas,&tempo_entre_alertas,4);
  read_config(ADDR_tempo_ultimo_alerta,&tempo_ultimo_alerta,4);
  read_config(ADDR_corrente_limite,&corrente_limite,4);
  read_config(ADDR_zero_set,&zero_set,4);
  read_config(ADDR_qtd_numeros,&qtd_numeros,1);
  read_config(ADDR_vector_numeros,&numeros,50);

}

void Executar_Cada_Segundo(){
  
  Calcula_SOC();
  if(corrente_limite != 0){
    if(aux_corrente > corrente_limite){
      tempo_corrente_verif++;
      if(tempo_corrente_verif > 30){
        tempo_corrente_verif = 0;
        tempo_corrente_verif_low = 0;
        detect_high_current = true;
        disable_interrupts(GLOBAL);
        fprintf(MONITOR_SERIAL,"Detect High Current\r\n");
        enable_interrupts(GLOBAL);
      }
    }
    else{
      tempo_corrente_verif_low++;
      if(tempo_corrente_verif_low > 30){
        tempo_corrente_verif_low = 0;
        tempo_corrente_verif = 0;
        detect_high_current = false;
        disable_interrupts(GLOBAL);
        fprintf(MONITOR_SERIAL,"Detect Low Current\r\n");
        enable_interrupts(GLOBAL);
      }
      
    }
  }
  if(aquisicao_tensao_partida){

    aquisicao_tensao_partida = FALSE;
    Obtem_SOH();

  }
  if(comando_disponivel_UART){
  
    disable_interrupts(GLOBAL);
    comando = Get_Comando();
    
    if(comando != 0){

      Executa_Comando(comando);
      
    }
    if(resposta_SIM == 1){
      disable_interrupts(GLOBAL);
      resposta_SIM = 0;
      comando = Get_Comando();
      Executa_Comando(comando);
      delay_ms(100);
      envia_SIM800L("AT+CMGD=1,1\r\n", "OK");
    }
    comando = 0;
    clear_command();
  }
  
  return;
}

void Executar_Cada_Minuto(){
  
  index_envio = 0;
  if(detect_high_current){
    
    if((qtd_numeros>0 && qtd_numeros<4) && tempo_entre_alertas>0){
      tempo_ultimo_alerta++;
      disable_interrupts(INT_EXT); 

      if(tempo_ultimo_alerta >= tempo_entre_alertas){   
        tempo_ultimo_alerta = 0;
        for(index_envio = 0; index_envio < qtd_numeros; index_envio++){
          memset (numero, 0x00, sizeof(numero));
          obtem_numero(index_envio,numero);
          disable_interrupts(GLOBAL);
          fprintf(MONITOR_SERIAL,"Send SMS Numero (%u/%u): %s\r\n",index_envio+1,qtd_numeros,numero);
          enable_interrupts(GLOBAL);
          Send_SMS(numero,"!!!!ALERTA: Bateria em descarga rapida VERIFIQUE SEU VEICULO!!!");
  
        }
      }
      enable_interrupts(INT_EXT);
    }
      
  }  
  return;
}

void Obtem_SOH(){
 
  V1 = 255;
  V2 = 255;
  index = 0;

  for(index=0;index<15;index++){
    
    if(leitura_tensao_partida[index]<V1)
      V1 = leitura_tensao[index];
   
  }
  

  return;
}

void Calcula_SOC(){
  
  int8 i=0;
  char exibe[50];

  for(i=0; i<100;i++)
    aux_leitura[i] = leitura_corrente[i];
  aux_corrente = obtem_mediana(aux_leitura,100);
  
  for(i=0; i<100;i++)
    aux_leitura[i] = offset_current_ref[i];
  aux_offset_current_ref = obtem_mediana(aux_leitura,100);

  for(i=0; i<100;i++)
    aux_leitura[i] = leitura_tensao[i];
  aux_tensao = obtem_mediana(aux_leitura,100);
  
  aux_tensao = aux_tensao*(15.0/255.0);
  zero_set_aux = (aux_corrente-(aux_offset_current_ref));
  aux_corrente = (zero_set_aux-zero_set)*355;
  if(aux_corrente > -750 && aux_corrente < 750) aux_corrente = 0;

  disable_interrupts(GLOBAL);
  sprintf(exibe,"Corrente: %6.0f Tensao: %2.2f \r\n",aux_corrente,aux_tensao);
  fprintf(MONITOR_SERIAL,exibe);
  enable_interrupts(GLOBAL);

}

unsigned int16 obtem_mediana(unsigned int16 *num, int8 tam){  

  int16 aux; 
  int16 i, j;

  for (i = 0; i < (tam-1); i++){
    for (j = i+1; j < tam; j++){
      if (num[i] < num[j])
      {
        aux = num[i];
        num[i] = num[j];
        num[j] = aux;
      }
    }
  }
  return num[50]; 
}