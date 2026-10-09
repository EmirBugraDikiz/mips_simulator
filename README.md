# MIPS Two-Pass Assembler (C)

A modular, educational MIPS assembler implemented in C11. The assembler processes a MIPS assembly source file in two passes and produces in-memory 32-bit text and data images. A CPU simulator is planned but **not implemented yet**.

## Implemented Features

- Line preprocessing, tokenization, and parsing of supported MIPS assembly syntax
- Two-pass assembly with `.text` / `.data` section tracking
- Symbol table construction and intermediate representation (IR) during PASS1
- Label resolution and instruction encoding during PASS2
- In-memory `text_image` and `data_image` output with configured base addresses
- Modular CMake build with separate core, frontend, and assembler libraries
- Unit tests and optional AddressSanitizer / UndefinedBehaviorSanitizer builds

> The assembler supports a subset of MIPS instructions and directives, not the complete MIPS32 ISA. The output is currently held in memory and printed by the CLI; it is not an ELF executable or a CPU simulation.

## Architecture

```text
MIPS assembly source (.mips)
          |
          v
PASS1: preprocess -> lexer -> parser
          |            |
          +--> symbol table + intermediate representation (IR)
                                   |
                                   v
PASS2: IR + symbol table -> label resolution and instruction encoding
                                   |
                                   v
                   AssemblerOutput (in memory)
                     text_image[] / data_image[]
                                   |
                                   v
                       CLI hexadecimal dump
```

PASS1 tracks `.text` and `.data` addresses, records labels in the symbol table, and appends parsed statements to the IR. PASS2 consumes the IR and symbol table to encode supported instructions and populate the output images. The assembler uses text base `0x00400000` and data base `0x10010000` in its CLI configuration.

## Requirements

- C11 compiler (GCC or Clang)
- CMake 3.20 or newer
- A build tool supported by CMake
- Linux recommended for development and Valgrind

## Build

From the repository root:

```bash
cmake -S . -B build
cmake --build build
```

## Run

The CLI uses `data/files/assembly_input.mips` by default:

```bash
./build/mips_app
```

Or pass a custom input file:

```bash
./build/mips_app path/to/program.mips
```

The application reports PASS1/PASS2 status, symbol addresses, and the hexadecimal words in the `.text` and `.data` images. See `data/files/assembly_input.mips` for a sample program.

## Tests

```bash
ctest --test-dir build --output-on-failure
```

To enable sanitizers in a separate build directory:

```bash
cmake -S . -B build-sanitize -DENABLE_ASAN=ON -DENABLE_UBSAN=ON
cmake --build build-sanitize
ctest --test-dir build-sanitize --output-on-failure
```

Valgrind can be used on a regular (non-ASan) build:

```bash
valgrind --leak-check=full ./build/mips_app
```

## Repository Structure

```text
app/                 CLI entry point
include/             Public module headers
src/front/           Preprocessor, lexer, parser
src/core/            IR, symbol table, ISA helpers, errors
src/asm/             PASS1 and PASS2
tests/unit_tests/    Unit tests and CTest registration
data/files/          Sample assembly input
CMakeLists.txt       Build configuration
```

## Design Notes

**Why two passes?** Labels can be referenced before they are defined. PASS1 establishes section addresses and symbols; PASS2 uses those addresses to encode references and instructions.

**Why an IR?** The intermediate representation separates parsing from encoding and allows PASS2 to work on structured statements instead of raw source lines.

**Output format:** The CLI prints 32-bit words with their corresponding configured addresses. A serialized binary format and a simulator are not currently provided.

## Roadmap

- [x] Preprocessing, lexer, parser
- [x] Symbol table and IR
- [x] PASS1 and PASS2
- [x] CLI machine-code dump
- [ ] Improve diagnostics and remove temporary debug prints
- [ ] Expand supported instructions and tests
- [ ] CPU simulator (fetch/decode/execute)

## References

- *Digital Design and Computer Architecture* — Harris & Harris
- *Computer Organization and Design* — Patterson & Hennessy
- MIPS32 architecture documentation
