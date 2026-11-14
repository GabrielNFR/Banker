# 1. Objetivo

O enunciado pede a implementação do algoritmo de Banker em C, com número de clientes e de recursos definidos pela entrada, compilação por Makefile, leitura de `customer.txt` e `commands.txt` e geração de `result.txt`. Também pede o tratamento de erros de argumentos e de arquivos inválidos.

# 2. Checklist de Requisitos

| Requisito do enunciado | Status | Como testar / evidência |
|:-----------------------|:-------|:------------------------|
| Implementar em C, compilar com Makefile e gerar o executável `banker` | OK | `make clean && make` compila sem warnings com `-Werror`, incluindo os módulos separados. Evidência: `evidencias.log`, 2026-11-14 17:07:28-03:00. |
| Usar dimensões dinâmicas para os recursos | OK | `m = argc - 1`; o teste com dois recursos e dois argumentos terminou sem erro. Evidência: `evidencias.log`, 2026-11-12 16:51:37-03:00. |
| Receber os argumentos como número de instâncias por recurso | Parcial | `./banker 10 5 7` é aceito e os valores são convertidos para `long`. O vetor é guardado em `total`. Evidência: `evidencias.log`, 2026-11-11 17:01:04-03:00. |
| Ler `customer.txt` e preencher a matriz `Maximum` | OK | O caminho válido terminou sem erro; a leitura e o preenchimento estão em `validaCustomer`. Evidência: `evidencias.log`, 2026-11-12 16:51:37-03:00. |
| Tratar erro de abertura e formato do `customer.txt` | OK | Arquivo ausente, linha malformada e colunas incompatíveis retornaram erro. Evidência: `evidencias.log`, 2026-11-12 16:51:37-03:00. |
| Ler `commands.txt` e validar `RQ`, `RL` e `*` | OK | O arquivo de exemplo gerou 11 comandos na ordem esperada; o GDB confirmou quantidade e tipos. Evidência: `evidencias.log`, 2026-11-13 19:33:41-03:00. |
| Tratar erro de abertura e formato do `commands.txt` | OK | Arquivo ausente, comando inválido, valores faltando ou sobrando, cliente fora do intervalo, `*` com token extra, valor negativo e linha vazia retornaram erro. Evidência: `evidencias.log`, 2026-11-13 19:33:41-03:00. |
| Inicializar `Allocation` com zeros, `Need` com `Maximum` e `Available` com `total` | OK | O GDB mostrou `n = 5`, `m = 3`, `allocation` toda zero, `need` igual a `maximum` e `available 10 5 7`. Evidência: `evidencias.log`, 2026-11-14 17:07:28-03:00. |
| Liberar a memória alocada | OK | Valgrind sem vazamentos no caminho válido com estado inicializado: 37 alocações e 37 liberações. Evidência: `evidencias.log`, 2026-11-14 17:07:28-03:00. |

# 3. Como Reproduzir

Compilar:

```sh
make clean
make
```

Executar:
```sh
./banker <recurso A> <recurso B> <recurso C>
```
Exemplo:
```sh
./banker 10 5 7
```

O programa lê `customer.txt` e `commands.txt` no diretório atual. Os testes de erro e o valgrind estão registrados no `evidencias.log`.

# 4. Arquitetura (resumida)

**Arquivos e responsabilidades:**

1. `banker.c`: ponto de entrada e orquestração das leituras, inicialização e limpeza.
2. `parse.h/.c`: `ResultadoParse`, `parseLong` e `validaPos`.
3. `customer.h/.c`: struct `Customer`, `validaCustomer`, `inicializaCustomer` e `liberaCustomer`.
4. `commands.h/.c`: `TipoComando`, `Comando`, `ListaComandos`, `validaCommands` e `liberaCommands`.
5. `Makefile`: compilação dos módulos e ligação do executável `banker`.
6. `customer.txt`: entrada de exemplo com 5 clientes e 3 recursos.
7. `commands.txt`: entrada de exemplo com comandos `RQ`, `RL` e `*`.
8. `evidencias.log`: log das sessões de terminal.
9. `gnfr.pdf`: este arquivo, é o relatório da implementação.

**Decisões técnicas:**

1. Dimensões dinâmicas: `m = argc - 1`, porque o enunciado não fixa a quantidade de recursos. Consequência: a validação compara a quantidade de colunas com `m_esperado` e a matriz é alocada com `n` e `m`.
2. Contagens em `long`: para evitar truncamento e permitir a checagem de overflow com `ERANGE`. Consequência: os valores são lidos com `parseLong` e validados antes de serem aceitos.
3. Parser manual com `strtol` no `customer.txt`: para detectar campo vazio e vírgula final. Consequência: a validação rejeita esses casos.
4. Leitura de `customer.txt` em duas passagens: a primeira valida e conta as linhas; a segunda preenche `maximum`. Consequência: erros de formato são detectados antes da alocação da matriz.
5. Leitura de `commands.txt` em duas passagens: a primeira conta as linhas; a segunda valida os tokens e preenche a lista. Consequência: a lista tem capacidade para todas as linhas e nenhum comando é executado durante o parsing.
6. Comandos em memória: `Comando` guarda tipo, cliente e recursos; `ListaComandos` guarda o vetor e a quantidade. Consequência: a ordem do arquivo é preservada e a validação acontece antes de alterar o estado.
7. Gramáticas separadas: `strtok` para os comandos separados por espaço e parser manual com vírgula para o `customer.txt`. Consequência: cada arquivo mantém o seu formato.
8. Separação em módulos: `parse`, `customer`, `commands` e `banker` ficam em pares `.h/.c`, e o `banker.h` foi removido; o Makefile usa uma regra genérica `%.o: %.c` e dependências explícitas de header. Consequência: cada módulo tem uma responsabilidade e o arquivo único com 400 linhas deixou de existir.
9. Inicialização em `inicializaCustomer`: `allocation` começa zero, `need` copia `maximum` e `available` copia `total`. Se uma alocação falha, a própria função chama `liberaCustomer`. Consequência: o `main` só orquestra e não repete essa limpeza no caminho de erro.
10. Liberação centralizada: `liberaCustomer` libera as matrizes e o vetor `available`; `liberaCommands` libera a lista e os recursos dos comandos. Consequência: o caminho de sucesso e o caminho de erro não deixam vazamentos, como mostra o valgrind.
11. Assinatura de `validaCommands`: a função recebe `n` e `m` em vez da struct `Customer`. Consequência: `commands.c` não depende de `customer.h` e conhece apenas as dimensões de que precisa. 
12. Limpeza no `main`: as chamadas de `liberaCommands` e `liberaCustomer` ficam no `main`. Consequência: `customer.c` não inclui `commands.h` e cada módulo só libera o que alocou.

# 5. Estratégias e Diário de Desenvolvimento

## 5.1 Estratégias da semana

| Estratégia | Nome curto | Contexto | Motivo da troca (se houve) |
|:-----------|:-----------|:---------|:---------------------------|
| S1 | Validação sequencial com strtol | Validar o formato do `customer.txt` linha a linha, com contador de colunas | Não houve troca de estratégia até aqui |
| S2 | Parsing de commands com strtok | Ler `commands.txt`, validar tokens e guardar a lista antes da execução | Estratégia nova para o segundo arquivo; S1 continua no `customer.txt` |
| S3 | Separação em módulos | Dividir parser, customer, commands e main em pares `.h/.c` | Refatoração para reduzir acoplamento; não houve troca de estratégia de parsing |
| S4 | Inicialização do estado | Alocar `Allocation`, `Need` e `Available` a partir de `Maximum` e `total` | Estratégia nova para a etapa 5; S1 e S2 continuam no parsing |

## 5.2 Diário de Tentativas

| # | Estrat. | O que tentei | Resultado | Hipótese/Causa (se falhou) | Quando | Evidência |
|:--|:--------|:-------------|:----------|:---------------------------|:-------|:----------|
| 1 | S1 | Validei a leitura dos argumentos. A saída foi `vetor[0] = 10`, `vetor[1] = 5` e `vetor[2] = 7`. | OK |  | 2026-11-11 17:01:04-03:00 | `evidencias.log` |
| 2 | S1 | Testei a validação do arquivo de teste e o retorno foi `0`. | Parcial | O retorno `0` indicava que o arquivo era aceito. | 2026-11-11 22:03:24-03:00 | `evidencias.log` |
| 3 | S1 | Testei a linha com vírgula final e o programa retornou `0`. | Falha | O laço aceitava fim de linha depois da vírgula. | 2026-11-11 23:39:32-03:00 | `evidencias.log` |
| 4 | S1 | Ajustei o laço e repeti o teste. A linha passou a ser rejeitada. | OK |  | 2026-11-11 23:42:06-03:00 | `evidencias.log` |
| 5 | S1 | Implementei a segunda passagem e a liberação com `liberaCustomer`. | OK |  | 2026-11-12 16:51:37-03:00 | `evidencias.log` |
| 6 | S1 | Testei o caminho válido, arquivo ausente, linha malformada, colunas incompatíveis e arquivo com dois recursos. | OK |  | 2026-11-12 16:51:37-03:00 | `evidencias.log` |
| 7 | S2 | Implementei a leitura de `commands.txt` com validação de `RQ`, `RL` e `*` e armazenamento em lista. | OK |  | 2026-11-13 19:33:41-03:00 | `evidencias.log` |
| 8 | S2 | Testei o `commands.txt` de exemplo e os casos inválidos: arquivo ausente, comando inválido, valores faltando ou sobrando, cliente fora do intervalo, `*` com token extra, valor negativo e linha vazia. | OK |  | 2026-11-13 19:33:41-03:00 | `evidencias.log` |
| 9 | S2 | Rodei o valgrind no caminho válido e no caminho de erro do `commands.txt`. | OK |  | 2026-11-13 19:33:41-03:00 | `evidencias.log` |
| 10 | S3 | Separei as funções em `parse`, `customer`, `commands` e `main`, removi o `banker.h` e atualizei as dependências no Makefile. | OK |  | 2026-11-14 17:07:28-03:00 | `evidencias.log` |
| 11 | S4 | Implementei `inicializaCustomer` com `allocation` zero, `need` copiado de `maximum` e `available` copiado de `total`. | OK |  | 2026-11-14 17:07:28-03:00 | `evidencias.log` |
| 12 | S4 | Conferi o estado inicial com GDB e rodei o valgrind. | OK |  | 2026-11-14 17:07:28-03:00 | `evidencias.log` |

# 6. Evidências

## 6.1 Log automático

Usei `script -a evidencias.log` nas sessões. O log registra comandos, saídas e timestamps.

## 6.2 Prints

Não há prints no repositório. As evidências desta etapa estão no `evidencias.log`.

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
| Compilei do zero seguindo só o que está no relatório | Sim. `make clean && make` funciona. |
| Rodei os testes e evidencias.log foi gerado | Sim. |
| Tenho prints obrigatórios | |
| Testei pelo menos um caso limite e um caso inválido | Sim. |
| Preenchi a seção de uso de IA | Sim. |
| Revisei o relatório e removi frases genéricas / vazias | Pendente. |

# 10. Se eu tivesse mais 2 horas

Aplique as instruções abaixo de forma objetiva e verificável, evitando texto genérico.
Escreva 5-10 linhas sobre o que você melhoraria (robustez, testes, warnings, performance, organização).

*[...]*

# Regras de Integridade (não mudam)

- Ocorrência relevante sem evidência (log ou print) = não conta para a nota.
- Contradição entre prints, log e relato = relatório perde credibilidade.
- Indício de evidência fabricada = entrega invalidada.