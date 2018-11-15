#IFNDEF MEMORY_H
#DEFINE MEMORY_H

//*********************************** ENDERECOS EEPROM ******************************************* 

#define ADDR_tempo_entre_alertas              getenv("EEPROM_ADDRESS") + 0  //int32
#define ADDR_tempo_ultimo_alerta              getenv("EEPROM_ADDRESS") + 4  //int32
#define ADDR_corrente_limite                  getenv("EEPROM_ADDRESS") + 8  //int32
#define ADDR_zero_set                         getenv("EEPROM_ADDRESS") + 12 //int32
#define ADDR_qtd_numeros                      getenv("EEPROM_ADDRESS") + 16 //int8
#define ADDR_vector_numeros                   getenv("EEPROM_ADDRESS") + 17 //Vetor int 8 com 50 posicoes

//*************************************************************************************************

//************************************ DEAFULT EEPROM *********************************************

#rom int32 ADDR_tempo_entre_alertas = {0}
#rom int32 ADDR_tempo_ultimo_alerta = {0} 
#rom int32 ADDR_corrente_limite = {0} 
#rom int32 ADDR_zero_set = {0} 
#rom int8  ADDR_qtd_numeros = {0} 
#rom int8  ADDR_vector_numeros = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}

//*************************************************************************************************


void read_config(unsigned int32 address, unsigned int8 *dataptr, unsigned int8 count);
void write_config(unsigned int32 address, unsigned int8 *dataptr, unsigned int8 count);

#include "Memory.c"
#ENDIF