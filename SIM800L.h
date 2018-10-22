#IFNDEF SIM800L_H
#DEFINE SIM800L_H

int1 envia_SIM800L(char *send, char *recive);
int1 Send_SMS(char *numero,char *mensagem);
int1 Read_SMS();
int1 Get_Locate(char *numero);
void Get_Coordenadas();

#include "SIM800L.c"

#ENDIF