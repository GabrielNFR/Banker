#ifndef CUSTOMER_H
#define CUSTOMER_H

typedef struct {
    int NUMBER_OF_CUSTOMERS;
    int NUMBER_OF_RESOURCES;
    long **maximum;
    long **allocation;
    long **need;
    long *available;
} Customer;

int validaCustomer(const char *f, int m_esperado, Customer *c);
int inicializaCustomer(Customer *c, const long *total);
void liberaCustomer(Customer *c);

#endif
