#include "commands.h"
#include "customer.h"
#include "parse.h"
#include <stdio.h>
#include <stdlib.h>

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

    liberaCommands(&comandos);
    liberaCustomer(&c);
    free(total);
    return 0;
}
