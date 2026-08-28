# Analisador Léxico — MiniC

## Como compilar

```
gcc -Wall -Wextra -pedantic -std=c11 minilexer.c -o minilexer
```

Compila liso, sem nenhum warning (já testei várias vezes pra garantir).

## Como rodar

Linux/macOS (WSL também):

```
./minilexer caminho/para/arquivo.mc
```

Windows:

```
.\minilexer.exe caminho\para\arquivo.mc
```

Se você esquecer de passar o arquivo, ele te avisa:

```
Uso: minilexer <arquivo-fonte>
```

E se o arquivo não existir ou não abrir, ele fala o erro e sai sem travar
nem nada.

## Como eu pensei pra implementar isso

- **Leitura do arquivo:** em vez de ficar lendo caractere por caractere
  direto do disco com `fgetc`/`ungetc` (o que dá mais dor de cabeça pra
  controlar linha/coluna quando você precisa "voltar" um caractere), eu
  carreguei o arquivo inteiro pra um buffer na memória de uma vez só
  (função `lerArquivo`) e fico andando por esse buffer com um índice. Fica
  bem mais fácil espiar o próximo caractere sem bagunçar nada.
- **Linha e coluna:** só tem um lugar no código que realmente "consome" um
  caractere, que é a função `avancar`. Ali eu decido se incrementa linha
  (quando é `\n`) ou coluna. Assim eu não corro o risco de atualizar errado
  em algum canto esquecido do código.
- **Números:** pra não ficar fazendo um monte de `if` diferente pra cada
  caso de número (inteiro, real, com erro...), eu simplesmente deixo o
  scanner "comer" toda a sequência de dígitos e pontos que aparecer na
  frente, e só DEPOIS eu olho quantos pontos apareceram pra decidir o que é:
  0 pontos = inteiro, 1 ponto (sem terminar em ponto) = real, qualquer outra
  coisa (tipo `12.` ou `1.2.3`) = erro. Resolveu os dois casos malucos do
  enunciado com a mesma lógica, sem precisar tratar cada um na mão.
- **Literal de caractere (tipo `'a'`):** eu leio tudo que tiver entre as
  aspas simples até achar a aspa de fechamento, quebrar linha ou acabar o
  arquivo (o que vier primeiro). Depois eu olho quantos caracteres ficaram
  no meio: se foi zero, é vazio (erro); se foi mais de um, também é erro;
  se não achou a aspa de fechamento, é erro de "não terminado".
- **Operadores tipo `==`, `!=`, `<=` etc:** sempre checo os de 2 caracteres
  ANTES dos de 1 caractere. Se eu checasse `=` primeiro, nunca ia conseguir
  formar `==`, porque ia parar no primeiro `=` e ler dois tokens separados
  em vez de um só.
- **Erro não trava o programa:** se aparece um caractere estranho tipo `@`
  ou `#`, eu registro o erro e sigo lendo o resto do arquivo numa boa — não
  para tudo no primeiro erro que encontra.
- **Não estoura buffer:** o lexema fica guardado num vetor de tamanho fixo
  (64 posições). Testei com um identificador gigante de propósito (73
  caracteres) só pra confirmar que o código não escreve além do limite do
  vetor — ele simplesmente para de copiar caractere quando o buffer enche,
  mas continua "lendo" o resto do identificador do arquivo normalmente.

## O que ficou de fora (de propósito)

- Comentário de bloco `/* ... */` — não pedia no enunciado.
- String entre aspas duplas — também não pedia.
- Escape em literal de caractere tipo `'\n'` — o enunciado até fala que não
  precisa.
- Identificador com mais de 31 caracteres eu trato só como erro (não gero
  um token de identificador "cortado" junto) — achei mais limpo assim.

## Os testes

Tá tudo na pasta `testes/`, um arquivo `.mc` pra cada situação que o
enunciado pediu (palavra reservada, identificador válido, identificador
gigante, número inteiro, número real, número quebrado, todos os
operadores, todos os delimitadores, comentário, literal de caractere bom e
ruim, caractere inválido, arquivo vazio, arquivo só com espaço/comentário e
um caso sem espaço nenhum tipo `if(x>=10){x=x+1;}`). As saídas de cada um
tão salvas em `evidencias/saidas_completas.txt`, pra não precisar rodar
tudo de novo pra conferir.

O relatório com a explicação mais teórica e as perguntas do enunciado
respondidas tá no [`RELATORIO.md`](./RELATORIO.md).
