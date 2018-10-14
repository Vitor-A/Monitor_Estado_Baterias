// Projeto Monitor do Estado de Baterias - TAI 
// Data: Setembro de 2018


//******************* Interrupção do Timer 0 / RTCC **************************
//  
//  Cristal externo = 16 MHz - Uso de PLL*4 -> Frequencia de trabalho = 16MHz 
//  Prescaler = 1:16
//  Tout =(4*prescaler* (256-TMR0))/fclk  
//  Para interrupção a cada 1 ms
//  TMR0 = 6 
// 
//****************************************************************************

#include <18F4550.h>

#fuses NOWDT,NOPROTECT,NOLVP,MCLR,HSPLL,PLL4
#device PASS_STRINGS=IN_RAM
#use delay(clock=16000000)
#use rs232(uart1,baud=115200, xmit=PIN_C6, rcv=PIN_C7, stream = SIM800L_SERIAL, ERRORS)
#use fast_io (ALL)
#priority rda, rtcc,

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
//Fim declaração de variáveis

#INT_RTCC
void Timer_0(){

  set_timer0(6);  
  Miliseconds++;

  if(Miliseconds == 1000){
    seconds++;
    Miliseconds = 0;
    One_Second = TRUE;
    output_toggle(PIN_A1);
    if(seconds==60){
      seconds = 0;
      One_Minute = TRUE;
    }
  }
  return;
}

void main()
{
  set_tris_a (0b00000000);                                                     //Ra7-Ra6-Ra5-Ra4-Ra3-Ra2-Ra1-Ra0
  set_tris_b (0b00000000);                                                     //Rb7-Rb6-Rb5-Rb4-Rb3-Rb2-Rb1-Rb0
  set_tris_c (0b10000000);                                                     //Rc7-Rc6-Rc5-Rc4-Rc3-Rc2-Rc1-Rc0
  set_tris_d (0b00000000); 
  
  setup_adc_ports(NO_ANALOGS);
  setup_adc(ADC_OFF);
  setup_wdt(WDT_OFF);
  setup_timer_0(RTCC_INTERNAL | RTCC_DIV_16 | RTCC_8_BIT);
  set_timer0(6);
  
  enable_interrupts(INT_TIMER0);
  enable_interrupts(INT_RDA);
  enable_interrupts(GLOBAL); 
  output_high(PIN_C5);
  delay_ms(2000);
  output_low(PIN_C5);
  delay_ms(5000);
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

  if(comando_disponivel_UART){
  
    disable_interrupts(GLOBAL);
    comando = Get_Comando();
    
    if(comando != 0){

      Executa_Comando(comando);
      
    }
    if(resposta_SIM == 1){
      resposta_SIM = 0;
      comando = Get_Comando();
      Executa_Comando(comando);
      delay_ms(100);
      envia_SIM800L("AT+CMGD=1,1\r\n", "OK");
    }
    comando = 0;
    comando_disponivel_UART = FALSE;
    enable_interrupts(GLOBAL);
    enable_interrupts(INT_RDA);

  }

}

void Executar_Cada_Minuto(){
  //Send_SMS("031995822739","TESTE");
}