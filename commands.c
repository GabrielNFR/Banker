#include "commands.h"
#include "parse.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int validaCommands(const char *f, int n, int m, ListaComandos *lista) {
    if (lista == NULL) return 1;
    lista->itens = NULL;
    lista->quantidade = 0;

    FILE *fp = fopen(f, "r");
    if (fp == NULL) {
        fprintf(stderr, "Erro ao abrir o arquivo commands.txt\n");
        return 1;
    }

    char *linha = NULL;
    size_t cap = 0;
    int total_linhas = 0;

    // Primeira passagem: conta as linhas para dimensionar a lista.
    while (getline(&linha, &cap, fp) != -1) {
        total_linhas++;
    }

    if ((ferror(fp)) || !feof(fp)) { fprintf(stderr, "erro de leitura em '%s'\n", f); goto erro; }
    if (total_linhas == 0) { fprintf(stderr, "arquivo vazio\n"); goto erro; }

    // Uma posição por linha; quantidade será preenchida no parsing.
    lista->itens = calloc(total_linhas, sizeof(*lista->itens));
    if (lista->itens == NULL) { perror("calloc"); goto erro; }

    if (fseek(fp, 0, SEEK_SET) != 0) { perror("fseek"); goto erro; }

    // Segunda passagem: valida e armazena os comandos na ordem do arquivo.
    for (int i = 0; i < total_linhas; i++) {
        if (getline(&linha, &cap, fp) == -1) {
            fprintf(stderr, "erro relendo linha %d\n", i + 1);
            goto erro;
        }

        linha[strcspn(linha, "\r\n")] = '\0';

        if (*linha == '\0') { fprintf(stderr, "linha %d vazia\n", i + 1); goto erro; }

        char *tok = strtok(linha, " \t");
        if (tok == NULL) { fprintf(stderr, "linha %d: comando esperado\n", i + 1); goto erro; }

        if (strcmp(tok, "*") == 0) {
            if (strtok(NULL, " \t") != NULL) {
                fprintf(stderr, "linha %d: token extra\n", i + 1);
                goto erro;
            }

            lista->itens[lista->quantidade].tipo = CMD_PRINT;
            lista->itens[lista->quantidade].cliente = -1;
            lista->itens[lista->quantidade].recursos = NULL;
            lista->quantidade++;
            continue;
        }

        TipoComando tipo;
        if (strcmp(tok, "RQ") == 0) {
            tipo = CMD_RQ;
        } else if (strcmp(tok, "RL") == 0) {
            tipo = CMD_RL;
        } else {
            fprintf(stderr, "linha %d: comando inválido\n", i + 1);
            goto erro;
        }

        tok = strtok(NULL, " \t");
        if (tok == NULL) {
            fprintf(stderr, "linha %d: cliente esperado\n", i + 1);
            goto erro;
        }

        char *fim;
        long indice;
        if (parseLong(tok, &fim, &indice) != PARSE_OK || *fim != '\0') {
            fprintf(stderr, "linha %d: cliente invalido\n", i + 1);
            goto erro;
        }
        if (indice < 0 || indice >= n) {
            fprintf(stderr, "linha %d: cliente %ld fora do intervalo\n", i + 1, indice);
            goto erro;
        }
        int cliente = (int)indice;

        long *recursos = malloc(m * sizeof(*recursos));
        if (recursos == NULL) {
            perror("malloc");
            goto erro;
        }

        for (int j = 0; j < m; j++) {
            tok = strtok(NULL, " \t");
            if (tok == NULL) {
                fprintf(stderr, "linha %d: esperava %d valores, encontrou %d\n",
                        i + 1, m, j);
                free(recursos);
                goto erro;
            }

            long valor;
            if (parseLong(tok, &fim, &valor) != PARSE_OK || *fim != '\0') {
                fprintf(stderr, "linha %d: valor inválido\n", i + 1);
                free(recursos);
                goto erro;
            }
            recursos[j] = valor;
        }

        if (strtok(NULL, " \t") != NULL) {
            fprintf(stderr, "linha %d: token extra\n", i + 1);
            free(recursos);
            goto erro;
        }

        // Só armazena depois de validar o cliente e todos os recursos.
        lista->itens[lista->quantidade].tipo = tipo;
        lista->itens[lista->quantidade].cliente = cliente;
        lista->itens[lista->quantidade].recursos = recursos;
        lista->quantidade++;
    }
    
    fclose(fp);
    free(linha);
    
    return 0;

    erro:
        if (fp) fclose(fp);
        free(linha);
        liberaCommands(lista);
        return 1;
}

void liberaCommands(ListaComandos *lista) {
    if (lista == NULL) return;

    if (lista->itens != NULL) {
        for (int i = 0; i < lista->quantidade; i++) {
            free(lista->itens[i].recursos);
        }
        free(lista->itens);
    }

    lista->itens = NULL;
    lista->quantidade = 0;
}
