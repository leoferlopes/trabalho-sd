/*
 * Problema do Barbeiro Dorminhoco
 * 
 * Universidade Federal Fluminense
 * Departamento de Ciência da Computação
 * Sistemas Distribuídos - 2016.1
 * 
 * Alunos: Bernardo Lopes, Leonardo Lopes e Romulo Martins
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mpi.h"

#define ID_COORDENADOR 0;
#define ID_BARBEIRO 1;

#define CADEIRAS 10;

#define TAG_RESPOSTA 200;
#define TAG_PEDIDO 201;

#define TRUE 1;
#define FALSE 0;

int tagResposta = TAG_RESPOSTA;
int tagPedido = TAG_PEDIDO;
int idCoordenador = ID_COORDENADOR;
int idBarbeiro = ID_BARBEIRO;
int cadeiras = CADEIRAS;

int barbeiroLivre = FALSE;

//Variáveis relaciondas às cadeiras da barbearia
int* cadeirasOcupadas;
int qtdCadeirasOcupadas = 0; //Contador de cadeiras ocupadas
int proxCliente = -1; //Posição do próximo cliente no array
int proxCadeiraVazia = 0; //Próxima cadeira vazia no array

int rank, size;

//---------- Mensagens exibidas durante a execução do programa ----------

void cortando_cabelo() { 
  // ou seja o barbeiro está ocupado
  printf("O Barbeiro esta cortando o cabelo de alguem!\n");
}

void cabelo_cortado(int processo) {
    printf("BARBEIRO: Cabelo do cliente %d cortado!\n", processo);
}

void cliente_chegou(int processo) {
  printf("Cliente %d chegou para cortar cabelo!\n", processo);
}

void cliente_sentou(int processo, int cadeira) {
    printf("COORD: Cliente %d se sentou na cadeira %d e pegou uma Caras de %d enquanto espera.\n", processo, cadeira, rand()%40 + 1970);
}

// void cliente_proximo(int processo) {
//     printf("COORD: Cliente %d chamado para sentar na cadeira do barbeiro.\n", processo);
// }

void atendendo_cliente(int processo) {
  printf("BARBEIRO: Cliente %d esta tendo o cabelo cortado!\n", processo);
}

// void cliente_satisfeito(int processo){
//     printf("CLIENTE: Cliente %d pagou R$ %d,00 e foi embora satisfeito!\n", processo, rand()%100 + 1);
// }

void cliente_desiste(int processo) {
  printf("CLIENTE: Cliente %d desistiu! (O salao estah muito cheio!)\n", processo);
}

void mandaOrdemParaOBarbeiro() {
    if (barbeiroLivre && qtdCadeirasOcupadas > 0) {
        int processo = cadeirasOcupadas[proxCliente]; /* pega o primeiro elemento da fila */
        if (++proxCliente == cadeiras){ //Fazer o loop no contador se der "overflow"
            proxCliente = 0;
        }
        // cliente_proximo(processo);
        qtdCadeirasOcupadas--;
        // printf("DEBUG: Próximo cliente está na cadeira: %d\nDEBUG: %d cadeiras ocupadas\n", proxCliente, qtdCadeirasOcupadas);
        MPI_Send(&processo, 1, MPI_INT, idBarbeiro, tagPedido, MPI_COMM_WORLD);
        barbeiroLivre = FALSE;
    }
}

void coordenador() {
    int processo;

    cadeirasOcupadas = (int*) malloc(cadeiras * sizeof (int));
   
    while (1) {
        MPI_Recv(&processo, 1, MPI_INT, MPI_ANY_SOURCE, tagPedido, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // testa se a mensagem é do barbeiro
        if (processo == idBarbeiro) {
            barbeiroLivre = TRUE;
            mandaOrdemParaOBarbeiro();
        } else if (qtdCadeirasOcupadas < cadeiras) { // Testa se tem espaço na fila
            cadeirasOcupadas[proxCadeiraVazia] = processo;
            qtdCadeirasOcupadas++; //Ocupa uma cadeira
            cliente_sentou(processo, proxCadeiraVazia + 1);
            if (qtdCadeirasOcupadas == 1) { //Se este processo for o único na fila, definir cliente atual como próximo
                proxCliente = proxCadeiraVazia;
            }
            // printf("DEBUG: Próximo cliente está na cadeira: %d\nDEBUG: %d cadeiras ocupadas\n", proxCliente, qtdCadeirasOcupadas);
            if (++proxCadeiraVazia == cadeiras) { //Fazer o loop no contador se der "overflow"
                proxCadeiraVazia = 0;
            }
            mandaOrdemParaOBarbeiro();
        } else {
            int message = FALSE;
            MPI_Send(&message, 1, MPI_INT, processo, tagResposta, MPI_COMM_WORLD);
        }
    }
    free(cadeirasOcupadas);
}

void cortarCabelo(int processo) {
    atendendo_cliente(processo);
    //la la ri la la la
    cabelo_cortado(processo);
}

void barbeiro() {
    while (1) {
        // avisa o coordenador que está livre
        MPI_Send(&rank, 1, MPI_INT, idCoordenador, tagPedido, MPI_COMM_WORLD);

        // recebe ordem de serviço
        int processo;
        MPI_Recv(&processo, 1, MPI_INT, MPI_ANY_SOURCE, tagPedido, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // corta o cabelo do cliente
        cortarCabelo(processo);
        // envia um ok para o cliente
        int message = TRUE;
        MPI_Send(&message, 1, MPI_INT, processo, tagResposta, MPI_COMM_WORLD);
    }
}

void cliente() {
    usleep(500000);
    MPI_Send(&rank, 1, MPI_INT, idCoordenador, tagPedido, MPI_COMM_WORLD);

    int resposta;
    MPI_Recv(&resposta, 1, MPI_INT, MPI_ANY_SOURCE, tagResposta, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    if (!resposta) {
        cliente_desiste(rank);
    }
}


//---------- Função principal --------------------------
int main(int argc, char *argv[]) {

    /* Inicia o MPI */

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    switch (rank) {
        case 0:
            coordenador();
            break;
        case 1:
            barbeiro();
            break;
        default:
            cliente();
            break;
    }

    MPI_Finalize();
    return 0;
}