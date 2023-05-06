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
    int tempoRestante;
    int linhaInstrucao;
    int posicaoMemoria;
    struct BCP * proximo;
} BCP;

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

void main(){
    BCP* lista = NULL;
    BCP* itemLista = (BCP*) malloc(sizeof(BCP));
    itemLista->estado = ATIVO;
    itemLista->id = 1;
    itemLista->nome[0] = 'A';
    itemLista->tempoRestante = 10;
    itemLista->proximo = NULL;
    BCP* itemLista2 =(BCP*) malloc(sizeof(BCP));
    itemLista2->estado = ATIVO;
    itemLista2->id = 2;
    itemLista2->nome[0] = 'a';
    itemLista2->tempoRestante = 11;
    itemLista2->proximo = NULL;
    BCP* itemLista3 = malloc(sizeof(BCP));
    itemLista3->estado = ATIVO;
    itemLista3->id = 1;
    itemLista3->nome[0] = 'a';
    itemLista3->tempoRestante = 14;
    itemLista3->proximo = NULL;
    BCP* itemLista4 = malloc(sizeof(BCP));
    itemLista4->estado = ATIVO;
    itemLista4->id = 1;
    itemLista4->nome[0] = 'a';
    itemLista4->tempoRestante = 12;
    itemLista4->proximo = NULL;

    lista = adicionarProcessoALista(lista, itemLista);
    lista = adicionarProcessoALista(lista, itemLista2);
    lista = adicionarProcessoALista(lista, itemLista3);
    lista = adicionarProcessoALista(lista, itemLista4);
    printf("%d",itemLista->id);
    printaLista(lista);
    printf("\n");
}