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
    int tempoRestante;
    int linhaInstrucao;
    int posicaoMemoria;
    FILE* arquivoFonte;
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
    BCP* processo = (BCP*) malloc(sizeof(BCP));
    processo->arquivoFonte = fopen(strcat(aux,fonte),"r");
    processo->proximo = NULL;

    fscanf(processo->arquivoFonte,"%s\n",processo->nome);
    processo->id = pid;
    processo->estado = PRONTO;
    pid++;
    int tempo = 0;
    processo->tempoRestante = 0;
    char instrucao[40] = "";
    while(!feof(processo->arquivoFonte)){
        fscanf(processo->arquivoFonte,"%s",&instrucao);
        if(strcmp("exec",instrucao)==0){
            printf("\nexec\n");
        }
        else if(strcmp("write",instrucao)==0){
            printf("\nwrite\n");
        }
        else if(strcmp("read",instrucao)==0){
            printf("\nread\n");
        }
        fscanf(processo->arquivoFonte,"%d\n",&tempo);
        processo->tempoRestante += tempo;
    }
    adicionarProcessoAoBCP(processo);
    return;
}

void finalizarProcesso(int pid){
    BCP* cabecaLista = bcp;
    BCP* proximo = bcp;
    if(bcp->proximo != NULL) proximo = bcp->proximo;
    if(bcp->id == pid){
        fclose(bcp->arquivoFonte);
        free(bcp);
        bcp = proximo;
        return;
    } 
    while(proximo != NULL && proximo->id != pid){
        bcp = bcp->proximo;
        proximo = proximo->proximo;
    }

    if(proximo == NULL){
        bcp = cabecaLista;
        printf("\nProcesso não encontrado\n");
        return;
    }

    bcp->proximo = proximo->proximo;
    fclose(proximo->arquivoFonte);
    free(proximo);
    printf("\nProcesso finalizado\n");
}

void printaBCP(){
    if(bcp == NULL) return;
    printf("Id %d\n",bcp->id);
    printf("Nome %s\n",bcp->nome);
    printf("Tempo restante %d\n\n",bcp->tempoRestante);

    while(bcp->proximo != NULL){
        bcp = bcp->proximo;
        printf("Id %d\n",bcp->id);
        printf("Nome %s\n",bcp->nome);
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
    //TODO
    return;
}

void semaforoV(int s){
    //TODO
    return;
}

void interrupcaoProcesso(){
    //TODO
    return;
}

void executaProcesso(){
    //TODO
    return;
}

void memLoadReq(){
    //TODO
    return;
}

void memLoadFinish(){
    //TODO
    return;
}

void main(int argc, char* argv[]){
    BCP* bcp = NULL;
    for(int i=1;i<argc;i++){
        criarProcesso(argv[i]);
    }
    finalizarProcesso(3);
    printaBCP();
    limparBCP();
}