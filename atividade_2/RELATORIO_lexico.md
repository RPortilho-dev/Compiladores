# Relatório — Analisador Léxico MiniC

**Estudante:** Renato Portilho
**RA** 24101301
**Disciplina:** Compiladores — 6º semestre
**Atividade:** Fazer um analisador léxico do zero, em C, pra uma linguagem
inventada chamada MiniC

## 1. O que é análise léxica (resumindo com minhas palavras)

Análise léxica é basicamente a primeira etapa de um compilador/interpretador:
é onde o código-fonte, que é só um monte de caractere solto, vira uma lista
de "pedaços com significado" chamados tokens — tipo palavra reservada,
identificador, número, operador, delimitador etc. O texto original que virou
aquele token se chama lexema. O legal é que nessa etapa o programa nem se
importa se o código faz sentido gramatical — ele só separa e classifica.
Quem vai checar se a gramática tá certa é o parser, que vem depois.

## 2. Como eu resolvi o problema

Fiz um scanner de passada única, meio no estilo "vai comendo o máximo que
puder antes de decidir o que é" ("maximal munch", mas na prática é só: em 
vez de ficar checando caractere a caractere toda hora sejá formou um token,
eu deixo ele consumir tudo que ainda faz sentido pro tipo
que eu tô tentando reconhecer, e só decido a categoria no final).

Resumo do fluxo:

1. Carrego o arquivo inteiro pra memória de uma vez.
2. Fico andando por esse texto com um índice, e uso duas funções (`espiar` e
   `espiarProximo`) pra dar uma olhada no caractere atual e no próximo sem
   precisar "consumir" eles — isso é essencial pra decidir coisas tipo se
   `/` é divisão ou início de comentário, ou se `=` é sozinho ou é `==`.
3. A cada token que eu vou pegar, primeiro eu pulo espaço em branco e
   comentário `//`. Depois eu olho o caractere que sobrou e decido pra qual
   "reconhecedor" mandar ele: identificador/palavra reservada, número,
   literal de caractere, operador ou delimitador. Se não encaixar em nada
   disso, vira erro léxico e o programa CONTINUA lendo o resto (não trava).
4. Cada token guarda a linha e a coluna de onde ele começou — isso é
   atualizado só em um lugar do código (função `avancar`), então não tem
   risco de eu esquecer de atualizar em algum canto.

## 3. Exemplo de entrada e saída

Entrada (`testes/14_sem_espacos.mc`), um código bem colado, sem espaço:

```
if(x>=10){x=x+1;}
```

Saída (só um pedaço):

```
1:1  | PALAVRA_RESERVADA | if
1:3  | DELIMITADOR       | (
1:4  | IDENTIFICADOR     | x
1:5  | OPERADOR          | >=
1:7  | NUMERO_INTEIRO    | 10
```

Agora um exemplo com erro (`testes/11_caracteres_invalidos.mc`):

```
int x = 10 @ 2;
```

Saída:

```
1:9  | NUMERO_INTEIRO    | 10
ERRO_LEXICO | linha 1, coluna 12 | simbolo invalido: @
1:14 | NUMERO_INTEIRO    | 2
```

Repara que depois do `@` ele continuou de boa, pegou o `2` e o `;` que vinham
depois — não travou nem parou a análise, que era um dos requisitos.

## 4. As partes que mais me deram trabalho

- **Número quebrado (tipo `12.` ou `1.2.3`):** no começo eu pensei em tratar
  cada caso separado, mas ficou mais simples deixar o código "comer" toda a
  sequência de dígito+ponto que aparecer, e só no final contar quantos
  pontos apareceram pra decidir se é inteiro, real ou erro. Isso resolveu os
  dois exemplos malucos do enunciado ao mesmo tempo, sem precisar escrever
  um `if` pra cada padrão diferente.
- **Literal de caractere tipo `'x'`:** tinha que diferenciar `''` (vazio),
  `'ab'` (tem letra demais) e `'x` (esqueceu de fechar a aspa). Pra isso eu
  leio tudo que tiver dentro das aspas até achar o fechamento (ou a linha
  acabar), e só depois conto quantos caracteres sobraram no meio pra saber
  qual dos três problemas é.
- **Não estourar o vetor com identificador gigante:** o teste pedia um
  identificador de mais de 31 caracteres, e eu tinha que continuar lendo ele
  inteiro do arquivo (pra não bagunçar a leitura do próximo token), mas sem
  deixar o programa escrever passado do tamanho do vetor que guarda o
  lexema. Resolvi separando dois contadores: um que conta quantos
  caracteres eu realmente li do arquivo (pra saber se passou de 31) e outro
  que conta quantos eu de fato copiei pro vetor (que para de crescer quando
  o buffer enche).

## 5. Divisão do trabalho

Fiz sozinho, é atividade individual.

## 6. Respondendo as perguntas do enunciado

**1. Por que palavra reservada e identificador começam sendo reconhecidos
pela mesma regra?**
Porque olhando só os caracteres, `if` e uma variável qualquer tipo `idade`
são a mesma coisa: começam com letra e seguem com letra/número/underline.
A diferença só existe depois, quando eu comparo o texto formado com a
listinha de palavras reservadas (`int`, `float`, `if`, etc). Então primeiro
eu deixo ler o "nome" inteiro normal, e só no final decido se é palavra
reservada ou identificador comum.

**2. Por que tem que checar operador de 2 caracteres antes do de 1?**
Porque se eu checasse `=` primeiro, ele já ia fechar o token ali mesmo e
nunca ia dar chance de virar `==`. Aí em vez de um token `==` eu ia gerar
dois tokens `=` `=` separados, o que tá errado. Checando os combos de dois
caracteres primeiro (dando uma espiadinha no próximo caractere antes de
decidir), garanto que sempre pego o token "maior" possível.

**3. Diferença entre erro léxico e erro sintático?**
Erro léxico é quando um pedaço do texto nem forma um token válido — tipo o
`@` sozinho, que não é nada conhecido da linguagem. Erro sintático é
diferente: os tokens até são válidos individualmente, mas a ordem/estrutura
deles não bate com a gramática. Tipo `int = 10;` — `int`, `=`, `10`, `;` são
todos tokens ok, só que falta o nome da variável no meio. O léxico não
enxerga isso porque ele só olha token por token, não a "frase" inteira.

**4. Por que o analisador tem que continuar depois de achar um símbolo
inválido?**
Porque senão ia ser um saco pra debugar: você corrige um erro, roda de novo,
acha o próximo erro, corrige, roda de novo... Já que o scanner passa o
arquivo inteiro de uma vez, faz muito mais sentido ele já te falar TODOS os
erros que achou de uma vez só, e não só o primeiro. No código isso é o
`obterProximoToken` gerando um token de erro pra cada problema e seguindo em
frente normalmente pro próximo caractere.

**5. Qual o risco de não checar o limite do vetor do lexema?**
Se eu deixasse um identificador gigante escrever livre no vetor, ele ia
passar do tamanho reservado na memória (isso se chama buffer overflow) e
começar a sobrescrever memória que não é dele — de outra variável, de outra
estrutura, sei lá. Em C isso é comportamento indefinido: pode travar o
programa, corromper dado sem avisar nada, ou até virar brecha de segurança
em casos mais sérios. No meu código, a função `guardarCaractere` sempre
confere se ainda cabe no vetor antes de escrever, e testei isso na prática
com um identificador de 73 caracteres — não deu problema nenhum.

**6. Em `int = 10;`, onde esse erro seria pego, já que todo caractere forma
token válido?**
Só lá na análise sintática mesmo. Pro léxico, `int`, `=`, `10` e `;` são
tokens 100% válidos, sem erro nenhum. O problema é que falta um
identificador entre `int` e `=`, e checar isso é trabalho do parser, que
entende a gramática da linguagem — o léxico não tem esse contexto.

**7. Qual a vantagem de guardar os identificadores numa tabela de
símbolos?**
Ajudaria muito nas próximas etapas do compilador: dá pra checar se a
variável foi declarada antes de usar, evitar declarar a mesma variável duas
vezes, guardar o tipo dela pra checar depois se as contas fazem sentido
(tipo somar int com float), e também economiza memória porque você guarda o
nome da variável uma vez só na tabela, em vez de duplicar o texto toda vez
que ela aparece no código.

## 7. Fechando

Consegui cobrir tudo que o enunciado pedia: reconhece todas as categorias de
token, calcula linha/coluna certinho, ignora espaço e comentário, detecta
erro léxico e continua rodando mesmo assim, e não estoura vetor em nenhum
canto. Rodei o exemplo do próprio enunciado e bateu 100% (26 tokens, 0
erros), e os 15 testes que eu criei confirmam que os casos de erro e os
casos "de fronteira" (tipo arquivo vazio ou código colado sem espaço)
também funcionam do jeito esperado.
