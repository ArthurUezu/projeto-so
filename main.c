#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

#define ATIVO 'a';
#define PRONTO 'p';
#define ESPERA 'e';
#define INATIVO 'i';

typedef struct BCP {
    int id;
    char nome[30];
    char estado;
    int tempoTotal;
    int tempoRestante;
    int linhaInstrucao;
    int posicaoMemoria;
} BCP;


void semaforoP(int s, BCP* processo){
    if(s > 0){
        s--;
    }
}

void semaforoV(int s){

}

void main(){

    printf("oi");
}