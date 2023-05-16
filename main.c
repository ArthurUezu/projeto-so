#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <semaphore.h>

#define ATIVO 'a'
#define PRONTO 'p'
#define ESPERA 'e'
#define INATIVO 'i'

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

void interrupcaoProcesso(){
    if(bcp->tempoRestante == 0){
        printf("\nExecução do processo chegou ao fim.\n");
        finalizarProcesso(bcp->id);
        return;
    }
    if(bcp->proximo != NULL){
        BCP* proximo = bcp->proximo;
        bcp->estado = ESPERA;
        bcp->proximo = proximo->proximo;
        proximo->proximo = bcp;

        proximo->estado = ATIVO;
        bcp = proximo;
    }

    return;
}

void adicionarProcessoAoBCP(BCP* processo){
    BCP* cabecaLista = bcp;
    if(bcp == NULL){
        bcp = processo;
        return;
    }
    if(bcp->proximo == NULL){
            bcp->proximo = processo;
        if(bcp->tempoRestante > processo->tempoRestante){
            interrupcaoProcesso();
        }
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
    processo->tempoRestante = 0;
    int tempo = 0;
    char instrucao[40] = "";
    while(!feof(processo->arquivoFonte)){
        fscanf(processo->arquivoFonte,"%s",&instrucao);
        fscanf(processo->arquivoFonte,"%d\n",&tempo);
        processo->tempoRestante += tempo;
    }
    fseek(processo->arquivoFonte, 0, SEEK_SET);
    fscanf(processo->arquivoFonte,"%s\n",&instrucao);
    adicionarProcessoAoBCP(processo);
    return;
}

void finalizarProcesso(int pid){
    BCP* cabecaLista = bcp;
    BCP* proximo = bcp;
    if(bcp->proximo != NULL) proximo = bcp->proximo;
    if(bcp->id == pid){
        if(proximo == bcp){
            fclose(bcp->arquivoFonte);
            free(bcp);
            bcp = NULL;
        }
        else {
            fclose(bcp->arquivoFonte);
            free(bcp);
            bcp = proximo;
        }
        if(bcp != NULL) bcp->estado = ATIVO;
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
    bcp->estado = ATIVO;
    fclose(proximo->arquivoFonte);
    free(proximo);
    printf("\nProcesso finalizado\n");
}

void printaBCP(){
    if(bcp == NULL) return;
    BCP* cabeca = bcp;
    printf("\n\nBCP:\n");
    printf("Id %d\n",bcp->id);
    printf("Nome %s\n",bcp->nome);
    printf("Estado %c\n",bcp->estado);
    printf("Tempo restante %d\n\n",bcp->tempoRestante);

    while(bcp->proximo != NULL){
        bcp = bcp->proximo;
        printf("Id %d\n",bcp->id);
        printf("Nome %s\n",bcp->nome);
        printf("Estado %c\n",bcp->estado);
        printf("Tempo restante %d\n\n",bcp->tempoRestante);
    }
    bcp = cabeca;
}

void limparBCP(){
    BCP* aux = bcp;
    while(bcp->proximo != NULL){
        bcp = bcp->proximo;
        free(aux);
        aux = bcp;
    }
}


void semaforoP(int s){
    //TODO
    return;
}

void semaforoV(int s){
    //TODO
    return;
}




void executaProcesso(){
    BCP* processo = bcp;
    printf("Executando processo com PID: %d", processo->id);
    if(feof(processo->arquivoFonte)){
        interrupcaoProcesso();
        return;
    }
    if(processo->estado != ATIVO){
        processo->estado = ATIVO;
    }
    char instrucao[40] = "";
    int tempo = 0;
    fscanf(processo->arquivoFonte,"%s",&instrucao);
    if(instrucao == "P" || instrucao == "V"){
        //SEMAFORO
        tempo = 200;
    }
    else {
        fscanf(processo->arquivoFonte,"%d\n",&tempo);
    }
    processo->linhaInstrucao++;
    processo->tempoRestante -= tempo;
    printf("\nInstrução: %s\nTempo de execução: %d\n",instrucao,tempo);
    if(processo->tempoRestante <= 0 || instrucao != "exec"){
        interrupcaoProcesso();
    }
}

void memLoadReq(){
    //TODO
    return;
}

void memLoadFinish(){
    //TODO
    return;
}

void menu(){
    int escolha = -1;
    printaBCP();
    while(escolha == -1){
        printf("\n\nDigite a opção!\n");
        printf("0) Executar até o fim\n");
        printf("1) Executar\n");
        printf("2) Adicionar processo\n");
        printf("3) Interromper processo\n");
        printf("4) Matar processo\n");
        printf("5) Finalizar\nDigite: ");
        scanf("%d",&escolha);
    }
    switch(escolha){
        case 0:
            while(bcp!=NULL){
                executaProcesso();
            }
            break;
        case 1:
            executaProcesso();
            break;
        case 2:
            char arquivo[50] = "";
            printf("\n\nDigite o PATH ('./nomearquivo')\nDigite: ");
            scanf("%s",arquivo);
            criarProcesso(arquivo);
            break;
        case 3:
            interrupcaoProcesso();
            break;
        case 4:
            printf("\n\nDigite o id do processo que deseja matar\nDigite: ");
            scanf("%d",&escolha);
            finalizarProcesso(escolha);
            break;
        case 5:
            limparBCP();
            break;
            

    }
}
void ShortestRemainingTimeFirst(){
    while(1){
        menu();
        if(bcp == NULL) return;
    }
}

void main(int argc, char* argv[]){
    for(int i=1;i<argc;i++){
        criarProcesso(argv[i]);
    }
    ShortestRemainingTimeFirst();
    // executaProcesso();
    // limparBCP();
}