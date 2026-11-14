#ifndef COMMANDS_H
#define COMMANDS_H

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

int validaCommands(const char *f, int n, int m, ListaComandos *lista);
void liberaCommands(ListaComandos *lista);

#endif
