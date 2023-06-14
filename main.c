/*
    Feito por Miguel Donizeti da Silva e Silva
              Arthur Borsonaro Uezu
              Luis Felipe Furilli


*/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <sys/select.h>
#define ATIVO 'a'
#define PRONTO 'p'
#define ESPERA 'e'
#define INATIVO 'i'

int memoria[125000]; //125000 posições, cada posição corresponde a 8kbytes
long espfree=125000;

int semaforos[255];

typedef struct BCP {
    int id;
    char nome[40];
    char estado;
    int tempoRestante;
    int linhaInstrucao;
    int posicaoMemoria;
    int tamanho;
    FILE* arquivoFonte;
    char semaforos[255];
    struct BCP* proximo;
} BCP;

BCP* bcp = NULL;

void adicionarProcessoAoBCP(BCP* processo);

void printaBCP(){
    if(bcp == NULL) return;
    BCP* cabeca = bcp;
    printf("\n\nBCP:\n");
    printf("Id %d\n",bcp->id);
    printf("Nome %s\n",bcp->nome);
    printf("Estado %c\n",bcp->estado);
    printf("Tempo restante %d\n",bcp->tempoRestante);
    printf("Linha de instrução %d\n",bcp->linhaInstrucao);
    printf("Memória ocupada %dkb\n\n", bcp->tamanho);

    while(bcp->proximo != NULL){
        bcp = bcp->proximo;
        printf("Id %d\n",bcp->id);
        printf("Nome %s\n",bcp->nome);
        printf("Estado %c\n",bcp->estado);
        printf("Tempo restante %d\n",bcp->tempoRestante);
        printf("Linha de instrução %d\n",bcp->linhaInstrucao);
        printf("Memória ocupada %dkb\n\n", bcp->tamanho);
    }
    bcp = cabeca;
}

int pid = 0;

void memLoadReq(BCP * processo){
    BCP * process = processo;
    int pid = process->id;
    int tamanho = (int)((process->tamanho)/8);
    printf("tamanho do programa %d",tamanho);
    for (int i=1;i<125000;i++){
        if (memoria[i]==0){
            if(tamanho!=0){
                memoria[i]=pid;
                tamanho--;
                espfree=espfree-1;
            }
            
        }
    }
}

void inicializaSemaforo(char identificador){
    if(semaforos[identificador] == -100){
        semaforos[identificador] = 1;
    }
}

void semaforoV(char identificador, int pid){ // post ou signal
    semaforos[identificador]++;
}

void semaforoP(char identificador, int pid){ // wait
    semaforos[identificador]--;
}

void memLoadFinish(BCP * processo){
    BCP  * process = processo;
    int nblocos = (int)((process->tamanho)/8);
    int i=0;
    espfree=espfree+nblocos;
    for (int i=1;i<125000;i++){
        if (process->id==memoria[i]){
            memoria[i]=0;
            nblocos--;
        }
        else if (nblocos==0){
            break;
        }
    }
    
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
    memLoadFinish(proximo);
    printf("\nProcesso finalizado\n");
}

void interrupcaoProcesso(){
    for(int i=0;i<strlen(bcp->semaforos);i++){
        if(bcp->semaforos[i] == '\0'){
            continue;
        }
        if(semaforos[bcp->semaforos[i]] == 0){
            return;
        }

    }
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

    if(bcp->tempoRestante > processo->tempoRestante){
        processo->proximo = bcp->proximo;
        bcp->proximo = processo;
        interrupcaoProcesso();
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
    fscanf(processo->arquivoFonte,"%d\n",&processo->id);
    fscanf(processo->arquivoFonte,"%d\n",&processo->tamanho);
    fscanf(processo->arquivoFonte,"%[^\n]\n",processo->semaforos);
    for(int i = 0; i<strlen(processo->semaforos);i++){
        inicializaSemaforo(processo->semaforos[i]);
    }
    processo->estado = PRONTO;
    pid++;
    processo->tempoRestante = 0;
    int tempo = 0;
    char instrucao[40] = "";
    while(!feof(processo->arquivoFonte)){
        fscanf(processo->arquivoFonte,"%s",instrucao);
        if(instrucao[0]=='P' || instrucao[0]=='V'){
            tempo=200;
        }
        else{
            fscanf(processo->arquivoFonte,"%d\n",&tempo);
        }
        processo->tempoRestante += tempo;
    }
    fseek(processo->arquivoFonte, 0, SEEK_SET);
    fscanf(processo->arquivoFonte,"%s\n",instrucao);
    fscanf(processo->arquivoFonte,"%d\n",&processo->id);
    fscanf(processo->arquivoFonte,"%d\n",&processo->tamanho);
    fscanf(processo->arquivoFonte,"%[^\n]\n",processo->semaforos);
    adicionarProcessoAoBCP(processo);
    memLoadReq(processo);

    return;
}




void limparBCP(){
    BCP* aux = bcp;
    while(bcp->proximo != NULL){
        bcp = bcp->proximo;
        free(aux);
        aux = bcp;
    }
}

void executaProcesso(){
    BCP* processo = bcp;
    if(processo == NULL) {
        printf("\nBCP vazio, para adicionar processos, abra o menu\n");
        return;
    }
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
    fscanf(processo->arquivoFonte,"%s",instrucao);
    if(instrucao[0]=='P' || instrucao[0]=='V'){
        if(instrucao[0]=='P'){
            semaforoP(instrucao[2],processo->id);
        }
        else{
            semaforoV(instrucao[2],processo->id);
        }

        tempo = 200;

    } else {
        fscanf(processo->arquivoFonte,"%d\n",&tempo);
    }
    processo->linhaInstrucao++;
    processo->tempoRestante -= tempo;
    printf("\nInstrução: %s\nTempo de execução: %d\nMemória disponível: %ldmb\n",instrucao,tempo,espfree*8/1000);

    if(processo->tempoRestante <= 0 ){
        interrupcaoProcesso();
    }
}

int kbhit()
{
    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds); //STDIN_FILENO is 0
    select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
    return FD_ISSET(STDIN_FILENO, &fds);
}

int menu(){
    int escolha = -1;
    printaBCP(bcp);
    printf("\nAperte ENTER para abrir o menu\n");
    if(kbhit()){
        printf("\n\nDigite a opção!\n");
        printf("1) Adicionar processo\n");
        printf("2) Matar processo\n");
        printf("3) Sair\nDigite: ");
    
        scanf("%d",&escolha);
    }
    switch(escolha){
        
        case 1:
            char arquivo[50] = "";
            printf("\n\nDigite o PATH ('./nomearquivo')\nDigite: ");
            scanf("%s",arquivo);
            criarProcesso(arquivo);
            break;
        case 2:
            printf("\n\nDigite o id do processo que deseja matar\nDigite: ");
            scanf("%d",&escolha);
            finalizarProcesso(escolha);
            break;
        case 3:
            limparBCP();
            if(bcp != NULL)
                finalizarProcesso(bcp->id);
            return 0;
            break;
        

    }
    return 1;
}


void ShortestRemainingTimeFirst(){
    int finalizar = 1;
    while(finalizar){
        system("clear");
        executaProcesso();
        finalizar = menu();
        sleep(1);
    }
}

int main(int argc, char* argv[]){
    int i=0;
    for(i=0;i<255;i++){
        semaforos[i] = -100;
    }
    for(i=1;i<argc;i++){
        criarProcesso(argv[i]);
    }
    ShortestRemainingTimeFirst();
    return 0;
}