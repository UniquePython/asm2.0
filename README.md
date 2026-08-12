# asm2.0

[![CI](https://github.com/UniquePython/asm2.0/actions/workflows/ci.yml/badge.svg)](https://github.com/UniquePython/asm2.0/actions/workflows/ci.yml)

The future of asm.

## Quick Start

Clone the repository:
```bash
# Using HTTPS
git clone https://github.com/UniquePython/asm2.0.git

# Using SSH
git clone git@github.com:UniquePython/asm2.0.git

cd asm2.0
```

Build the project:
```bash
make clean && make
```

Compile and run the example program:
```bash
make run ARGS="--compile examples/prog.asm2"
./prog.out
echo $?   # 42
```

## Language Reference

### Overview

asm2.0 is a small, statement-based assembly language for Linux x86-64.
Every program is a flat sequence of statements, each ending in a
semicolon (label declarations end in a colon instead). There is
exactly one required statement per program (`entry`) and the rest are
used to build up the program's behavior.

The compiler translates a `.asm2` source file directly into a
statically linked, directly executable ELF64 binary — no separate
assembler or linker step is involved.

### Comments

A `#` starts a comment that runs to the end of the line:

```
# this is a comment
entry start; # so is this
```

### Statements

There are four statement kinds.

#### `entry`

```
entry <label>;
```

Declares which label execution starts at. Every program must have
**exactly one** `entry` statement, and the label it names must be
declared somewhere in the same program (see `label` below). `entry`
can appear anywhere in the file — it does not need to come before the
label it refers to.

```
entry start;
```

#### `label`

```
label <name>:
```

Declares a named position in the program. Statements following a
label, up to the next label (or the end of the file), belong to it.
Label names must be unique within a program.

```
label start:
    syscall;
```

#### `move`

```
move <number> to <register>;
```

Loads an unsigned integer literal into a register. Currently, the
source of a `move` must be a numeric literal and the destination must
be a register — see [Limitations](#limitations) for what's not yet
supported here.

```
move 42 to edi;
```

#### `syscall`

```
syscall;
```

Performs a Linux x86-64 system call using the current register
values, following the standard x86-64 Linux syscall ABI: the syscall
number goes in `eax`, and the first three arguments go in `edi`,
`esi`, and `edx` respectively. `syscall` takes no operands of its
own — set up the registers with `move` beforehand.

Note that the full x86-64 syscall ABI supports up to six arguments
(the remaining three going in `r10`, `r8`, and `r9`), but those
registers aren't exposed by asm2.0 yet — see
[Limitations](#limitations). Only syscalls needing three or fewer
arguments are currently reachable.

```
label start:
    move 231 to eax;   # syscall number 231 = exit_group
    move 42 to edi;    # first argument = exit code
    syscall;
```

### Operands

An operand is either:

* **A number** — an unsigned decimal integer literal (e.g. `42`,
  `231`). No hexadecimal, binary, or negative literals are supported
  yet.
* **A register** — one of the eight names listed below.

### Registers

asm2.0 exposes the eight 32-bit x86-64 general-purpose registers:

| Name  | Purpose (by x86-64 convention) |
|-------|---------------------------------|
| `eax` | syscall number / return value  |
| `ecx` | general purpose                |
| `edx` | 3rd syscall argument            |
| `ebx` | general purpose                |
| `esp` | stack pointer                   |
| `ebp` | base pointer                    |
| `esi` | 2nd syscall argument            |
| `edi` | 1st syscall argument            |

### A Complete Example

This program calls `exit(42)` via the Linux `exit_group` syscall:

```
entry start;

label start:
    move 231 to eax;
    move 42 to edi;
    syscall;
```

Compiling and running it:

```bash
./bin/asm2.0 --compile prog.asm2
./prog.out
echo $?   # 42
```

### Compiler Stages & CLI

The compiler can be stopped after any of its three stages:

```bash
asm2.0 --tokenize <file>   # run the lexer, print the token stream
asm2.0 --parse <file>      # run the lexer + parser, print the AST
asm2.0 --compile <file>    # run the full pipeline, emit an executable
```

`--compile` accepts an optional output path:

```bash
asm2.0 --compile prog.asm2 --output myprogram
```

If `--output` is omitted, a default name is derived from the input
file: its directory is discarded, everything from the first `.`
onward is stripped, and `.out` is appended — always, even if nothing
was stripped. The result is written to the current working directory,
regardless of where the input file lives. For example:

| Input                  | Default output (in CWD) |
|-------------------------|--------------------------|
| `prog.asm2`             | `prog.out`               |
| `src/my.prog.asm2`      | `my.out`                 |
| `prog` (no extension)   | `prog.out`                |

The emitted file is a self-contained, directly executable ELF64
binary (no separate linking step) and is marked executable
automatically.

### Exit Codes

| Code | Meaning                          |
|------|-----------------------------------|
| `0`  | Success                          |
| `2`  | CLI usage error                  |
| `3`  | Lexer error(s)                   |
| `4`  | Parser error(s)                  |
| `5`  | Codegen error                    |

### Limitations

asm2.0 is under active development. Some things you might reasonably
expect are not implemented yet:

* **`move` only supports an immediate-to-register form.**
  Register-to-register moves (`move eax to ebx;`) and moving into a
  numeric literal are not supported — the source must be a number and
  the destination must be a register.
* **Immediates for `move` are capped at 32 bits** (`0` to
  `4294967295`), even though integer literals themselves can be as
  large as 64 bits elsewhere in the language.
* **No arithmetic, comparison, or control-flow statements** — there is
  currently no way to add, subtract, compare, jump, or branch.
* **Only 8 of the 16 x86-64 general-purpose registers are exposed**
  (the classic 32-bit-named subset: `eax`, `ecx`, `edx`, `ebx`, `esp`,
  `ebp`, `esi`, `edi`). There's no access to `r8`–`r15`, and no way to
  address the full 64-bit register width. This also means only the
  first three syscall arguments are reachable (see `syscall` above).
* **No negative number literals.**
* **No hexadecimal or binary literal syntax** — only decimal.

## License

Licensed under the MIT License.