#include "commands.h"
#include "customer.h"
#include "parse.h"
#include "safety.h"
#include <stdio.h>
#include <stdlib.h>

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

    // Lê e valida os dois arquivos antes de inicializar o estado.
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