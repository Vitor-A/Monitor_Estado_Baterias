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

// Declaração de Variáveis

int1 comando_disponivel_UART = FALSE;
int1 One_Second = FALSE;
int1 One_Minute = FALSE;
int16 Miliseconds = 0;
int16 seconds = 0;
char comando = 0;
int1 resposta_SIM = false;
int8 leitura_tensao[500];
int16 leitura_corrente[100];
int1 partida_iniciada = FALSE;
int1 aquisicao_tensao_partida = FALSE;
int1 veiculo_ligado = FALSE;
int16 index = 0;
int8 V1 = 255;
int8 V2 = 255;
//Fim declaração de variáveis

#INT_RTCC
void Timer_0(){

  set_timer0(6);  
  
  if(partida_iniciada){
    
    if(index<500){ 

      leitura_tensao[index] = (read_adc()>>2);
      output_toggle(PIN_A0);
      index++;
    } 
    else{
      partida_iniciada = FALSE;
      aquisicao_tensao_partida = TRUE;
      enable_interrupts(INT_RDA);
      enable_interrupts(INT_EXT);
    }
  }
  else{
    Miliseconds++;
    
    if(Miliseconds == 1000){
      seconds++;
      Miliseconds = 0;
      One_Second = TRUE;
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
  
  enable_interrupts(INT_RTCC);
  enable_interrupts(INT_RDA);
  enable_interrupts(INT_EXT_H2L);
  enable_interrupts(GLOBAL); 
  output_low(PIN_D2);
  delay_ms(2000);
  output_high(PIN_D2);
  delay_ms(15000);
  Send_SMS("031995822739","INICIANDO...");

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

void Executar_Cada_Segundo(){
  
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
  

  return;
}

void Obtem_SOH(){
 
  V1 = 255;
  V2 = 255;
  index = 0;

  for(index=0;index<15;index++){
    
    if(leitura_tensao[index]<V1)
      V1 = leitura_tensao[index];
   
  }
  

  return;
}