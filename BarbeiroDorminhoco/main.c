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

int rank, size;

void coordenador() {
    int processo;
    int barbeiroLivre = TRUE;
    while (1) {
        MPI_Recv(&processo, 1, MPI_INT, MPI_ANY_SOURCE, tagPedido, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
        // testa se a mensagem é do barbeiro
        if(processo == idBarbeiro){
            // barbeiro avisa que está livre
            // manda uma ordem de serviço para ele
        }
        
        // Testa se tem espaço na fila
        if (0) {
            // Enfilera o pedido do processo
            // ou já manda para o barbeiro se o barbeiro estiver livre
        } else {
            int message = FALSE;
            MPI_Send(&message, 1, MPI_INT, processo, tagResposta, MPI_COMM_WORLD);
        }
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