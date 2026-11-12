# 1. Objetivo

O enunciado pede a implementação do algoritmo de Banker em C, com número de clientes e de recursos definidos pela entrada, compilação por Makefile, leitura de `customer.txt` e `commands.txt` e geração de `result.txt`. Também pede o tratamento de erros de argumentos e de arquivos inválidos.

# 2. Checklist de Requisitos

| Requisito do enunciado | Status | Como testar / evidência |
|:-----------------------|:-------|:------------------------|
| Implementar em C, compilar com Makefile e gerar o executável `banker` | OK | `make clean && make` compila e gera `banker`. Evidência: `evidencias.log`, sessão iniciada em 2026-11-11 17:00:59-03:00. |
| Usar dimensões dinâmicas para os recursos | Parcial | `m = argc - 1` no `main`; a validação do `customer.txt` compara as colunas com `m_esperado`. Evidência: `banker.c`. |
| Receber os argumentos como número de instâncias por recurso | Parcial | `./banker 10 5 7` é aceito e os valores são convertidos para `long`. O vetor é guardado em `total`. Evidência: `evidencias.log`, linhas 11 a 19. |
| Validar o formato do `customer.txt` | Parcial | O parser valida as linhas e rejeita vírgula final. Evidência: `evidencias.log`, linhas 53 a 66 e 72 a 86. |
| Tratar erro de abertura e formato do `customer.txt` | Parcial | `validaCustomer` trata `fopen` e formato. Evidência: `banker.c`. |

# 3. Como Reproduzir

Compilar:

```sh
make clean
make
```

Executar:

```sh
./banker <recurso1> <recurso2> <recurso3>
Exemplo: 
./banker 10 5 7
```

# 4. Arquitetura (resumida)

**Arquivos e responsabilidades:**

1. `banker.c`: ponto de entrada, validação dos argumentos e parser de validação do `customer.txt`.
2. `banker.h`: definição da struct `Customer` e protótipos de `validaInt` e `validaCustomer`.
3. `Makefile`: compilação do executável `banker` e alvo `clean`.
4. `evidencias.log`: log das sessões de terminal.
5. `gnfr.pdf`: este arquivo, é o relatório da implementação.

**Decisões técnicas:**

1. Dimensões dinâmicas: `m = argc - 1`, porque o enunciado não fixa a quantidade de recursos. Consequência: a validação do `customer.txt` compara a quantidade de colunas com `m_esperado`.
2. Contagens em `long`: para evitar truncamento e permitir a checagem de overflow com `ERANGE`. Consequência: os valores são lidos com `strtol` e validados antes de serem aceitos.
3. Parser manual com `strtol` em vez de `strtok`: para detectar campo vazio e vírgula final. Consequência: a validação rejeita esses casos.

# 5. Estratégias e Diário de Desenvolvimento

## 5.1 Estratégias da semana

| Estratégia | Nome curto | Contexto | Motivo da troca (se houve) |
|:-----------|:-----------|:---------|:---------------------------|
| S1 | Validação sequencial com strtol | Validar o formato do `customer.txt` linha a linha, com contador de colunas | Não houve troca de estratégia até aqui |

## 5.2 Diário de Tentativas

| # | Estrat. | O que tentei | Resultado | Hipótese/Causa (se falhou) | Quando | Evidência |
|:--|:--------|:-------------|:----------|:---------------------------|:-------|:----------|
| 1 | S1 | Validei a leitura dos argumentos. A saída foi `vetor[0] = 10`, `vetor[1] = 5` e `vetor[2] = 7`. | OK |  | 2026-11-11 17:01:04-03:00 | `evidencias.log`, linhas 11 a 19 |
| 2 | S1 | Testei a validação do arquivo de teste e o retorno foi `0`. | Parcial | O retorno `0` indicava que o arquivo era aceito. | 2026-11-11 22:03:24-03:00 | `evidencias.log`, linhas 39 a 45 |
| 3 | S1 | Testei a linha com vírgula final e o programa retornou `0`. | Falha | O laço aceitava fim de linha depois da vírgula. | 2026-11-11 23:39:32-03:00 | `evidencias.log`, linhas 53 a 66 |
| 4 | S1 | Ajustei o laço e repeti o teste. A linha passou a ser rejeitada. | OK |  | 2026-11-11 23:42:06-03:00 | `evidencias.log`, linhas 72 a 86 |

# 6. Evidências

A regra continua: toda afirmação relevante precisa de prova. O que muda é como capturar essa prova, para não depender de print manual toda hora.

## 6.1 Log automático (faz o trabalho pesado por você)

No início da sessão de trabalho, rode:

```
script -a evidencias.log
```

Isso grava automaticamente todos os comandos e saídas do terminal num arquivo, sem você precisar tirar print de cada linha. Comece a sessão com date, whoami e pwd, assim o log já tem timestamp, usuário e diretório sem esforço extra. **O evidencias.log deve ser enviado dentro do .tar.**

## 6.2 Prints

- Erro principal: o momento do bug mais relevante da semana.
- Validação final: teste passando / saída correta.

Prints extras só se algo excepcional aconteceu e o log sozinho não explica, você precisa ter esse feeling, mas posso dar uma ajuda (ex.: comportamento visual, crash com stack trace longo).

# 7. Uso de IA

Obrigatório preencher mesmo se a resposta for "não utilizei".

**Onde usei IA (2–3 linhas):** 

Utilizei a IA para elaboração de testes para verificar se meu código estava correto.

**Prompts principais (liste, sem história):**

1. *Elabore testes de casos de borda para verificar que o gerenciamento de erros do meu código funciona*

**O que validei manualmente e como:** Rodei todos os testes e confirmei que a build passa com -Werror, sem aviso. Escrevi todo o código da implementação e o preenchimento do relatório na mão. Utilizei a template do guia de relatórios disponibilizada no classroom, reproduzi ela em markdown, e depois converti para pdf.

# 8. Reflexão Final (5–8 linhas)

O que você melhoraria com mais tempo, principais decisões técnicas e erros que evitaria na próxima. Tudo junto, sem subseções separadas.

**Reflexão:** *[...]*

# 9. Checklist Final de Entrega

| Item | Confirmado? |
|:-----|:------------|
| Compilei do zero seguindo só o que está no relatório |  |
| Rodei os testes e evidencias.log foi gerado |  |
| Tenho prints obrigatórios |  |
| Testei pelo menos um caso limite e um caso inválido |  |
| Preenchi a seção de uso de IA |  |
| Revisei o relatório e removi frases genéricas / vazias |  |

# 10. Se eu tivesse mais 2 horas

Aplique as instruções abaixo de forma objetiva e verificável, evitando texto genérico.
Escreva 5-10 linhas sobre o que você melhoraria (robustez, testes, warnings, performance, organização).

*[...]*

# Regras de Integridade (não mudam)

- Ocorrência relevante sem evidência (log ou print) = não conta para a nota.
- Contradição entre prints, log e relato = relatório perde credibilidade.
- Indício de evidência fabricada = entrega invalidada.