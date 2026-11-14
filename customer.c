#include "customer.h"
#include "parse.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
   
   // Primeira passagem: valida o formato e conta clientes e colunas.
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

   // Aloca a matriz depois de conhecer n e m.
   c->maximum = calloc(n, sizeof(*c->maximum));
   if (!c->maximum) { perror("calloc"); goto erro; }
   for (int i = 0; i < n; i++) {
       if (!(c->maximum[i] = malloc(m * sizeof(**c->maximum)))) {
           perror("malloc");
           goto erro;
       }
   }

   if (fseek(fp, 0, SEEK_SET) != 0) { perror("fseek"); goto erro; }

   // Segunda passagem: preenche maximum com os valores já validados.
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

int inicializaCustomer(Customer *c, const long *total) {
    c->allocation = calloc(c->NUMBER_OF_CUSTOMERS, sizeof(*c->allocation));
    if (!c->allocation) {
        perror("calloc");
        goto erro;
    }

    for (int i = 0; i < c->NUMBER_OF_CUSTOMERS; i++) {
        c->allocation[i] = calloc(c->NUMBER_OF_RESOURCES, sizeof(**c->allocation));
        if (!c->allocation[i]) {
            perror("calloc");
            goto erro;
        }
    }

    c->need = calloc(c->NUMBER_OF_CUSTOMERS, sizeof(*c->need));
    if (!c->need) {
        perror("calloc");
        goto erro;
    }

    for (int i = 0; i < c->NUMBER_OF_CUSTOMERS; i++) {
        c->need[i] = malloc(c->NUMBER_OF_RESOURCES * sizeof(**c->need));
        if (!c->need[i]) {
            perror("malloc");
            goto erro;
        }

        for (int j = 0; j < c->NUMBER_OF_RESOURCES; j++) {
            c->need[i][j] = c->maximum[i][j];
        }
    }

    c->available = malloc(c->NUMBER_OF_RESOURCES * sizeof(*c->available));
    if (!c->available) {
        perror("malloc");
        goto erro;
    }

    for (int j = 0; j < c->NUMBER_OF_RESOURCES; j++) {
        c->available[j] = total[j];
    }
    
    return 0;

    erro:
        liberaCustomer(c);
        return 1;
}
