char numeros[50];
int8 qtd_numeros=0;
char *posicao_numero;

int1 insere_numero(char *numero){ // >+5531995822739>+5531995822739>+5531995822739
 
  int8 index = qtd_numeros*15;
  int8 index_2 = 0;

  if(qtd_numeros == 3){

    return 0;

  }

  posicao_numero = strstr(numeros,numero);
  
  if(posicao_numero == 0){

    numeros[index] ='>';
    
    for(index = index+1;numero[index_2]!='\0';index++){
     
      numeros[index] = numero[index_2];
      index_2++;
       
    }
    numeros[index]='\0';
    qtd_numeros++;
    return 1;
  
  }
  return 0;
}


void limpa_numeros(){

  qtd_numeros = 0;
  memset (numeros, 0x00, sizeof(numeros));
  
}