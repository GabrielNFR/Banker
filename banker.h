#ifndef BANKER_H
#define BANKER_H

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

ResultadoParse parseLong(const char *s, char **fim, long *saida);
int validaPos(const char *s, long *saida); 
int validaCustomer(const char *f, int m_esperado, Customer *c);
int validaCommands(const char *f, const Customer *c, ListaComandos *lista);
void liberaCommands(ListaComandos *lista);
void liberaCustomer(Customer *c);

#endif