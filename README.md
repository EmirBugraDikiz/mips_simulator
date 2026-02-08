# MIPS Assembler & CPU Simulator

This project is an educational implementation of a MIPS assembler and CPU simulator written in C.
Its primary goal is to gain a deep understanding of computer architecture and low-level system 
design by building an assembler and execution model from scratch. It is a self-learning journey.

The project is developed incrementally with a strong focus on correctness, modularity and memory safety.

---

## Features

- Data structure to handle and support dynamic line of input and length (Input Program)
- Error handling and debugging while developing the project to avoid invalid function argument.
- Two-pass assembler architecture
- Lexer and parser for MIPS assembly syntax
- Symbol table and intermediate representation (IR)
- Comprehensive error handling
- Modular project structure
- CMake-based build system
- Memory analysis using Valgrind and AddressSanitizer

---

## High-Level Architecture

The assembler is divided into independent modules

- **Lexer**: Tokenizes each raw assembly line input and put the results to a dynamic token data structure
- **Parser**: Converts each token array that acquired from each line into structured statements
- **Symbol Table**: Stores labels and corresponding address in phase "pass1"
- **Intermediate Representation**: Holds parsed instructions before encoding
- **Pass1**: Builds the symbol table and validates program structure
- **Pass2 (planned)**: Encodes instructions into machine code
- **CPU Sİmulator (planned)**: Executes instructions using a fetch-decode-execute cycle 


This seperation allows each stage of the pipeline to be developed, tested, and reasoned about independently.

---

# Design Decisions

### Why a Two-Pass Assembler?
A two-pass assembler design was chosen to correctly handle forward referenced labels and to 
seperate symbol resolution from instruction encoding. This mirrors real-world assembler
implementations and simplifies both validation and error reporting.

### Why an Intermediate Representation (IR)?
An intermediate representation decouples parsing from encoding. This allows syntactic and semantic checks to be performed before instruction encoding and makes the assembler easier to extend with additional instructions or execution stages.

### Error Handling Strategy
Errors are detected as early as possible during parsing and pass1. The assembler reports
meaningful error messages while ensuring that allocated resources are released properly to
avoid memory leaks.

---


## Testing & Debugging

- Runtime memory analysis using **Valgrind**
- Additional checks with **AddressSanitizer**
- Defensive programming practises applied across all modules

---

## References

- Computer Architecture and Design — Patterson & Hennesy
- Dİgital Design and Computer Architecture — Harris & Harris
- MIPS32 Architecture Specification

---

## Build & Project Structure

### Requirements


- GCC or Clang (C11 support)
- CMake >= 3.20
- Linux environment recommended

---


### Build Instructions

```bash
mkdir build
cd build
cmake ..
make
```

This will build:
- multiple static libraries representing different stages of the assembler
- the main assembler application
- unit test executables

### Running the Assembler
```bash
./mips_app
```

### Enabling Sanitizers

AddressSanitizer (ASan) and UndefinedBehaviourSanitizer (UBSan) can be enabled at configure time.
```bash
cmake -DENABLE_ASAN=ON -DENABLE_UBSAN=ON ..
make
```

The project is structured as multiple static libraries to clearly seperate
responsibilities between different components of the assembler:

- **mips_core**
  - Core data structures and utilities
  - Symbol table, IR, register mapping, and error handling

- **mips_front**
  - Frontend processing stages
  - Lexer, parser, and preprocessig logic

- **mips_asm**
  - Assembler passes
  - Currently implements Pass 1

- **mips_app**
  - Application entry point
  - Orchestrates the assembler pipeline

This modular design improves maintainability, testability and allows individual
components to be extended or replaced independently.

## Testing

Unit tests are build and executed using CTest

```bash
ctest
```


## Build Design Notes


Each stage of the assembler pipeline is implemented as a seperate static
library. This design demonstrates the logical seperation of an assembler into
frontend, core, and pass-specific stages.

Sanitizer support is integrated directly into the build system and can be enabled selectively.
