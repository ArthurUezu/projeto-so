# TO DO

- Semáforos
- Gerenciamento de memória
## Gerenciamento de memória
  - Pode ser feita por representação num

## Especificações do lele

Na primeira parte do projeto devem ser implementadas a interface de controle do sistema e as funções indicadas a seguir:

    processInterrupt (1) -- interrupção gerada pelo final do quantum-time de um processo
    semaphoreP (10) -- tratamento de bloqueio de processo
    semaphoreV (11) -- tratamento de desbloqueio de processo
    memLoadReq (6) -- chamada de operação de carregamento na memória
    memLoadFinish (7) -- sinalização de final de carregamento
    processCreate (2) -- chamada para iniciar a criação de um processo no BCP *
    processFinish (3) -- chamada para terminar a existência de um processo no BCP * 

* BCP (Bloco de Controle de Processos) é a estrutura que armazena os dados dos processos em execução 
 
Algoritmo de Escalonamento

Deve ser implementado o algoritmo SRTF (Shortest Remaining Time First), sendo que para a determinação do tempo de execução se deve somar todos os valores de exec, read, write e print existentes no código do programa sintético a ser executado.
O processo que estiver em execução será interrompido sempre que solicitar algo diferente de exec, ou o usuário solicitar a execução de um novo processo (processCreate).
Chamadas para P(s) e V(s), quando não gerarem bloqueio do processo, contabilizarão 200 unidades de tempo e manterão o processo que as executou na CPU, exceto se a operação seguinte for de entrada/saída.
 
Gerenciamento de memória

    O espaço total da memória é de 1 Gbyte (já descontado o espaço ocupado pelo SO);
    Deve ser simulada uma memória com alocação em blocos, com páginas de 8 kbytes;
    A ocupação da memória deve ser gerenciada através de uma estrutura baseada em segmentos;
    O mecanismo de memória virtual deve ser implementado usando o algoritmo da segunda chance. 

 
Interface de controle

A interface de controle deverá permitir que sejam submetidos novos processos, visualizados dados sobre o estado dos processos em execução e sobre o estado de ocupação da memória.
 
Deve ser observado que as visualizações devem ser solicitadas explicitamente, ficando normalmente visível apenas informações sobre o processo em execução. 
