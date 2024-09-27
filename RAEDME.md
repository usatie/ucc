# UCC - A Simple C Compiler

UCC is a simple C compiler written in C. It supports a subset of the C language and generates x86-64 assembly code. This project is inspired by the [compilerbook](https://www.sigbus.info/compilerbook) and aims to learn how to build a compiler from scratch.

## Features

- Tokenization of C source code
- Parsing of C syntax into an Abstract Syntax Tree (AST)
- Semantic analysis and type checking
- Code generation for x86-64 assembly
- Support for basic C constructs: variables, functions, conditionals, loops, and more

## Directory Structure

- `src/`: Contains the source code for the compiler
  - `main.c`: Entry point of the compiler
  - `tokenize.c`: Tokenizer implementation
  - `parse.c`: Parser implementation
  - `codegen.c`: Code generator implementation
  - `type.c`: Type system implementation
- `include/`: Contains header files
  - `ucc.h`: Header file with type definitions and function declarations
- `test.sh`: Script for running tests
- `Dockerfile`: Docker setup for the project
- `.gitignore`: Git ignore file
- `.vimrc`: Vim configuration file

## Getting Started

### Prerequisites

- GCC (GNU Compiler Collection)
- Make
- Docker (optional, for running in a containerized environment)

### Building the Compiler

To build the compiler, run the following command:

```sh
make
```

### Running the Compiler

To compile a C file using UCC, run:

```sh
./ucc <source-file.c>
```

This will generate an assembly file `tmp.s` which can be assembled and linked using GCC:

```sh
gcc -o <output-file> tmp.s
```

### Running Tests

To run the provided tests, execute:

```sh
make test
```

This will run the `test.sh` script which compiles and runs various test cases to ensure the compiler works correctly.

### Using Docker

You can use Docker to build and test the compiler in a containerized environment. To build and test using Docker, run:

```sh
make docker-test
```

To start an interactive shell in the Docker container, run:

```sh
make run
```

## Code Overview

### Tokenization

The tokenizer (`tokenize.c`) converts the input C source code into a stream of tokens. Each token represents a meaningful element of the source code, such as keywords, identifiers, operators, and literals.

### Parsing

The parser (`parse.c`) takes the stream of tokens and constructs an Abstract Syntax Tree (AST) representing the structure of the C program. It handles various C constructs like expressions, statements, and function declarations.

### Code Generation

The code generator (`codegen.c`) traverses the AST and generates x86-64 assembly code. It handles the translation of C constructs into their corresponding assembly instructions.

### Type System

The type system (`type.c`) provides type checking and type inference for the C program. It ensures that operations are performed on compatible types and manages type conversions.

## Acknowledgements

This project is inspired by, and follows the structure of, the [compilerbook](https://www.sigbus.info/compilerbook) by Rui Ueyama. To write this README, I used [gpt-4o](https://platform.openai.com/docs/models/gpt-4o) and [Cursor](https://cursor.sh/).