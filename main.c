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
#include <pthread.h>
#include <semaphore.h>

#define ATIVO 'a'
#define PRONTO 'p'
#define ESPERA 'e'
#define INATIVO 'i'

int memoria[125000]; //125000 posições, cada posição corresponde a 8kbytes
long espfree=125000;

int semaforos[255];
int menuAberto = 0;

sem_t mutexDisco;
sem_t mutexImpressao;
sem_t mutexProcessador;
sem_t print;

typedef struct requisicaoImpressao {
    int pid;
    int tempo;
    struct requisicaoImpressao* proximo;
} requisicaoImpressao;

typedef struct requisicaoDisco {
    int operacao;
    int pid;
    int trilha;
    struct requisicaoDisco* proximo;   
} requisicaoDisco;

typedef struct BCP {
    int id;
    char nome[40];
    char estado;
    int tempoRestante;
    int linhaInstrucao;
    int posicaoMemoria;
    int tamanho;
    int semaforoES;
    FILE* arquivoFonte;
    char semaforos[255];
    struct BCP* proximo;
} BCP;

requisicaoDisco* listaRequisicaoDisco = NULL;
requisicaoImpressao* listaRequisicaoImpressao = NULL;
BCP* bcp = NULL;

int pid = 0;
int finalizar = 1;

void printaImpressao();
void *gerenciamentoImpressao();
void novaRequisicaoImpressao(int pid, int tempo);
void finalizarOperacaoImpressao();
void printaDisco();
void *gerenciamentoDisco();
void novaRequisicaoDisco(int pid, int trilha, int operacao);
void finalizarOperacaoDisco(int pid, int tempo);
void adicionarProcessoAoBCP(BCP* processo);
void memLoadReq(BCP * processo);
void inicializaSemaforo(char identificador);
void semaforoV(char identificador, int pid);
void semaforoP(char identificador, int pid);
void memLoadFinish(BCP * processo);
void finalizarProcesso(int pid);
void interrupcaoProcesso();
void criarProcesso(char* fonte);
void limparBCP();
void executaProcesso();
int kbhit();
int menu();
void* ShortestRemainingTimeFirst();

void printaImpressao() {
    sem_wait(&print);
    printf("\n\n==========================\n\nFila de requisição de impressão:\n\n");
    if(listaRequisicaoImpressao == NULL){
        printf("Fila vazia\n");
        sem_post(&print);
        return;
    }
    requisicaoImpressao* cabeca = listaRequisicaoImpressao;
    printf("Id processo requisitor a imprimir: %d\n", listaRequisicaoImpressao->pid);
    printf("Tempo restante de impressão: %d\n\n", listaRequisicaoImpressao->tempo);
    while(listaRequisicaoImpressao->proximo != NULL){
        printf("Id processo requisitor: %d\n", listaRequisicaoImpressao->pid);
        printf("Tempo restante de impressão: %d\n\n", listaRequisicaoImpressao->tempo);
        listaRequisicaoImpressao = listaRequisicaoImpressao->proximo;
    }
    listaRequisicaoImpressao = cabeca;
    sem_post(&print);
}
void *gerenciamentoImpressao(){
    while(finalizar){
        requisicaoImpressao* requisicao = listaRequisicaoImpressao;
        sleep(2);
        while(menuAberto == 1);
        if(requisicao != NULL){
            sem_wait(&mutexImpressao);
            finalizarOperacaoImpressao();
            sem_post(&mutexImpressao);
        }
    }
    return NULL;
}

void novaRequisicaoImpressao(int pid, int tempo){
    requisicaoImpressao* novaRequisicao = (requisicaoImpressao*) malloc(sizeof(requisicaoImpressao));
    novaRequisicao->pid = pid;
    novaRequisicao->tempo = tempo;
    novaRequisicao->proximo = NULL;
    sem_wait(&mutexImpressao);
    if(listaRequisicaoImpressao == NULL){
        listaRequisicaoImpressao = novaRequisicao;
        sem_post(&mutexImpressao);
        return;
    }
    requisicaoImpressao* cabeca = listaRequisicaoImpressao;
    while(listaRequisicaoImpressao->proximo != NULL){
        listaRequisicaoImpressao = listaRequisicaoImpressao->proximo;
    }

    listaRequisicaoImpressao->proximo = novaRequisicao;
    listaRequisicaoImpressao = cabeca;
    sem_post(&mutexImpressao);
}

void finalizarOperacaoImpressao() {
    requisicaoImpressao* requisicao = listaRequisicaoImpressao;
    listaRequisicaoImpressao = listaRequisicaoImpressao->proximo;
    sem_wait(&mutexProcessador);
    BCP* cabeca = bcp; 
    while(bcp->id != pid && bcp->proximo != NULL){
        bcp = bcp->proximo;
    }
    bcp->tempoRestante -= requisicao->tempo;
    bcp = cabeca;
    sem_post(&mutexProcessador);
    if(bcp->tempoRestante <= 0){
        finalizarProcesso(bcp->id);
    }
    free(requisicao);
}


void printaDisco(){
    printf("\n\n==========================\n\nFila de requisição de disco:\n\n");
    if(listaRequisicaoDisco == NULL){
        printf("Fila vazia\n");
        return;
    }
    requisicaoDisco* cabeca = listaRequisicaoDisco;
    printf("Id processo requisitor lendo disco: %d\n", cabeca->pid);
    printf("Trilha sendo lida: %d\n", cabeca->trilha);
    if(cabeca->trilha){
        printf("Operação: read\n\n");
    }
    else {
        printf("Operação: write\n\n");
    }
    
    while(cabeca->proximo != NULL){
        printf("Id processo requisitor: %d\n", cabeca->pid);
        printf("Trilha a ser lida: %d\n", cabeca->trilha);
        if(cabeca->trilha){
        printf("Operação: read\n\n");
    }
    else {
        printf("Operação: write\n\n");
    }
        cabeca = cabeca->proximo;
    }
}

void *gerenciamentoDisco(){ // CONSUMER e PRODUCER
    while(finalizar){
        requisicaoDisco* requisicao = listaRequisicaoDisco;
        sleep(2);
        while(menuAberto == 1);
        if(requisicao != NULL){
            sem_wait(&mutexDisco);
            listaRequisicaoDisco = listaRequisicaoDisco->proximo;
            finalizarOperacaoDisco(requisicao->pid, requisicao->trilha);
            free(requisicao);
            sem_post(&mutexDisco);
        }
    }
    return NULL;
}

void novaRequisicaoDisco(int pid, int trilha, int operacao){
    requisicaoDisco* novaRequisicao = (requisicaoDisco*) malloc(sizeof(requisicaoDisco));
    novaRequisicao->pid = pid;
    novaRequisicao->trilha = trilha;
    novaRequisicao->operacao = operacao;
    novaRequisicao->proximo = NULL;
    sem_wait(&mutexDisco);
    if(listaRequisicaoDisco == NULL){
        listaRequisicaoDisco = novaRequisicao;
        sem_post(&mutexDisco);
        return;
    }
    requisicaoDisco* cabeca = listaRequisicaoDisco;
    while(listaRequisicaoDisco->proximo != NULL){
        listaRequisicaoDisco = listaRequisicaoDisco->proximo;
    }

    listaRequisicaoDisco->proximo = novaRequisicao;
    listaRequisicaoDisco = cabeca;
    sem_post(&mutexDisco);
}

void finalizarOperacaoDisco(int pid, int tempo){
    sem_wait(&mutexProcessador);
    BCP* cabeca = bcp; 
    while(cabeca->id != pid && cabeca->proximo != NULL){
        cabeca = cabeca->proximo;
    }
    cabeca->tempoRestante -= tempo;
    cabeca->semaforoES = 1;
    cabeca = cabeca;
    sem_post(&mutexProcessador);
    if(cabeca->tempoRestante <= 0){
        finalizarProcesso(cabeca->id);
    }
    interrupcaoProcesso();
}

void trataconcorrencia( ){

}

void printaBCP(){
    if(bcp == NULL) return;
    sem_wait(&print);
    BCP* cabeca = bcp;
    printf("\n\n==========================\n\nBCP:\n");
    printf("Id %d\n",cabeca->id);
    printf("Nome %s\n",cabeca->nome);
    printf("Estado %c\n",cabeca->estado);
    printf("Tempo restante %d\n",cabeca->tempoRestante);
    printf("Linha de instrução %d\n",cabeca->linhaInstrucao);
    printf("Memória ocupada %dkb\n\n", cabeca->tamanho);
    printf("Estado do semaforo de ES %d\n\n", cabeca->semaforoES);

    while(cabeca->proximo != NULL){
        cabeca = cabeca->proximo;
        printf("Id %d\n",cabeca->id);
        printf("Nome %s\n",cabeca->nome);
        printf("Estado %c\n",cabeca->estado);
        printf("Tempo restante %d\n",cabeca->tempoRestante);
        printf("Linha de instrução %d\n",cabeca->linhaInstrucao);
        printf("Memória ocupada %dkb\n\n", cabeca->tamanho);
        printf("Estado do semaforo de ES %d\n\n", cabeca->semaforoES);
    }
    sem_post(&print);
}



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
    sem_wait(&mutexProcessador);
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
        sem_post(&mutexProcessador);
        return;
    }
    while(proximo != NULL && proximo->id != pid){
        bcp = bcp->proximo;
        proximo = proximo->proximo;
    }

    if(proximo == NULL){
        bcp = cabecaLista;
        printf("\nProcesso não encontrado\n");
        sem_post(&mutexProcessador);
        return;
    }

    bcp->proximo = proximo->proximo;
    bcp->estado = ATIVO;
    fclose(proximo->arquivoFonte);
    free(proximo);
    memLoadFinish(proximo);
    sem_post(&mutexProcessador);
    printf("\nProcesso finalizado\n");
}

void interrupcaoProcesso(){
    printf("\nInterrompendo");
    if(bcp == NULL){
        return;
    }

    if(bcp->proximo == NULL){
        while(bcp->semaforoES == 0);
        return;
    }

    if(bcp->semaforoES == 0){
        BCP* proximo = bcp->proximo;
        BCP* aux = bcp;
        if(proximo->semaforoES == 0 && proximo->proximo == NULL){
            if(listaRequisicaoDisco->pid == bcp->id){
                while(bcp->semaforoES == 0);
                return; 
            } else {
                bcp->estado = ESPERA;
                bcp->proximo = proximo->proximo;
                proximo->proximo = bcp;
                proximo->estado = ATIVO;
                bcp = proximo;
                while(bcp->semaforoES == 0);
                return;
            }
        }
        while(proximo->proximo != NULL && proximo->semaforoES == 0){
            proximo = proximo->proximo;
            aux = aux->proximo;
        }
        if(proximo->semaforoES == 1){
            aux->estado = ESPERA;
            aux->proximo = proximo->proximo;
            proximo->proximo = bcp;
            proximo->estado = ATIVO;
            bcp = proximo;
            return;
        }
        if(proximo->proximo == NULL){
            printf("\nTodos os processos estão aguardando dados\n");
            while(bcp->semaforoES == 0);
            return;
        }
    }

    if(bcp->tempoRestante == 0){
        printf("\nExecução do processo chegou ao fim.\n");
        finalizarProcesso(bcp->id);
        return;
    }

    for(int i=0;i<strlen(bcp->semaforos);i++){
        if(bcp->semaforos[i] == '\0'){
            continue;
        }
        if(semaforos[bcp->semaforos[i]] == 0){
            return;
        }
    }
    
    BCP* proximo = bcp->proximo;
    bcp->estado = ESPERA;
    bcp->proximo = proximo->proximo;
    proximo->proximo = bcp;
    proximo->estado = ATIVO;
    bcp = proximo;
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
    processo->semaforoES = 1;
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

void limparFilaImp(){
    requisicaoImpressao* aux = listaRequisicaoImpressao;
    while(listaRequisicaoImpressao->proximo != NULL){
        listaRequisicaoImpressao = listaRequisicaoImpressao->proximo;
        free(aux);
        aux = listaRequisicaoImpressao;
    }
}

void limparFilaDisc(){
    requisicaoDisco* aux = listaRequisicaoDisco;
    while(listaRequisicaoDisco->proximo != NULL){
        listaRequisicaoDisco = listaRequisicaoDisco->proximo;
        free(aux);
        aux = listaRequisicaoDisco;
    }
}

void executaProcesso(){
    BCP* processo = bcp;
    if(processo == NULL) {
        printf("\nBCP vazio, para adicionar processos, abra o menu\n");
        return;
    }
    if(processo->tempoRestante <= 0){
        finalizarProcesso(processo->id);
    }
    sem_wait(&print);
    printf("Executando processo com PID: %d", processo->id);
    if(feof(processo->arquivoFonte)){
        sem_post(&print);
    printf("Executando processo com PID: %d", processo->id);
        interrupcaoProcesso();
        finalizarProcesso(processo->id);
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
    printf("\nInstrução: %s\nTempo de execução: %d\nMemória disponível: %ldmb\n",instrucao,tempo,espfree*8/1000);
    if(strcmp(instrucao,"read") == 0 || strcmp(instrucao,"write") == 0){
        processo->semaforoES = 0;
        if(strcmp(instrucao, "read")){
            novaRequisicaoDisco(processo->id, tempo,1);
        }
        else {
            novaRequisicaoDisco(processo->id, tempo,0);
        }
        interrupcaoProcesso();
    } else if (strcmp(instrucao,"print") == 0){
        novaRequisicaoImpressao(processo->id, tempo);
    }
    else{
        processo->tempoRestante -= tempo;
    }

    sem_post(&print);
    if(processo->tempoRestante <= 0 ){
        interrupcaoProcesso();
    }
}

int kbhit(){
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
    printf("\nAperte ENTER para abrir o menu\n");
    if(kbhit()){
        menuAberto = 1;
        printf("\n\nDigite a opção!\n");
        printf("0) Voltar a execução\n");
        printf("1) Adicionar processo\n");
        printf("2) Matar processo\n");
        printf("3) Sair\nDigite: ");
        scanf("%d",&escolha);
    }
    switch(escolha){
        case 0: break;
        case 1:{
            char arquivo[50] = "";
            printf("\n\nDigite o PATH ('./nomearquivo')\nDigite: ");
            scanf("%s",arquivo);
            criarProcesso(arquivo);
            break;
        }
        case 2:{
            printf("\n\nDigite o id do processo que deseja matar\nDigite: ");
            scanf("%d",&escolha);
            finalizarProcesso(escolha);
            break;
        }
        case 3:{
            limparBCP();
            if(bcp != NULL)
                finalizarProcesso(bcp->id);
            return 0;
            break;
        }
    }
    menuAberto = 0;
    return 1;
}

void* ShortestRemainingTimeFirst(){
    while(finalizar){
        if(menuAberto == 0){
            executaProcesso();
        }
        sleep(2);
    }
}

int main(int argc, char* argv[]){
    sem_init(&mutexProcessador, 0, 1);
    sem_init(&mutexDisco, 0, 1);
    sem_init(&mutexImpressao, 0, 1);
    sem_init(&print, 0, 1);
    pthread_t disco, processador, impressao;
    int i=0;
    for(i=0;i<255;i++){
        semaforos[i] = -100;
    }
    for(i=1;i<argc;i++){
        criarProcesso(argv[i]);
    }
    pthread_create(&disco, NULL, gerenciamentoDisco, NULL);
    pthread_create(&processador, NULL, ShortestRemainingTimeFirst, NULL);
    pthread_create(&impressao, NULL, gerenciamentoImpressao, NULL);
    while(finalizar){
        system("clear");
        if(menuAberto == 0){
            printaDisco();
            printaImpressao();
            printaBCP();
        }
        finalizar = menu();
        sleep(1);
    }
    limparFilaDisc();
    limparFilaImp();
    pthread_join(disco,NULL);
    pthread_join(processador,NULL);
    pthread_join(impressao,NULL);

    sem_destroy(&mutexProcessador);
    sem_destroy(&mutexDisco);
    sem_destroy(&mutexImpressao);
    sem_destroy(&print);
    return 0;
}