#ifndef BANKER_H
#define BANKER_H

typedef struct {
    int num_clientes;
    int num_recursos;
    long **maximum;
    long **allocation;
    long **need;
    long *available;
} Customer;

int validaInt(const char *s, long *saida);
int validaCustomer(const char *f, int m_esperado, Customer *c);

#endif