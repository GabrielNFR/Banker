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

    int num_recursos = argc - 1;
    long *total = malloc(num_recursos * sizeof(long));
    if (total == NULL) {
        fprintf(stderr, "Erro ao alocar memória para o vetor 'total'.\n");
        return 1;
    }

    for (int i = 0; i < num_recursos; i++) {
        if (validaInt(argv[i + 1], &total[i]) != 0) {
            free(total);
            return 1;
        }
    }

    free(total);

    return 0;
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

int validaCustomer(const char *f, int m_esperado, Customer *c) {
   FILE *fp = fopen(f, "r");
   if (fp == NULL) {
       fprintf(stderr, "Erro ao abrir o arquivo customer.txt");
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

           errno = 0;
           char *fim;
           long v = strtol(s, &fim, 10);
           if (fim == s) { fprintf(stderr, "linha %d: numero esperado\n", n); goto erro; }
           if (errno == ERANGE) { fprintf(stderr, "linha %d: overflow\n", n); goto erro; }
           if (v < 0) { fprintf(stderr, "linha %d: valor negativo\n", n); goto erro; }
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

   if (n == 0) { fprintf(stderr, "arquivo vazio\n"); goto erro; }
   if (m != m_esperado) {
       fprintf(stderr, "arquivo tem %d coluna(s), esperado %d\n", m, m_esperado);
       goto erro;
   }

   return 0;

erro:
    fclose(fp);
    free(linha);
    return 1;
}