#include "banker.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
    if (argc - 1 < 1) {
        fprintf(stderr, "Deve conter pelo menos 1 argumento.\n");
        return 1;
    }

    int NUMBER_OF_RESOURCES = argc - 1;
    long *total = malloc(NUMBER_OF_RESOURCES * sizeof(long));
    if (total == NULL) {
        fprintf(stderr, "Erro ao alocar memória para o vetor 'total'.\n");
        return 1;
    }

    for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
        if (validaPos(argv[i + 1], &total[i]) != 0) {
            free(total);
            return 1;
        }
    }

    Customer c = {0};
    ListaComandos comandos = {0};
    int resultado = validaCustomer("customer.txt", NUMBER_OF_RESOURCES, &c);
    if (resultado == 0) {
        resultado = validaCommands("commands.txt", &c, &comandos);
    }

    liberaCommands(&comandos);
    liberaCustomer(&c);
    free(total);

    return resultado;
}

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

int validaCustomer(const char *f, int m_esperado, Customer *c) {
   FILE *fp = fopen(f, "r");
   if (fp == NULL) {
       fprintf(stderr, "Erro ao abrir o arquivo customer.txt\n");
       return 1;
   }

   char *linha = NULL;
   size_t cap = 0;
   int n = 0;
   int m = -1;
   
   memset(c, 0, sizeof *c);
   
   while (getline(&linha, &cap, fp) != -1) {
       linha[strcspn(linha, "\r\n")] = '\0';
       n++;

       if (*linha == '\0') {
           fprintf(stderr, "Linha %d vazia\n", n); goto erro;
       }

       const char *s = linha;
       int count = 0;
       for (;;) {
           while (*s == ' ' || *s == '\t') s++;

           char *fim;
           long v;
           ResultadoParse resultado = parseLong(s, &fim, &v);
           if (resultado == PARSE_SEM_DIGITO) { fprintf(stderr, "linha %d: numero esperado\n", n); goto erro; }
           if (resultado == PARSE_OVERFLOW) { fprintf(stderr, "linha %d: overflow\n", n); goto erro; }
           if (resultado == PARSE_NEGATIVO) { fprintf(stderr, "linha %d: valor negativo\n", n); goto erro; }
           count++;
           s = fim;

           while (*s == ' ' || *s == '\t') s++;
           if (!*s) break;
           if (*s != ',') { fprintf(stderr, "linha %d: esperado ',' ou fim\n", n); goto erro; }
           s++;
       }
     
       if (m < 0) {
           m = count;
       }
       else if (count != m) {
           fprintf(stderr, "linha %d: %d colunas(s), esperado %d\n", n, count, m);
           goto erro;
       }
   }
   if ((ferror(fp)) || !feof(fp)) { fprintf(stderr, "erro de leitura em '%s'\n", f); goto erro; }

   if (n == 0) { fprintf(stderr, "arquivo vazio\n"); goto erro; }
   if (m != m_esperado) {
       fprintf(stderr, "arquivo tem %d coluna(s), esperado %d\n", m, m_esperado);
       goto erro;
   }

   c->maximum = calloc(n, sizeof(*c->maximum));
   if (!c->maximum) { perror("calloc"); goto erro; }
   for (int i = 0; i < n; i++) {
       if (!(c->maximum[i] = malloc(m * sizeof(**c->maximum)))) {
           perror("malloc");
           goto erro;
       }
   }

   if (fseek(fp, 0, SEEK_SET) != 0) { perror("fseek"); goto erro; }

   for (int i = 0; i < n; i++) {
       if (getline(&linha, &cap, fp) == -1) {
           fprintf(stderr, "erro relendo linha %d\n", i + 1);
           goto erro;
       }
       
       linha[strcspn(linha, "\r\n")] = '\0';
       const char *s = linha;

       for (int j = 0; j < m; j++) {
           while (*s == ' ' || *s == '\t') s++;
           char *fim;
           if (parseLong(s, &fim, &c->maximum[i][j]) != PARSE_OK) {
               fprintf(stderr, "erro relendo linha %d\n", i + 1);
               goto erro;
           }
           s = fim;
           while (*s == ' ' || *s == '\t') s++;
           if (*s == ',') s++;
       }
   }

   fclose(fp);
   free(linha);

   c->NUMBER_OF_CUSTOMERS = n;
   c->NUMBER_OF_RESOURCES = m;
   
   return 0;

erro:
    if (fp) fclose(fp);
    free(linha);
    if (c->maximum) {
        for (int i = 0; i < n; i++) free(c->maximum[i]);
        free(c->maximum);
        c->maximum = NULL;
    }
    return 1;
}

int validaCommands(const char *f, const Customer *c, ListaComandos *lista) {
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

    while (getline(&linha, &cap, fp) != -1) {
        total_linhas++;
    }

    if ((ferror(fp)) || !feof(fp)) { fprintf(stderr, "erro de leitura em '%s'\n", f); goto erro; }
    if (total_linhas == 0) { fprintf(stderr, "arquivo vazio\n"); goto erro; }

    lista->itens = calloc(total_linhas, sizeof(*lista->itens));
    if (lista->itens == NULL) { perror("calloc"); goto erro; }

    if (fseek(fp, 0, SEEK_SET) != 0) { perror("fseek"); goto erro; }

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
        if (indice < 0 || indice >= c->NUMBER_OF_CUSTOMERS) {
            fprintf(stderr, "linha %d: cliente %ld fora do intervalo\n", i + 1, indice);
            goto erro;
        }
        int cliente = (int)indice;

        long *recursos = malloc(c->NUMBER_OF_RESOURCES * sizeof(*recursos));
        if (recursos == NULL) {
            perror("malloc");
            goto erro;
        }

        for (int j = 0; j < c->NUMBER_OF_RESOURCES; j++) {
            tok = strtok(NULL, " \t");
            if (tok == NULL) {
                fprintf(stderr, "linha %d: esperava %d valores, encontrou %d\n",
                        i + 1, c->NUMBER_OF_RESOURCES, j);
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

void liberaCustomer(Customer *c) {
    if (c == NULL) return;

    if (c->maximum != NULL) {
        for (int i = 0; i < c->NUMBER_OF_CUSTOMERS; i++) free(c->maximum[i]);
        free(c->maximum);
        c->maximum = NULL;
    }

    if (c->allocation != NULL) {
        for (int i = 0; i < c->NUMBER_OF_CUSTOMERS; i++) free(c->allocation[i]);
        free(c->allocation);
        c->allocation = NULL;
    }

    if (c->need != NULL) {
        for (int i = 0; i < c->NUMBER_OF_CUSTOMERS; i++) free(c->need[i]);
        free(c->need);
        c->need = NULL;
    }

    free(c->available);
    c->available = NULL;

    c->NUMBER_OF_CUSTOMERS = 0;
    c->NUMBER_OF_RESOURCES = 0;
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
