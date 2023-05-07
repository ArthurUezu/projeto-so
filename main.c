#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <semaphore.h>

#define ATIVO 'a';
#define PRONTO 'p';
#define ESPERA 'e';
#define INATIVO 'i';

typedef struct BCP {
    int id;
    char nome[40];
    char estado;
    int tempoTotal;
    int tempoRestante;
    int linhaInstrucao;
    int posicaoMemoria;
    struct BCP* proximo;
} BCP;

int pid = 0;

BCP* bcp = NULL;

void adicionarProcessoAoBCP(BCP* processo){
    BCP* cabecaLista = bcp;
    if(bcp == NULL){
        bcp = processo;
        return;
    }
    if(bcp->proximo == NULL){
        bcp->proximo = processo;
        return;
    }
    BCP* proximo = bcp->proximo;

    while(proximo != NULL && proximo->tempoRestante < processo->tempoRestante){
        proximo = proximo->proximo;
        bcp = bcp->proximo;
    }

    processo->proximo = proximo;
    bcp->proximo = processo;
    bcp = cabecaLista;
}

void criarProcesso(char* fonte){
    char aux[40]="./";
    FILE* fontePrograma = fopen(strcat(aux,fonte),"r");
    BCP* processo = (BCP*) malloc(sizeof(BCP));
    processo->proximo = NULL;

    fscanf(fontePrograma,"%s\n",processo->nome);
    processo->id = pid;
    processo->estado = PRONTO;
    pid++;
    int tempo = 0;
    processo->tempoRestante = 0;
    char instrucao[10] = "";
    while(!feof(fontePrograma)){
        fscanf(fontePrograma,"%s",&instrucao);
        if(strcmp("exec",instrucao)==0){
            printf("\nexec\n");
        }
        else if(strcmp("write",instrucao)==0){
            printf("\nwrite\n");
        }
        else if(strcmp("read",instrucao)==0){
            printf("\nread\n");
        }
        fscanf(fontePrograma,"%d\n",&tempo);
        processo->tempoRestante += tempo;
    }
    adicionarProcessoAoBCP(processo);
    return;
}

void printaBCP(){
    if(bcp == NULL) return;
    printf("Id %d\n",bcp->id);
    printf("Nome %s\n",bcp->nome);
    printf("Tempo restante %d\n\n",bcp->tempoRestante);

    while(bcp->proximo != NULL){
        bcp = bcp->proximo;
        printf("Id %d\n",bcp->id);
        printf("Nome %d\n",bcp->nome);
        printf("Tempo restante %d\n\n",bcp->tempoRestante);
    }
}

void limparBCP(){
    BCP* aux = bcp;
    while(bcp->proximo != NULL){
        bcp = bcp->proximo;
        free(aux);
        aux = bcp;
    }
}


void semaforoP(int s, BCP* processo){
    if(s > 0){
        s--;
    }
}

void semaforoV(int s){

}



void main(int argc, char* argv[]){
    BCP* bcp = NULL;
    for(int i=1;i<argc;i++){
        criarProcesso(argv[i]);
    }
    printaBCP();
    limparBCP();
}