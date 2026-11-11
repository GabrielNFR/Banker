#include "banker.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    if (argc - 1 < 1) {
        fprintf(stderr, "Deve conter pelo menos 1 argumento.\n");
        return 1;
    }

    int n = argc - 1;
    long *total = malloc(n * sizeof(long));
    if (total == NULL) {
        perror("Erro ao alocar memória para o vetor 'total'.\n");
        return 1;
    }

    for (int i = 0; i < n; i++) {
        if (validaInt(argv[i + 1], &total[i]) != 0) {
            free(total);
            return 1;
        }
    }

    for (int i = 0; i < n; i++) {
        printf("vetor[%d] = %ld\n", i, total[i]);
    }

    free(total);
    
}

int validaInt(const char *s, long *saida) {
    errno = 0;
    char *fim;
    long valor = strtol(s, &fim, 10);
    if (errno == ERANGE) {
        fprintf(stderr, "Erro: um ou mais dos argumentos é grande demais.\n");
        return 1;
    }
    if (fim == s) {
        fprintf(stderr, "Erro: um ou mais argumentos não são números inteiros positivos.\n");
        return 1;
    }
    if (*fim != '\0') {
        fprintf(stderr, "Erro: um ou mais argumentos não são números inteiros positivos.\n");
        return 1;
    }
    if (valor <= 0) {
        fprintf(stderr, "Erro: um ou mais argumentos não são números inteiros positivos.\n");
        return 1;
    }

    *saida = valor;
    return 0;
}
