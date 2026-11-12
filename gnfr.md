# 1. Objetivo

O enunciado pede a implementação do algoritmo de Banker em C, com número de clientes e de recursos definidos pela entrada, compilação por Makefile, leitura de `customer.txt` e `commands.txt` e geração de `result.txt`. Também pede o tratamento de erros de argumentos e de arquivos inválidos.

# 2. Checklist de Requisitos

| Requisito do enunciado | Status | Como testar / evidência |
|:-----------------------|:-------|:------------------------|
| Implementar em C, compilar com Makefile e gerar o executável `banker` | OK | `make clean && make` compila sem warnings com `-Werror`. Evidência: `evidencias.log`, 2026-11-12 16:51:37-03:00. |
| Usar dimensões dinâmicas para os recursos | OK | `m = argc - 1`; o teste com dois recursos e dois argumentos terminou sem erro. Evidência: `evidencias.log`, 2026-11-12 16:51:37-03:00. |
| Receber os argumentos como número de instâncias por recurso | Parcial | `./banker 10 5 7` é aceito e os valores são convertidos para `long`. O vetor é guardado em `total`. Evidência: `evidencias.log`, 2026-11-11 17:01:04-03:00. |
| Ler `customer.txt` e preencher a matriz `Maximum` | OK | O caminho válido terminou sem erro; a leitura e o preenchimento estão em `validaCustomer`. Evidência: `evidencias.log`, 2026-11-12 16:51:37-03:00. |
| Tratar erro de abertura e formato do `customer.txt` | OK | Arquivo ausente, linha malformada e colunas incompatíveis retornaram erro. Evidência: `evidencias.log`, 2026-11-12 16:51:37-03:00. |
| Liberar a memória alocada | OK | O valgrind não encontrou vazamentos no caminho válido. Evidência: `evidencias.log`, 2026-11-12 16:51:37-03:00. |

# 3. Como Reproduzir

Compilar:

```sh
make clean
make
```

Executar:

```sh
./banker 10 5 7
```

Os testes de erro e o valgrind estão registrados no `evidencias.log`.

# 4. Arquitetura (resumida)

**Arquivos e responsabilidades:**

1. `banker.c`: ponto de entrada, validação dos argumentos, leitura de `customer.txt` em duas passagens, preenchimento de `maximum` e liberação com `liberaCustomer`.
2. `banker.h`: definição da struct `Customer` e protótipos de `validaInt`, `validaCustomer` e `liberaCustomer`.
3. `Makefile`: compilação do executável `banker` e alvo `clean`.
4. `evidencias.log`: log das sessões de terminal.
5. `gnfr.pdf`: este arquivo, é o relatório da implementação.

**Decisões técnicas:**

1. Dimensões dinâmicas: `m = argc - 1`, porque o enunciado não fixa a quantidade de recursos. Consequência: a validação compara a quantidade de colunas com `m_esperado` e a matriz é alocada com `n` e `m`.
2. Contagens em `long`: para evitar truncamento e permitir a checagem de overflow com `ERANGE`. Consequência: os valores são lidos com `strtol` e validados antes de serem aceitos.
3. Parser manual com `strtol` em vez de `strtok`: para detectar campo vazio e vírgula final. Consequência: a validação rejeita esses casos.
4. Leitura em duas passagens: a primeira valida o arquivo e conta as linhas; a segunda preenche `maximum`. Consequência: erros de formato são detectados antes da alocação da matriz.
5. Liberação centralizada: `liberaCustomer` libera as matrizes e o vetor `available`. Consequência: o caminho de sucesso e o caminho de erro não deixam vazamentos, como mostra o valgrind.

# 5. Estratégias e Diário de Desenvolvimento

## 5.1 Estratégias da semana

| Estratégia | Nome curto | Contexto | Motivo da troca (se houve) |
|:-----------|:-----------|:---------|:---------------------------|
| S1 | Validação sequencial com strtol | Validar o formato do `customer.txt` linha a linha, com contador de colunas | Não houve troca de estratégia até aqui |

## 5.2 Diário de Tentativas

| # | Estrat. | O que tentei | Resultado | Hipótese/Causa (se falhou) | Quando | Evidência |
|:--|:--------|:-------------|:----------|:---------------------------|:-------|:----------|
| 1 | S1 | Validei a leitura dos argumentos. A saída foi `vetor[0] = 10`, `vetor[1] = 5` e `vetor[2] = 7`. | OK |  | 2026-11-11 17:01:04-03:00 | `evidencias.log` |
| 2 | S1 | Testei a validação do arquivo de teste e o retorno foi `0`. | Parcial | O retorno `0` indicava que o arquivo era aceito. | 2026-11-11 22:03:24-03:00 | `evidencias.log` |
| 3 | S1 | Testei a linha com vírgula final e o programa retornou `0`. | Falha | O laço aceitava fim de linha depois da vírgula. | 2026-11-11 23:39:32-03:00 | `evidencias.log` |
| 4 | S1 | Ajustei o laço e repeti o teste. A linha passou a ser rejeitada. | OK |  | 2026-11-11 23:42:06-03:00 | `evidencias.log` |
| 5 | S1 | Implementei a segunda passagem e a liberação com `liberaCustomer`. | OK |  | 2026-11-12 16:51:37-03:00 | `evidencias.log` |
| 6 | S1 | Testei o caminho válido, arquivo ausente, linha malformada, colunas incompatíveis e arquivo com dois recursos. | OK |  | 2026-11-12 16:51:37-03:00 | `evidencias.log` |

# 6. Evidências

## 6.1 Log automático

Usei `script -a evidencias.log` nas sessões. O log registra comandos, saídas e timestamps.

Na sessão iniciada em 2026-11-12 16:51:37-03:00:

- `make clean && make` compilou sem warnings.
- O caminho válido com `./banker 10 5 7` terminou sem erro.
- Arquivo ausente, linha malformada e colunas incompatíveis retornaram erro.
- O teste com dois recursos e dois argumentos terminou sem erro.
- O valgrind não encontrou vazamentos no caminho válido: 0 bytes em uso no fim, 10 alocações e 10 liberações.

## 6.2 Prints

Não há prints no repositório até o momento. As evidências desta etapa estão no `evidencias.log`. Os prints do erro principal e da validação final ficam pendentes.

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