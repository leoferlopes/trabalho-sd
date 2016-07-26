/*
 * Copyright (c) 2004-2006 The Trustees of Indiana University and Indiana
 *                         University Research and Technology
 *                         Corporation.  All rights reserved.
 * Copyright (c) 2006      Cisco Systems, Inc.  All rights reserved.
 *
 * Simple ring test program in C.
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

//Variáveis relaciondas às cadeiras da barbearia
int* cadeirasOcupadas = malloc(cadeiras * sizeof(int)); //O array em si
int qtdCadeirasOcupadas = 0; //Contador de cadeiras ocupadas
int proxCliente = -1; //Posição do próximo cliente no array
int proxCadeiraVazia = 0; //Próxima cadeira vazia no array

int barbeiroLivre = TRUE;
int rank, size;

void coordenador() {
    int processo;

    while (1) {
        MPI_Recv(&processo, 1, MPI_INT, MPI_ANY_SOURCE, tagPedido, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // testa se a mensagem é do barbeiro
        if (processo == idBarbeiro) {
            barbeiroLivre = TRUE;
            mandaOrdemParaOBarbeiro();
        }

        // Testa se tem espaço na fila
        if (qtdCadeirasOcupadas < cadeiras) {
            if (barbeiroLivre){
                mandaOrdemParaOBarbeiro();
            } else {
                cadeirasOcupadas[proxCadeiraVazia] = processo;
                qtdCadeirasOcupadas++; //Ocupa uma cadeira
                if (qtdCadeirasOcupadas == 1){ //Se este processo for o único na fila, definir cliente atual como próximo
                    proxCliente = proxCadeiraVazia;
                }
                if (++proxCadeiraVazia == cadeiras){ //Fazer o loop no contador se der "overflow"
                    proxCadeiraVazia = 0;
                }
            }
        } else {
            int message = FALSE;
            MPI_Send(&message, 1, MPI_INT, processo, tagResposta, MPI_COMM_WORLD);
        }
    }
}

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

void barbeiro() {

}

void cliente() {

}

int main(int argc, char *argv[]) {

    /* Start up MPI */

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    switch (rank) {
        case idCoordenador:
            coordenador();
            break;
        case idBarbeiro:
            barbeiro();
            break;
        default:
            cliente();
            break;
    }

    MPI_Finalize();
    return 0;
}