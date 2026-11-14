#include "parse.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

// Aceita zero e para no primeiro caractere inválido; o chamador valida o separador.
ResultadoParse parseLong(const char *s, char **fim, long *saida) {
    errno = 0;
    char *fim_local;
    long valor = strtol(s, &fim_local, 10);
    *fim = fim_local;

    if (fim_local == s) {
        return PARSE_SEM_DIGITO;
    }
    if (errno == ERANGE) {
        return PARSE_OVERFLOW;
    }
    if (valor < 0) {
        return PARSE_NEGATIVO;
    }

    *saida = valor;
    return PARSE_OK;
}

int validaPos(const char *s, long *saida) {
    char *fim;
    ResultadoParse resultado = parseLong(s, &fim, saida);

    if (resultado == PARSE_OVERFLOW) {
        fprintf(stderr, "Erro: um ou mais dos argumentos é grande demais.\n");
        return 1;
    }
    if (resultado != PARSE_OK) {
        fprintf(stderr, "Erro: um ou mais argumentos não são números inteiros positivos.\n");
        return 1;
    }
    if (*fim != '\0') {
        fprintf(stderr, "Erro: um ou mais argumentos não são números inteiros positivos.\n");
        return 1;
    }
    if (*saida == 0) {
        fprintf(stderr, "Erro: um ou mais argumentos não são números inteiros positivos.\n");
        return 1;
    }

    return 0;
}
