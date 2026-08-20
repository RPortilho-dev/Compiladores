# Mini C Compiler — Atividade Individual (Análise + Melhoria)

> **Este repositório é uma atividade individual da disciplina de Compiladores.**
> O projeto original é do autor [ironrinox](https://github.com/ironrinox/mini-c-compiler) (licença MIT, mantida em `LICENSE`).
> Meu trabalho aqui foi analisar a arquitetura, executar uma bateria de testes (léxicos, sintáticos e semânticos) e implementar uma melhoria individual — **suporte a comentários de uma linha (`//`)** no lexer.
>
> - 📄 Relatório completo da análise: [`RELATORIO.md`](./RELATORIO.md)
> - 🧪 Casos de teste usados: pasta [`testes/`](./testes)
> - 🔧 Melhoria implementada: comentários de linha (`//`) em `src/lexer.c`

---

# Mini C Compiler

## Overview

This project is a **minimal compiler** implemented in **C** for a small, toy programming language. The goal is **educational**: to understand the inner workings of a compiler from scratch. Even if you have experience with web development, scripting, or standard application programming, this project will guide you through the core concepts that transform **plain text code** into something a computer can execute.  

This repository is designed so that each part is **understandable and modular**, making it easy to extend with new features like conditionals, loops, and more complex operations.  

⚠️ Note: Currently, the project functions as an interpreter. Future versions aim to evolve it into a full compiler capable of generating machine code or bytecode.

---

## Project Status

⚠️ **Work in Progress (WIP)**

This project is **currently under development**. While the interpreter is fully functional for simple expressions and variable assignments, many features are still planned, including:
- More complex statements, loops, and advanced error handling.
- Future compilation phases like code generation, optimization, and assembly output.


![status](https://img.shields.io/badge/status-in%20progress-yellow)

---

## Tools and Language

The project is written entirely in **C**, chosen for its clarity and direct mapping to low-level programming concepts.  

No external libraries are required – the entire compiler is implemented using **plain C**, including:  
- Data structures for tokens, AST nodes, and symbol tables  
- String parsing and manipulation  
- Recursive functions for parsing and interpreting code

You will only need a **C compiler** (e.g., GCC or Clang) to build and run the project.

---

## Compiler Phases

The compiler is divided into several key **phases**, each responsible for a specific task. Understanding these phases is essential to grasp how modern compilers work:

1. **Lexical Analysis (Lexer / Tokenizer)**  
   - Converts raw text code into **tokens**, the smallest meaningful elements in a program (numbers, operators, keywords, identifiers).  
   - Example: the code `let x = 5 + 3;` is transformed into tokens: `[LET] [IDENTIFIER:x] [EQUAL] [NUMBER:5] [PLUS] [NUMBER:3] [SEMICOLON]`.

2. **Parsing**  
   - Organizes tokens into a **structured representation** called an **Abstract Syntax Tree (AST)**.  
   - The AST captures the **hierarchy and order of operations** in the program.  
   - Example: `5 + 3` becomes a tree node `PLUS` with children `5` and `3`.

3. **Semantic Analysis**  
   - Checks that the program makes sense: variables are declared before use, operations are valid, etc.  
   - Builds a **symbol table** to track variable names and their values.

4. **Code Generation / Interpretation**  
   - Converts the AST into something executable.  
   - In this project, we use an **interpreter** approach: the AST is traversed and executed directly, computing results and printing output.

5. **(Planned Extensions / Compilation)**  
   - Future phases may include optimization, bytecode generation, or outputting assembly code, transforming the interpreter into a true compiler. 

---

## Why This Project?

This project is perfect for developers who want to:  
- Understand **what happens under the hood** when code is compiled or interpreted.  
- Learn **core concepts of programming languages** and their execution.  
- Gain experience in **low-level programming** while still producing working, visible results.  

Even if your background is in web development, Python, or app programming, completing this project will give you insights into **how your code is transformed into machine-readable instructions**.

---

## Requirements

To compile and run the project, you need:

- A **C compiler** (GCC, Clang, or compatible).  
- A terminal or command prompt to execute compiled binaries.  
- No additional libraries are required.

---

## How to Build and Run

The compiler reads **source code from a file**, which must be specified as a **command-line argument** when executing the program.  

1. Open a terminal in the project directory.  
2. Compile the source files:
```bash
gcc src/main.c src/lexer.c src/parser.c src/interpreter.c src/utils.c -Iinclude -o mini-c.exe
```
3. Run the compiler/interpreter with a source file:
```bash
./mini-c.exe examples/test.txt
```
- Replace examples/test.txt with the path to your own source code file.
- The program will read the file, tokenize, parse, and interpret it.

Example code supported currently **(examples/test.txt)**:
```c
let x = 5 + 3;
let y = 1 + 1;
print(x + y);
```
Output:
```plaintext
10
```
> ⚠️ Note: The function that reads the source file is implemented in `src/utils.c`.

---

## Notes

- Only simple expressions and variable assignments are supported.  
- Statements must end with a semicolon `;`.  
- Parentheses `(` and `)` can be used for grouping expressions.  
- Accessing undefined variables or dividing by zero will terminate execution with an error message.
