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

## Example: Array Summation

The included [`data/files/assembly_input.mips`](data/files/assembly_input.mips) demonstrates an array-summation loop using arithmetic, branches, jumps, and load/store instructions.

```mips
.data
numbers: .word 12, 7, 25, -4, 10
result: .word 0

.text
main: ori $s0, $zero, 0x1001
sll $s0, $s0, 16
addiu $t0, $zero, 0
addiu $t1, $zero, 5
addiu $t2, $zero, 0
loop: beq $t0, $t1, done
sll $zero, $zero, 0
sll $t3, $t0, 2
addu $t4, $s0, $t3
lw $t5, 0($t4)
addu $t2, $t2, $t5
addiu $t0, $t0, 1
j loop
sll $zero, $zero, 0
done: sw $t2, 20($s0)
finish: j finish
sll $zero, $zero, 0
```

The five input values are `12, 7, 25, -4, 10`. The program is intended to calculate **50 (`0x00000032`)** and store it at `result` (`0x10010014`). This is the *expected runtime behavior*, not a result executed or verified by the assembler.

### Actual Assembler Output

The following output was obtained from the CLI after PASS1 and PASS2 completed successfully:

```text
PASS1: OK
  IR statements: 21
  Symbols: 6
  Text size: 68 bytes
  Data size: 24 bytes

SYMBOL TABLE
  Address       Section  Name
  0x10010000    .data    numbers
  0x10010014    .data    result
  0x00400000    .text    main
  0x00400014    .text    loop
  0x00400038    .text    done
  0x0040003C    .text    finish

PASS2: OK

TEXT IMAGE (17 words)
  Address       Word (hex)
0x00400000    0x34101001
0x00400004    0x00108400
0x00400008    0x24080000
0x0040000C    0x24090005
0x00400010    0x240A0000
0x00400014    0x11090008
0x00400018    0x00000000
0x0040001C    0x00085880
0x00400020    0x020B6021
0x00400024    0x8D8D0000
0x00400028    0x014D5021
0x0040002C    0x25080001
0x00400030    0x08100005
0x00400034    0x00000000
0x00400038    0xAE0A0014
0x0040003C    0x0810000F
0x00400040    0x00000000

DATA IMAGE (6 words)
  Address       Word (hex)
0x10010000    0x0000000C
0x10010004    0x00000007
0x10010008    0x00000019
0x1001000C    0xFFFFFFFC
0x10010010    0x0000000A
0x10010014    0x00000000

Assembly completed successfully.
```

**What this example validates:** the assembler assigns section addresses, resolves forward and backward labels, encodes R/I/J instructions, and initializes `.data` words including negative integers. The `DATA IMAGE` shows the *initial* `result` value (`0`); the assembler does not execute the `sw` instruction or modify it to `50`.

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
