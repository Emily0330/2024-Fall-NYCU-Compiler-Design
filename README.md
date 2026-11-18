# NYCU Compiler Design (Fall 2024) | P Language Compiler

This repository contains my coursework for Introduction to Compiler Design, taught by Prof. Yi-Ping You at National Yang Ming Chiao Tung University (NYCU) in Fall 2024 (academic year 113, semester 1). The project builds a compiler for the P language in stages, from lexical analysis through RISC-V assembly generation.

## Assignments

| Assignment | Topic | Where to start |
| --- | --- | --- |
| [HW0](hw0-Emily0330/README.md) | Development environment | Course guide to Docker, Git, and Make |
| [HW1](hw1-Emily0330/README.md) | Lexical analysis | [scanner.l](hw1-Emily0330/src/scanner.l) defines the Flex rules for recognizing tokens |
| [HW2](hw2-Emily0330/README.md) | Syntax analysis | [parser.y](hw2-Emily0330/src/parser.y) defines the Bison grammar and operator precedence |
| [HW3](hw3-Emily0330/README.md) | Abstract syntax tree (AST) | [parser.y](hw3-Emily0330/src/parser.y) builds the tree using the [AST node classes](hw3-Emily0330/src/include/AST) |
| [HW4](hw4-Emily0330/README.md) | Semantic analysis | [SemanticAnalyzer.cpp](hw4-Emily0330/src/lib/sema/SemanticAnalyzer.cpp) checks scope and types and reports semantic errors |
| [HW5](hw5-Emily0330/README.md) | RISC-V code generation | [CodeGenerator.cpp](hw5-Emily0330/src/lib/codegen/CodeGenerator.cpp) generates assembly from the AST and symbol table |

Each assignment has its own directory. In HW1–HW5, `test/test_cases/` contains P language test inputs and `report/README.md` documents the implementation.

## Build and test

Start Docker in Linux container mode. To run HW5, open PowerShell at the repository root and run the following commands.

```powershell
cd hw5-Emily0330
docker run --rm -it --mount "type=bind,source=$($PWD.Path),target=/workspace" -w /workspace laiyt/compiler-s24-hw5:latest bash
```

Inside the container, build the compiler and run the tests.

```bash
make
make test
```

For HW1–HW4, run the same build and test commands from the corresponding assignment directory. Use `laiyt/<IMAGE_NAME>:latest`, where `IMAGE_NAME` comes from that assignment's Makefile. HW5 uses the RISC-V toolchain and Spike to run the generated programs.

## Using this repository as a reference

Start with the assignment specification, then pick a small test case and trace how that stage processes it. This repository is shared as a learning reference. Follow your semester's course rules and complete your assignments independently.

The assignment directories retain the original course specifications, starter files, and test data. See the `LICENSE` file in each directory for the original MIT license and copyright notice.
