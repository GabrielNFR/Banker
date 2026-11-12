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

    Customer c = {0};
    int resultado = validaCustomer("customer.txt", num_recursos, &c);

    liberaCustomer(&c);
    free(total);

    return resultado;
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
   if (ferror(fp)) { fprintf(stderr, "erro de leitura em '%s'\n", f); goto erro; }

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
           c->maximum[i][j] = strtol(s, &fim, 10);
           s = fim;
           while (*s == ' ' || *s == '\t') s++;
           if (*s == ',') s++;
       }
   }

   fclose(fp);
   free(linha);

   c->num_clientes = n;
   c->num_recursos = m;
   
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

void liberaCustomer(Customer *c) {
    if (c == NULL) return;

    if (c->maximum != NULL) {
        for (int i = 0; i < c->num_clientes; i++) free(c->maximum[i]);
        free(c->maximum);
        c->maximum = NULL;
    }

    if (c->allocation != NULL) {
        for (int i = 0; i < c->num_clientes; i++) free(c->allocation[i]);
        free(c->allocation);
        c->allocation = NULL;
    }

    if (c->need != NULL) {
        for (int i = 0; i < c->num_clientes; i++) free(c->need[i]);
        free(c->need);
        c->need = NULL;
    }

    free(c->available);
    c->available = NULL;

    c->num_clientes = 0;
    c->num_recursos = 0;
}