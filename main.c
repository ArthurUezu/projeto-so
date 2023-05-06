#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

#define ATIVO 'a';
#define PRONTO 'p';
#define ESPERA 'e';
#define INATIVO 'i';

int pid = 0;

typedef struct BCP {
    int id;
    char nome[30];
    char estado;
    int tempoRestante;
    int linhaInstrucao;
    int posicaoMemoria;
    struct BCP * proximo;
} BCP;

BCP* criarProcesso(int tempo){
    BCP* processo = (BCP*) malloc(sizeof(BCP));
    processo->id = pid;
    processo->proximo = NULL;
    processo->linhaInstrucao = 0;
    processo->tempoRestante = tempo;
    pid++;
    return processo;
}

BCP* adicionarProcessoALista(BCP* lista, BCP* processo){
    BCP* cabecaLista = lista;
    if(lista == NULL){
        return processo;
    }
    if(lista->proximo == NULL){
        lista->proximo = processo;
        return lista;
    }
    BCP* proximo = lista->proximo;

    while(proximo != NULL && proximo->tempoRestante < processo->tempoRestante){
        proximo = proximo->proximo;
        lista = lista->proximo;
    }

    processo->proximo = proximo;
    lista->proximo = processo;
    return cabecaLista;
}

void printaLista(BCP* lista){
    printf("Id %d\n",lista->id);
    printf("Nome %d\n",lista->nome);
    printf("Tempo restante %d\n\n",lista->tempoRestante);

    while(lista->proximo != NULL){
        lista = lista->proximo;
        printf("Id %d\n",lista->id);
        printf("Nome %d\n",lista->nome);
        printf("Tempo restante %d\n\n",lista->tempoRestante);
    }
}

void semaforoP(int s, BCP* processo){
    if(s > 0){
        s--;
    }
}

void semaforoV(int s){

}

int teste(){
    BCP* lista = NULL;
    for(int i=0;i<100;i++){
        BCP* processo = criarProcesso(i);
        lista = adicionarProcessoALista(lista,processo);
    }
    for(int i=-100;i<0;i++){
        BCP* processo = criarProcesso(i);
        lista = adicionarProcessoALista(lista,processo);
    }
    printaLista(lista);
    lista = NULL;
    for(int i=0;i<100;i=i+2){

        BCP* processo = criarProcesso(i);

        lista = adicionarProcessoALista(lista,processo);

    }
    for(int i=1;i<100;i=i+2){

        BCP* processo = criarProcesso(i);

        lista = adicionarProcessoALista(lista,processo);

    }
    printaLista(lista);

}

void main(){
    teste();
    printf("\n");
}