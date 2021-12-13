Processor simulator - asm1
==============
This is simple simulation of processor

## Introduction

*Processor simulator* is an educational project. It contains three programs:
1. assembler - generates binary `.dull` file from `.asm1` code
2. disassembler - generates `.asm1` code from `.dull` binary
3. executor - simulation of processor, executes `.dull` binary

## Launching

To run one of examples use

    make

name of example, path to binary and side files specified in makefile.

To compile use

    make assembler
    make disassembler
    make executor

path to binary specified in makefile.

## Abstract machine

In general, it is x86 like executor, there are general purpose registers, several special registers, memory which contains code section and place for stack - size and offset are specified in executor.h

Word is 64 bit, memory is flat, stack frames include saved rip, rbp is absent - used only rsp.

General registers are reached depends on call depth. Instruction `mode` changes state of abstract machine.

Data and Text sections aren't divided in binary file, but should be divided in assembler file.

## Assembler syntax

In general, syntax is standard for assembler languages. Destination argument is first. Instruction `libcall` is substitute for standard library - just mock. Instruction `if*` defines will next instruction be executed.

Example:

```
.TEXT
_start
    mode    9
    libcall read_num
    mode    3
    libcall write_num
    syscall exit
```
