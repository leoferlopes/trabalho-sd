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
#include "mpi.h"

#define ID_COORDENADOR 0;
#define ID_BARBEIRO 1;

#define CADEIRAS 6;

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
int* cadeirasOcupadas = malloc(cadeiras * sizeof(int)); //O array em si
int qtdCadeirasOcupadas = 0; //Contador de cadeiras ocupadas
int proxCliente = -1; //Posição do próximo cliente no array
int proxCadeiraVazia = 0; //Próxima cadeira vazia no array

int rank, size;

void mandaOrdemParaOBarbeiro() {
    if (barbeiroLivre && qtdCadeirasOcupadas > 0) {
        int processo = cadeirasOcupadas[proxCliente]; /* pega o primeiro elemento da fila */
        if (++proxCliente == cadeiras){ //Fazer o loop no contador se der "overflow"
            proxCliente == 0;
        }
        MPI_Send(&processo, 1, MPI_INT, idBarbeiro, tagPedido, MPI_COMM_WORLD);
        qtdCadeirasOcupadas--;
        barbeiroLivre = FALSE;
    }
}

void coordenador() {
    int processo;

    int* cadeirasOcupadas = malloc(cadeiras * sizeof (int));
    int qtdCadeirasOcupadas = 0;
    int proxCliente = -1;
    int proxCadeiraVazia = 0;
   
    while (1) {
        MPI_Recv(&processo, 1, MPI_INT, MPI_ANY_SOURCE, tagPedido, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // testa se a mensagem é do barbeiro
        if (processo == idBarbeiro) {
            barbeiroLivre = TRUE;
            mandaOrdemParaOBarbeiro();
        }

        // Testa se tem espaço na fila
        if (qtdCadeirasOcupadas < cadeiras) {
            if (barbeiroLivre) {
                mandaOrdemParaOBarbeiro();
            } else {
                cadeirasOcupadas[proxCadeiraVazia] = processo;
                qtdCadeirasOcupadas++; //Ocupa uma cadeira
                if (qtdCadeirasOcupadas == 1) { //Se este processo for o único na fila, definir cliente atual como próximo
                    proxCliente = proxCadeiraVazia;
                }
                if (++proxCadeiraVazia == cadeiras) { //Fazer o loop no contador se der "overflow"
                    proxCadeiraVazia = 0;
                }
            }
        } else {
            int message = FALSE;
            MPI_Send(&message, 1, MPI_INT, processo, tagResposta, MPI_COMM_WORLD);
        }
    }
}

void cortarCabelo(int processo) {
    //TODO: cortar cabelo
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
    MPI_Send(&rank, 1, MPI_INT, idCoordenador, tagPedido, MPI_COMM_WORLD);

    int resposta;
    MPI_Recv(&resposta, 1, MPI_INT, MPI_ANY_SOURCE, tagResposta, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    if (resposta) {
        //TODO: corte de cabelo finalizado
    } else {
        //TODO: não existem cadeiras, ir embora
    }
}

//---------- Mensagens exibidas durante a execução do programa ----------

void cortando_cabelo() { 
  // ou seja o barbeiro está ocupado
  printf("O Barbeiro esta cortando o cabelo de alguem!\n");
}

void cliente_chegou() {
  printf("Cliente chegou para cortar cabelo!\n");
}

void atendendo_cliente() {
  printf("Cliente esta tendo o cabelo cortado!\n");
}

void cliente_desiste() {
  printf("Cliente desistiu! (O salao estah muito cheio!)\n");
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