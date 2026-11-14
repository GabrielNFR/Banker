#include <stdio.h>
#include <stdlib.h>
#include "safety.h"

int verificarSeg(const Customer *c) {
    int n = c->NUMBER_OF_CUSTOMERS;
    int m = c->NUMBER_OF_RESOURCES;

    long *work = calloc(m, sizeof(long));
    if (!work) {
        perror("calloc");
        return -1;
    }

    int *finish = calloc(n, sizeof(int));
    if (!finish) {
        perror("calloc");
        free(work);
        return -1;
    }

    for (int i = 0; i < m; i++) {
        work[i] = c->available[i];
    }

    int progresso = 1;

    while (progresso) {
        progresso = 0;

        for (int i = 0; i < n; i++) {
            if (finish[i] == 1) continue;
            int cabe = 1;

            for (int j = 0; j < m; j++) {
                if (c->need[i][j] > work[j]) {
                    cabe = 0;
                    break;
                }
            }
            if (cabe == 0) continue;

            for (int j = 0; j < m; j++) {
                work[j] += c->allocation[i][j];
            }
            finish[i] = 1;
            progresso = 1;
        }
    }

    int seguro = 1;
    for (int i = 0; i < n; i++) {
        if (finish[i] == 0) {
            seguro = 0;
            break;
        }
    }

    free(finish);
    free(work);
    
    return seguro;
}