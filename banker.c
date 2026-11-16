#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    PARSE_OK = 0,
    PARSE_SEM_DIGITO,
    PARSE_OVERFLOW,
    PARSE_NEGATIVO
} ResultadoParse;

typedef enum {
    CMD_RQ,
    CMD_RL,
    CMD_PRINT
} TipoComando;

typedef struct {
    TipoComando tipo;
    int cliente;
    long *recursos;
} Comando;

typedef struct {
    Comando *itens;
    int quantidade;
} ListaComandos;

typedef struct {
    int NUMBER_OF_CUSTOMERS;
    int NUMBER_OF_RESOURCES;
    long **maximum;
    long **allocation;
    long **need;
    long *available;
} Customer;

// parse: conversao de texto para long com validacao 

// Aceita zero e para no primeiro caractere invalido; o chamador valida o separador.
static ResultadoParse parseLong(const char *s, char **fim, long *saida) {
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

static int validaPos(const char *s, long *saida) {
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

/* customer: estrutura Customer, leitura do customer.txt e estado inicial    */

static void liberaCustomer(Customer *c);

static int validaCustomer(const char *f, int m_esperado, Customer *c) {
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

   // Segunda passagem: preenche maximum com os valores ja validados.
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

static void liberaCustomer(Customer *c) {
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

static int inicializaCustomer(Customer *c, const long *total) {
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

/* commands: leitura e validacao do commands.txt                             */

static void liberaCommands(ListaComandos *lista);

static int validaCommands(const char *f, int n, int m, ListaComandos *lista) {
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

    // Uma posicao por linha; quantidade sera preenchida no parsing.
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

        // So armazena depois de validar o cliente e todos os recursos.
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

static void liberaCommands(ListaComandos *lista) {
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

/* seguranca: verificarSeg, o teste de seguranca                                */

static int verificarSeg(const Customer *c) {
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

/* banker: main, execucao dos comandos e escrita do result.txt               */

static int executar(ListaComandos *lista, Customer *c, FILE *saida);
static void formataVetor(char *destino, size_t tamanho, const long *v, int m);
static void imprimeEstado(FILE *saida, const Customer *c);

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

    // Le e valida os dois arquivos antes de inicializar o estado.
    Customer c = {0};
    ListaComandos comandos = {0};
    int resultado = validaCustomer("customer.txt", NUMBER_OF_RESOURCES, &c);
    if (resultado == 0) {
        resultado = validaCommands("commands.txt", c.NUMBER_OF_CUSTOMERS,
                                   c.NUMBER_OF_RESOURCES, &comandos);
    }

    if (resultado != 0) {
        liberaCommands(&comandos);
        liberaCustomer(&c);
        free(total);
        return 1;
    }

    if (inicializaCustomer(&c, total) != 0) {
        liberaCommands(&comandos);
        free(total);
        return 1;
    }

    FILE *saida = fopen("result.txt", "w");
    if (saida == NULL) {
        fprintf(stderr, "Erro ao abrir o arquivo result.txt\n");
        liberaCommands(&comandos);
        liberaCustomer(&c);
        free(total);
        return 1;
    }

    int execucao = executar(&comandos, &c, saida);

    if (fclose(saida) != 0) {
        perror("fclose");
        execucao = 1;
    }

    liberaCommands(&comandos);
    liberaCustomer(&c);
    free(total);
    return execucao;
}

static void formataVetor(char *destino, size_t tamanho, const long *v, int m) {
    size_t usado = 0;
    destino[0] = '\0';

    for (int j = 0; j < m; j++) {
        int escrito = snprintf(destino + usado, tamanho - usado,
                               j == 0 ? "%ld" : " %ld", v[j]);
        if (escrito < 0 || (size_t)escrito >= tamanho - usado) {
            destino[usado] = '\0';
            return;
        }
        usado += (size_t)escrito;
    }
}

static void imprimeEstado(FILE *saida, const Customer *c) {
    int n = c->NUMBER_OF_CUSTOMERS;
    int m = c->NUMBER_OF_RESOURCES;
    char buf[(size_t)m * 24 + 1];

    fprintf(saida, "MAXIMUM   |   ALLOCATION  |   NEED\n");

    for (int i = 0; i < n; i++) {
        formataVetor(buf, sizeof buf, c->maximum[i], m);
        fprintf(saida, "%-10s|   ", buf);

        formataVetor(buf, sizeof buf, c->allocation[i], m);
        fprintf(saida, "%-12s|   ", buf);

        formataVetor(buf, sizeof buf, c->need[i], m);
        fprintf(saida, "%s \n", buf);
    }

    formataVetor(buf, sizeof buf, c->available, m);
    fprintf(saida, "AVAILABLE %s \n", buf);
}

static int executar(ListaComandos *lista, Customer *c, FILE *saida) {
    int m = c->NUMBER_OF_RESOURCES;
    char reqStr[(size_t)m * 24 + 1];
    char availStr[(size_t)m * 24 + 1];

    for (int i = 0; i < lista->quantidade; i++) {
        Comando *cmd = &lista->itens[i];

        switch (cmd->tipo) {
            case CMD_PRINT:
                imprimeEstado(saida, c);
                break;

            case CMD_RL: {
                int cliente = cmd->cliente;
                const long *req = cmd->recursos;

                for (int j = 0; j < m; j++) {
                    c->available[j] += req[j];
                    c->allocation[cliente][j] -= req[j];
                    c->need[cliente][j] += req[j];
                }

                formataVetor(reqStr, sizeof reqStr, req, m);
                fprintf(saida, "Release from customer %d the resources %s\n", cliente, reqStr);
                break;
            }

            case CMD_RQ: {
                int cliente = cmd->cliente;
                const long *req = cmd->recursos;
                int excede = 0;

                for (int j = 0; j < m; j++) {
                    if (req[j] > c->need[cliente][j]) {
                        excede = 1;
                        break;
                    } 
                }
                if (excede) {
                    formataVetor(reqStr, sizeof reqStr, req, m);
                    fprintf(saida, "The customer %d request %s was denied because exceed its maximum allocation\n",
                            cliente, reqStr);
                    break;
                }

                int semRecursos = 0;
                for (int j = 0; j < m; j++) {
                    if (req[j] > c->available[j]) {
                        semRecursos = 1;
                        break;
                    }
                }
                if (semRecursos) {
                    formataVetor(reqStr, sizeof reqStr, req, m);
                    formataVetor(availStr, sizeof availStr, c->available, m);
                    fprintf(saida, "The resources %s are not enough to customer %d request %s \n",
                            availStr, cliente, reqStr);
                    break;
                }

                for (int j = 0; j < m; j++) {
                    c->available[j] -= req[j];
                    c->allocation[cliente][j] += req[j];
                    c->need[cliente][j] -= req[j];
                }

                int seguro = verificarSeg(c);
                if (seguro == -1) {
                    for (int j = 0; j < m; j++) {
                        c->available[j] += req[j];
                        c->allocation[cliente][j] -= req[j];
                        c->need[cliente][j] += req[j];
                    }
                    fprintf(stderr, "Erro ao verificar o estado seguro\n");
                    return 1;
                }

                formataVetor(reqStr, sizeof reqStr, req, m);
                if (seguro == 0) {
                    for (int j = 0; j < m; j++) {
                        c->available[j] += req[j];
                        c->allocation[cliente][j] -= req[j];
                        c->need[cliente][j] += req[j];
                    }
                    fprintf(saida, "The customer %d request %s was denied because result in an unsafe state\n",
                            cliente, reqStr);
                } else {
                    fprintf(saida, "Allocate to customer %d the resources %s\n", cliente, reqStr);
                }
                break;
            }
        }
    }

    return 0;
}
