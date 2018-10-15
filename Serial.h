#IFNDEF SERIAL_H
#DEFINE SERIAL_H

#define  SMS_COMMAND             1
#define  SMS_READ_COMMAND        2           

void Executa_Comando(char comando);
char Get_Comando();
int32 get_value(char *pointer_to_string, int nro_caracteres,int posicao);
void get_numero(int nro_caracteres, int posicao, char *destino);
void clear_command();

#include"Serial.c"
#ENDIF