#ifndef PARSE_H
#define PARSE_H

typedef enum {
    PARSE_OK = 0,
    PARSE_SEM_DIGITO,
    PARSE_OVERFLOW,
    PARSE_NEGATIVO
} ResultadoParse;

ResultadoParse parseLong(const char *s, char **fim, long *saida);
int validaPos(const char *s, long *saida);

#endif
