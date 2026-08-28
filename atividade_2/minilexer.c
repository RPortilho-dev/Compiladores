/*
 * minilexer.c
 *
 * Analisador lexico (scanner) para a linguagem ficticia MiniC.
 * Disciplina de Compiladores.
 *
 * Le um arquivo-fonte MiniC caractere a caractere (a partir de um buffer
 * carregado inteiramente em memoria) e imprime, um por linha, cada token
 * reconhecido no formato:
 *
 *     LINHA:COLUNA | CATEGORIA | LEXEMA
 *
 * Erros lexicos sao reportados e a analise continua ate o fim do arquivo.
 *
 * Compilacao:
 *     gcc -Wall -Wextra -pedantic -std=c11 minilexer.c -o minilexer
 *
 * Uso:
 *     ./minilexer arquivo.mc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LEXEMA 64
#define MAX_IDENTIFICADOR 31

/* ------------------------------------------------------------------ */
/* Tipos de token                                                      */
/* ------------------------------------------------------------------ */

typedef enum {
    TOKEN_PALAVRA_RESERVADA,
    TOKEN_IDENTIFICADOR,
    TOKEN_NUMERO_INTEIRO,
    TOKEN_NUMERO_REAL,
    TOKEN_LITERAL_CARACTERE,
    TOKEN_OPERADOR,
    TOKEN_DELIMITADOR,
    TOKEN_ERRO
} TipoToken;

typedef struct {
    TipoToken tipo;
    char lexema[MAX_LEXEMA];
    int linha;
    int coluna;
    char mensagemErro[200]; /* usado apenas quando tipo == TOKEN_ERRO */
} Token;

/* ------------------------------------------------------------------ */
/* Estado do lexer                                                     */
/* ------------------------------------------------------------------ */

typedef struct {
    char *fonte;      /* conteudo inteiro do arquivo-fonte, terminado em \0 */
    long tamanho;     /* quantidade de caracteres em 'fonte' */
    long pos;         /* indice do proximo caractere a ser lido */
    int linha;        /* linha do proximo caractere a ser lido (1-based) */
    int coluna;       /* coluna do proximo caractere a ser lido (1-based) */
    int totalTokens;
    int totalErros;
} Lexer;

/* ------------------------------------------------------------------ */
/* Palavras reservadas                                                 */
/* ------------------------------------------------------------------ */

static const char *PALAVRAS_RESERVADAS[] = {
    "int", "float", "char", "if", "else", "while", "return", "print"
};
static const int QTD_PALAVRAS_RESERVADAS = 8;

int ehPalavraReservada(const char *lexema) {
    int i;
    for (i = 0; i < QTD_PALAVRAS_RESERVADAS; i++) {
        if (strcmp(lexema, PALAVRAS_RESERVADAS[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

/* ------------------------------------------------------------------ */
/* Classificacao de caracteres                                         */
/* ------------------------------------------------------------------ */

int ehInicioIdentificador(char c) {
    return isalpha((unsigned char)c) || c == '_';
}

int ehParteIdentificador(char c) {
    return isalnum((unsigned char)c) || c == '_';
}

int ehDelimitador(char c) {
    return c == '(' || c == ')' || c == '{' || c == '}' ||
           c == '[' || c == ']' || c == ';' || c == ',';
}

/* ------------------------------------------------------------------ */
/* Nomes das categorias (para impressao)                               */
/* ------------------------------------------------------------------ */

const char *nomeDoToken(TipoToken tipo) {
    switch (tipo) {
        case TOKEN_PALAVRA_RESERVADA: return "PALAVRA_RESERVADA";
        case TOKEN_IDENTIFICADOR:     return "IDENTIFICADOR";
        case TOKEN_NUMERO_INTEIRO:    return "NUMERO_INTEIRO";
        case TOKEN_NUMERO_REAL:       return "NUMERO_REAL";
        case TOKEN_LITERAL_CARACTERE: return "LITERAL_CARACTERE";
        case TOKEN_OPERADOR:          return "OPERADOR";
        case TOKEN_DELIMITADOR:       return "DELIMITADOR";
        case TOKEN_ERRO:              return "ERRO_LEXICO";
    }
    return "DESCONHECIDO";
}

/* ------------------------------------------------------------------ */
/* Leitura do arquivo-fonte para memoria                               */
/* ------------------------------------------------------------------ */

char *lerArquivo(const char *caminho, long *tamanhoLido) {
    FILE *arquivo = fopen(caminho, "rb");
    if (!arquivo) {
        return NULL;
    }

    if (fseek(arquivo, 0, SEEK_END) != 0) {
        fclose(arquivo);
        return NULL;
    }
    long tamanho = ftell(arquivo);
    if (tamanho < 0) {
        fclose(arquivo);
        return NULL;
    }
    rewind(arquivo);

    char *buffer = (char *)malloc((size_t)tamanho + 1);
    if (!buffer) {
        fclose(arquivo);
        return NULL;
    }

    size_t lidos = fread(buffer, 1, (size_t)tamanho, arquivo);
    buffer[lidos] = '\0';

    fclose(arquivo);
    *tamanhoLido = (long)lidos;
    return buffer;
}

/* ------------------------------------------------------------------ */
/* Funcoes auxiliares de leitura de caractere com controle de posicao  */
/* ------------------------------------------------------------------ */

/* Retorna o caractere na posicao atual sem avancar. */
char espiar(const Lexer *lex) {
    if (lex->pos >= lex->tamanho) return '\0';
    return lex->fonte[lex->pos];
}

/* Retorna o caractere logo apos a posicao atual, sem avancar. */
char espiarProximo(const Lexer *lex) {
    if (lex->pos + 1 >= lex->tamanho) return '\0';
    return lex->fonte[lex->pos + 1];
}

/* Consome o caractere atual, avancando posicao/linha/coluna. */
char avancar(Lexer *lex) {
    char c = espiar(lex);
    if (c == '\0') return c;

    lex->pos++;
    if (c == '\n') {
        lex->linha++;
        lex->coluna = 1;
    } else {
        lex->coluna++;
    }
    return c;
}

/* ------------------------------------------------------------------ */
/* Impressao de um token                                               */
/* ------------------------------------------------------------------ */

void imprimirToken(const Token *token) {
    char posicao[24];
    snprintf(posicao, sizeof(posicao), "%d:%d", token->linha, token->coluna);

    if (token->tipo == TOKEN_ERRO) {
        printf("ERRO_LEXICO | linha %d, coluna %d | %s\n",
               token->linha, token->coluna, token->mensagemErro);
    } else {
        printf("%-5s| %-18s| %s\n", posicao, nomeDoToken(token->tipo), token->lexema);
    }
}

/* ------------------------------------------------------------------ */
/* Reconhecimento de tokens                                            */
/* ------------------------------------------------------------------ */

/* Ignora espacos em branco e comentarios de uma linha ("//..."). 
 * Retorna 1 se algo foi ignorado (o chamador deve tentar novamente),
 * ou 0 se o caractere atual ja faz parte de um token relevante. */
int ignorarEspacosEComentarios(Lexer *lex) {
    int ignorouAlgo = 0;

    for (;;) {
        char c = espiar(lex);

        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            avancar(lex);
            ignorouAlgo = 1;
            continue;
        }

        if (c == '/' && espiarProximo(lex) == '/') {
            avancar(lex); /* consome primeiro '/' */
            avancar(lex); /* consome segundo '/' */
            while (espiar(lex) != '\0' && espiar(lex) != '\n') {
                avancar(lex);
            }
            ignorouAlgo = 1;
            continue;
        }

        break;
    }

    return ignorouAlgo;
}

/* Copia ate (MAX_LEXEMA - 1) caracteres para o lexema, sempre terminando
 * a string com '\0' e nunca escrevendo fora dos limites do vetor. */
void guardarCaractere(char *lexema, int *tamanhoAtual, char c) {
    if (*tamanhoAtual < MAX_LEXEMA - 1) {
        lexema[*tamanhoAtual] = c;
        (*tamanhoAtual)++;
    }
    /* Se o lexema estourar o buffer, os caracteres excedentes sao
     * apenas descartados da representacao textual, mas o caractere
     * de origem no arquivo sempre eh consumido pelo chamador — nao
     * ha leitura nem escrita fora dos limites do vetor. */
}

/* Reconhece identificador ou palavra reservada. */
void reconhecerIdentificador(Lexer *lex, Token *token) {
    int tamanho = 0;
    long quantidadeReal = 0; /* quantidade de caracteres realmente lidos */

    while (ehParteIdentificador(espiar(lex))) {
        char c = avancar(lex);
        guardarCaractere(token->lexema, &tamanho, c);
        quantidadeReal++;
    }
    token->lexema[tamanho] = '\0';

    if (quantidadeReal > MAX_IDENTIFICADOR) {
        token->tipo = TOKEN_ERRO;
        snprintf(token->mensagemErro, sizeof(token->mensagemErro),
                 "identificador '%s...' excede %d caracteres (%ld lidos)",
                 token->lexema, MAX_IDENTIFICADOR, quantidadeReal);
        return;
    }

    if (ehPalavraReservada(token->lexema)) {
        token->tipo = TOKEN_PALAVRA_RESERVADA;
    } else {
        token->tipo = TOKEN_IDENTIFICADOR;
    }
}

/* Reconhece numero inteiro ou real (ou numero malformado). Usa a regra
 * de "maximal munch" sobre digitos e pontos: consome toda a sequencia
 * contigua de [0-9.] e depois classifica pelo padrao resultante. */
void reconhecerNumero(Lexer *lex, Token *token) {
    int tamanho = 0;
    int quantidadePontos = 0;
    int terminaComPonto = 0;

    while (isdigit((unsigned char)espiar(lex)) || espiar(lex) == '.') {
        char c = avancar(lex);
        if (c == '.') {
            quantidadePontos++;
            terminaComPonto = 1;
        } else {
            terminaComPonto = 0;
        }
        guardarCaractere(token->lexema, &tamanho, c);
    }
    token->lexema[tamanho] = '\0';

    if (quantidadePontos == 0) {
        token->tipo = TOKEN_NUMERO_INTEIRO;
    } else if (quantidadePontos == 1 && !terminaComPonto) {
        token->tipo = TOKEN_NUMERO_REAL;
    } else {
        token->tipo = TOKEN_ERRO;
        snprintf(token->mensagemErro, sizeof(token->mensagemErro),
                 "numero malformado: %s", token->lexema);
    }
}

/* Reconhece literal de caractere: 'x'. Trata vazio, multiplos
 * caracteres e literal nao terminado como erro lexico. */
void reconhecerLiteralCaractere(Lexer *lex, Token *token) {
    int tamanho = 0;
    int quantidadeConteudo = 0;
    int terminado = 0;

    char aspaAbertura = avancar(lex); /* consome ' */
    guardarCaractere(token->lexema, &tamanho, aspaAbertura);

    while (espiar(lex) != '\0' && espiar(lex) != '\n' && espiar(lex) != '\'') {
        char c = avancar(lex);
        guardarCaractere(token->lexema, &tamanho, c);
        quantidadeConteudo++;
    }

    if (espiar(lex) == '\'') {
        char aspaFechamento = avancar(lex);
        guardarCaractere(token->lexema, &tamanho, aspaFechamento);
        terminado = 1;
    }
    token->lexema[tamanho] = '\0';

    if (!terminado) {
        token->tipo = TOKEN_ERRO;
        snprintf(token->mensagemErro, sizeof(token->mensagemErro),
                 "literal de caractere nao terminado: %s", token->lexema);
    } else if (quantidadeConteudo == 0) {
        token->tipo = TOKEN_ERRO;
        snprintf(token->mensagemErro, sizeof(token->mensagemErro),
                 "literal de caractere vazio: %s", token->lexema);
    } else if (quantidadeConteudo > 1) {
        token->tipo = TOKEN_ERRO;
        snprintf(token->mensagemErro, sizeof(token->mensagemErro),
                 "literal de caractere com mais de um caractere: %s", token->lexema);
    } else {
        token->tipo = TOKEN_LITERAL_CARACTERE;
    }
}

/* Reconhece operadores de um ou dois caracteres. Retorna 1 se
 * reconheceu algo, 0 caso o caractere atual nao seja um operador. */
int reconhecerOperador(Lexer *lex, Token *token) {
    char c = espiar(lex);
    char proximo = espiarProximo(lex);
    int tamanho = 0;

    /* Operadores de dois caracteres — verificados ANTES dos de um
     * caractere, para que "==" nao seja lido como dois tokens "=". */
    if ((c == '=' && proximo == '=') ||
        (c == '!' && proximo == '=') ||
        (c == '<' && proximo == '=') ||
        (c == '>' && proximo == '=') ||
        (c == '&' && proximo == '&') ||
        (c == '|' && proximo == '|')) {
        char c1 = avancar(lex);
        char c2 = avancar(lex);
        guardarCaractere(token->lexema, &tamanho, c1);
        guardarCaractere(token->lexema, &tamanho, c2);
        token->lexema[tamanho] = '\0';
        token->tipo = TOKEN_OPERADOR;
        return 1;
    }

    if (c == '+' || c == '-' || c == '*' || c == '/' || c == '%' ||
        c == '=' || c == '<' || c == '>' || c == '!') {
        char c1 = avancar(lex);
        guardarCaractere(token->lexema, &tamanho, c1);
        token->lexema[tamanho] = '\0';
        token->tipo = TOKEN_OPERADOR;
        return 1;
    }

    return 0;
}

/* Obtem o proximo token do arquivo-fonte. Retorna 0 quando o fim do
 * arquivo eh alcancado (sem gerar token). */
int obterProximoToken(Lexer *lex, Token *token) {
    ignorarEspacosEComentarios(lex);

    char c = espiar(lex);
    if (c == '\0') {
        return 0; /* fim de arquivo */
    }

    token->linha = lex->linha;
    token->coluna = lex->coluna;
    token->mensagemErro[0] = '\0';

    if (ehInicioIdentificador(c)) {
        reconhecerIdentificador(lex, token);
        return 1;
    }

    if (isdigit((unsigned char)c)) {
        reconhecerNumero(lex, token);
        return 1;
    }

    if (c == '\'') {
        reconhecerLiteralCaractere(lex, token);
        return 1;
    }

    if (reconhecerOperador(lex, token)) {
        return 1;
    }

    if (ehDelimitador(c)) {
        int tamanho = 0;
        char consumido = avancar(lex);
        guardarCaractere(token->lexema, &tamanho, consumido);
        token->lexema[tamanho] = '\0';
        token->tipo = TOKEN_DELIMITADOR;
        return 1;
    }

    /* Caractere invalido: registra erro e consome apenas ele, para
     * que a analise possa continuar no proximo caractere. */
    {
        int tamanho = 0;
        char invalido = avancar(lex);
        guardarCaractere(token->lexema, &tamanho, invalido);
        token->lexema[tamanho] = '\0';
        token->tipo = TOKEN_ERRO;
        snprintf(token->mensagemErro, sizeof(token->mensagemErro),
                 "simbolo invalido: %s", token->lexema);
    }
    return 1;
}

/* ------------------------------------------------------------------ */
/* main                                                                 */
/* ------------------------------------------------------------------ */

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: minilexer <arquivo-fonte>\n");
        return EXIT_FAILURE;
    }

    long tamanho = 0;
    char *fonte = lerArquivo(argv[1], &tamanho);
    if (!fonte) {
        fprintf(stderr, "Erro: nao foi possivel abrir o arquivo '%s'\n", argv[1]);
        return EXIT_FAILURE;
    }

    Lexer lex;
    lex.fonte = fonte;
    lex.tamanho = tamanho;
    lex.pos = 0;
    lex.linha = 1;
    lex.coluna = 1;
    lex.totalTokens = 0;
    lex.totalErros = 0;

    Token token;
    while (obterProximoToken(&lex, &token)) {
        imprimirToken(&token);
        if (token.tipo == TOKEN_ERRO) {
            lex.totalErros++;
        } else {
            lex.totalTokens++;
        }
    }

    printf("\nTotal de tokens: %d\n", lex.totalTokens);
    printf("Total de erros lexicos: %d\n", lex.totalErros);

    free(fonte);
    return EXIT_SUCCESS;
}
