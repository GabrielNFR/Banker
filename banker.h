#ifndef BANKER_H
#define BANKER_H

typedef enum {
    PARSE_OK = 0,
    PARSE_SEM_DIGITO,
    PARSE_OVERFLOW,
    PARSE_NEGATIVO
} ResultadoParse;

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
int validaCommands(const char *f, Customer *c);
void liberaCustomer(Customer *c);

#endif