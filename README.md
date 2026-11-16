# Algoritmo de Banker

Implementação do algoritmo de Banker (Silberschatz et al., *Operating System Concepts*, 10ª edição, p. P-45) em C, em um único arquivo `banker.c`. O número de clientes e de recursos é definido pela entrada, não pelo código.

O programa lê `customer.txt` e `commands.txt` do diretório atual, aplica os comandos na ordem e escreve o transcript em `result.txt`.

## Compilação

Requer um compilador C com suporte a `-std=gnu11` e `getline`, como `gcc` ou `clang`, e `make`.

```sh
make
```

O Makefile compila `banker.c` em um único objeto e liga o executável `banker`. Para remover o executável e o objeto:

```sh
make clean
```

## Execução

```sh
./banker 10 5 7
```

Cada argumento é o número de instâncias de um tipo de recurso. A quantidade de recursos é `argc - 1`, então o exemplo acima tem três recursos com 10, 5 e 7 instâncias. Os argumentos precisam ser inteiros positivos.

O programa usa o diretório atual para os três arquivos. Se `customer.txt` ou `commands.txt` não existir, não puder ser aberto ou estiver fora do formato, ele imprime uma mensagem em `stderr` e encerra com código diferente de zero.

## Formato de `customer.txt`

Uma linha por cliente. A quantidade de linhas é o número de clientes. Cada linha tem a mesma quantidade de colunas, igual ao número de argumentos, separadas por vírgula. Não pode haver campo vazio nem vírgula no fim.

```text
7,5,3
3,2,2
9,0,2
2,2,2
4,3,3
```

Esse arquivo tem 5 clientes e 3 recursos. Os valores viram a matriz `Maximum`.

## Formato de `commands.txt`

Uma linha por comando, com campos separados por espaço. Os comandos são:

- `RQ cliente a b ...`: pedido de alocação, com exatamente um valor por recurso;
- `RL cliente a b ...`: liberação, com exatamente um valor por recurso;
- `*`: imprime o estado atual, sem nenhum outro token na linha.

O índice do cliente vai de `0` até `n - 1`.

```text
RQ 0 0 2 1
RQ 2 3 0 2
RQ 1 2 0 0
RQ 4 0 0 2
RL 0 0 1 1
RQ 3 2 1 1
*
RQ 4 4 0 0
RQ 1 1 0 2
RQ 0 0 2 0
RQ 2 0 1 0
```

## Saída

O programa escreve `result.txt` com uma linha por operação aceita ou negada e, para cada `*`, a tabela do estado. A saída esperada para os exemplos acima é:

```text
Allocate to customer 0 the resources 0 2 1
Allocate to customer 2 the resources 3 0 2
Allocate to customer 1 the resources 2 0 0
Allocate to customer 4 the resources 0 0 2
Release from customer 0 the resources 0 1 1
Allocate to customer 3 the resources 2 1 1
MAXIMUM   |   ALLOCATION  |   NEED
7 5 3     |   0 1 0       |   7 4 3
3 2 2     |   2 0 0       |   1 2 2
9 0 2     |   3 0 2       |   6 0 0
2 2 2     |   2 1 1       |   0 1 1
4 3 3     |   0 0 2       |   4 3 1
AVAILABLE 3 3 2
The resources 3 3 2 are not enough to customer 4 request 4 0 0
Allocate to customer 1 the resources 1 0 2
The customer 0 request 0 2 0 was denied because result in an unsafe state
The customer 2 request 0 1 0 was denied because exceed its maximum allocation
```

O `result.txt` do projeto é a referência exata, incluindo os espaços no fim das linhas da tabela.

As mensagens de negação são:

- `The customer N request A B C was denied because exceed its maximum allocation` quando o pedido passa do que o cliente ainda pode pedir;
- `The resources X Y Z are not enough to customer N request A B C` quando o pedido passa do que está disponível;
- `The customer N request A B C was denied because result in an unsafe state` quando o estado simulado deixa de ser seguro.

Um pedido negado não interrompe a execução. O programa continua no próximo comando e só encerra com erro se um arquivo não puder ser lido ou se faltar memória.

## Como o programa decide

O estado é formado por `Maximum`, `Allocation`, `Need` e `Available`. `Need` começa igual a `Maximum` e é atualizado junto com `Allocation`.

Para `RQ`, as checagens acontecem nesta ordem:

1. o pedido cabe em `Need`;
2. o pedido cabe em `Available`;
3. o estado continua seguro depois de simular a alocação.

Se a terceira falhar, a alocação é desfeita e o pedido é negado. Para `RL`, o programa faz o inverso: devolve ao `Available`, retira da `Allocation` e soma ao `Need`. O comando `*` apenas imprime o estado.

O teste de segurança está em `verificarSeg`. Ele copia `Available` para `Work`, marca todos os clientes como não finalizados e repete a busca por um cliente cuja `Need` inteira caiba em `Work`. Quando encontra, soma a `Allocation` desse cliente a `Work` e o marca como finalizado. O estado é seguro se todos terminarem.

## Tratamento de erros

Antes de executar qualquer comando, o programa valida:

- a quantidade de argumentos e cada argumento como inteiro positivo;
- o formato do `customer.txt` e o número de colunas;
- o formato do `commands.txt`, o comando, o índice do cliente e a quantidade de valores por recurso.

Em qualquer um desses casos, imprime uma mensagem coerente em `stderr` e encerra com código diferente de zero. A falha de abertura do `result.txt` também é tratada.

## Estrutura do projeto

| Arquivo | Responsabilidade |
|:--|:--|
| `banker.c` | implementação completa: parsing, estrutura `Customer`, comandos, teste de segurança, execução e `main` |
| `Makefile` | compila `banker.c` em `banker.o` e liga o executável `banker`; `make clean` remove os dois |
| `customer.txt` | entrada de exemplo com 5 clientes e 3 recursos |
| `commands.txt` | comandos de exemplo |
| `result.txt` | saída esperada para o exemplo |
| `evidencias.log` | log das sessões de teste |
| `gnfr.md` | relatório da implementação |

## Testes

Compilar do zero e rodar o exemplo:

```sh
make clean
make
```

Para comparar a saída com a referência sem sobrescrever o `result.txt` do projeto:

```sh
mkdir -p /tmp/banker-check
cp banker customer.txt commands.txt /tmp/banker-check/
cd /tmp/banker-check
./banker 10 5 7
cmp /caminho/do/projeto/result.txt result.txt
```

O `cmp` não imprime nada quando os arquivos são idênticos.

Valgrind no exemplo:

```sh
valgrind --leak-check=full --show-leak-kinds=all ./banker 10 5 7
```

Os testes de erro e as sessões completas estão em `evidencias.log`. O relatório em `gnfr.md` descreve as decisões e as evidências.

## Limitações conhecidas

- `RL` não valida se a liberação é maior do que a `Allocation` do cliente. O enunciado não define mensagem para esse caso, então o programa aplica os valores do arquivo. Uma liberação maior deixa a `Allocation` negativa, soma ao `Available` recursos que o cliente nunca teve e pode elevar o `Need` acima do `Maximum`. Com isso, o teste de segurança pode considerar seguro um estado impossível.
- A tabela usa larguras fixas de 10 e 12 caracteres para as colunas, iguais às do `result.txt` de referência. Vetores muito longos podem desalinhar as colunas.
