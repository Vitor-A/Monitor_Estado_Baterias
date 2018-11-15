#IFNDEF TAI_BATERIA_H
#DEFINE TAI_BATERIA_H

//Protótipo de Funções

void Executar_Cada_Segundo();
void Executar_Cada_Minuto();
unsigned int16 obtem_mediana(unsigned int16 *num, int8 tam);
void Calcula_SOC();
void Obtem_SOH();
void le_EEPROM (void);

#ENDIF