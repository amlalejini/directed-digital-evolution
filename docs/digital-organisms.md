# Digital Organisms

Here, we provide supplemental information about the digital organisms used in our experiments.

## Virtual Hardware Components

Each digital organism is defined by a sequence of program instructions (its genome) and a set of virtual hardware components used to interpret and express those instructions.
The virtual hardware and genetic representation used in this work extends that of [@dolson_exploring_2019,hernandez_what_2022]. The virtual hardware includes the following components:

- **Instruction pointer**:
  A marker that indicates the position in the genome currently being executed.
  Instructions may influence how the instruction pointer moves through the genome (e.g., `if` instructions).
- **Memory registers**:
  A digital organism has access to 16 memory registers (abbreviated as REG-0 through REG-15) for performing computations.
  Each register can store a single floating point value, and instructions can read and write to registers.
  Registers are initialized with values corresponding to their register ID (e.g., REG-0 is initialized with the value 0).
- **Memory stacks**:
  Each digital organism has access to 16 memory stacks (abbreviated as STK-0 through STK-15).
  Instructions can push values onto a stack and pop them off later.
- **Input and output buffers**:
  Each digital organism has a read-only input buffer and a write-only output buffer.
  Digital organisms could execute instructions to read values from their input buffer and execute instructions to write values to their output buffer.
  When an organism is born, their output buffer is empty, and we initialize their input buffer with 2 values (as all computational tasks used in this work had a maximum of 2 inputs), each value ranging between 0 and 100000000.
  Input buffers are accessed in order and are circular; that is, when an instruction reads from the input buffer, it reads the next value (starting with the first) and wraps around to the beginning after reading the last value in the buffer.
  To perform a computational task (e.g., those in Table 1 of our manuscript), an organism must load values from their input buffer into their memory registers, compute the requisite function, and output the result to their output buffer.
  During an organism's lifetime, we analyzed their output buffer to determine if they performed any of the designated computational tasks.
- **Scopes**:
  Each digital organism could make use of 16 ``scopes'' plus a global scope, making 17 possible scopes.
  Below is an introduction to scopes as a mechanism for modularity, which is adapted from [@dolson_exploring_2019]:
  > In software development, the _scope_ of a variable specifies the region of code in which that element may be used.
  In a sense, a scope is like a programmatic membrane, capable of encapsulating programmatic elements (e.g., variables, functions, _et cetera_) and allowing regions to be looped through or skipped entirely.
  Our genetic representation gives programs control over instruction- and memory-scoping, which allows programs to manage flow control and variable lifetime.
  >
  > In our genetic representation, scopes provide the backbone on top of which all of the other modularity-promoting features, such as loops and functions, are built.
  All instructions in a program exist within a scope, be it the default outermost scope or one of the 16 other available scopes that can be accessed via instructions.
  The 16 inner scopes have a hierarchy to them, such that higher-numbered scopes are always nested inside lower-numbered scopes.
  >
  > Starting at the beginning of a program, all instructions before the first change of scope are in the outermost (global) scope.
  After a scope-changing instruction occurs in the program, subsequent instructions are added to the new scope until another scope-changing instruction is encountered, and so on.
  These scopes are ordered numerically.
  Higher-numbered scopes are always nested inside lower-numbered scopes.
  Scopes can be exited with the `break` instruction or by any instruction that moves control to a lower-numbered scope.
  >
  > Scopes are also the foundation of program modules (functions) in our genetic representation.
  The `define` instruction allows the program to put instructions into a scope and associate the contents of that scope with one of 16 possible function names.
  Later, if that function is called (using the `call` instruction), the program enters the scope in which that function was defined and executes the instructions within that scope in sequence, including any internal (nested) scopes.
  >
  > Similarly, scopes are the foundation of loops.
    Two kinds of loops exist in the instruction set used here: while loops and countdown loops.
    Loops of both types have a corresponding scope, which contains the sequence of instructions that make up the body of the loop.
    Both types of loops repeat their body (i.e., the contents of their associated scope) until the value in an argument-specified register is 0.
    Countdown loops automatically decrement this register by one on every iteration.
    When any instruction is encountered that would cause the program to leave the current scope, the current iteration is ended and the next one begins.
- **Self-replication machinery**:
  A digital organism's virtual hardware tracks how many instructions an organism has copied (regardless of how many of those copies are erroneous), and prevents an organism from dividing until it has copied at least as many instructions as are in its genome.

## Instruction set

The table below gives the instruction set used by digital organisms in our digital directed evolution experiments.
Note that each instruction in an organism's genome contains three arguments, which may modify the effect of the instruction.
Instruction arguments are limited to the values 0 through 15, and arguments are used to specify any of the following: registers, raw values, or scopes.
In an instruction's description, we denote argument values as ARG-0, ARG-1, and ARG-2 where ARG-0 represents the value of the first argument for the instruction and so on.

Instructions that operate on bitstrings (e.g., `Not` and `Nand`) operate on the underlying bit representation of the values stored in the relevant registers.
When the result of a binary (true/false) comparison is stored in a register (e.g., from a `TestEqu` instruction), false is stored as 0 and true is stored as 1.

We excluded the following instructions from our genetic programming experiment: `CopyInst`, `DivideSelf`.

| Instruction | # Arguments | New scope | Description |
| ---         | ---         | ---       | ---         |
| `Nop`       | 0 | None    | No operation |
| `Inc`       | 1 | None    | REG[ARG-0] = REG[ARG-0] + 1 |
| `Dec`       | 1 | None    | REG[ARG-0] = REG[ARG-0] - 1 |
| `Not`       | 1 | None    | REG[ARG-0] = !REG[ARG-0] |
| `SetReg`    | 2 | None    | REG[ARG-0] = ARG-1 |
| `Add`       | 3 | None    | REG[ARG-2] = REG[ARG-0] + REG[ARG-1] |
| `Sub`       | 3 | None    | REG[ARG-2] = REG[ARG-0] - REG[ARG-1] |
| `Mult`      | 3 | None    | REG[ARG-2] = REG[ARG-0] * REG[ARG-1] |
| `Div`       | 3 | None    | REG[ARG-2] = REG[ARG-0] / REG[ARG-1] |
| `Mod`       | 3 | None    | REG[ARG-2] = REG[ARG-0] \% REG[ARG-1] |
| `Nand`      | 3 | None    | REG[ARG-2] = !REG[ARG-0] \& REG[ARG-1] |
| `TestEqu`   | 3 | None    | REG[ARG-2] = REG[ARG-0] == REG[ARG-1] |
| `TestNEqu`  | 3 | None    | REG[ARG-2] = REG[ARG-0] != REG[ARG-1] |
| `TestLess`  | 3 | None    | REG[ARG-2] = REG[ARG-0] $<$ REG[ARG-1] |
| `If`        | 2 | ARG-1   | If REG[ARG-0] != 0, continue to SCOPE[ARG-1]; else, skip SCOPE[ARG-1] |
| `While`     | 2 | ARG-1   | Repeat the contents of SCOPE[ARG-1] until REG[ARG-0] equals 0 |
| `Countdown` | 2 | ARG-1   | Repeat the contents of SCOPE[ARG-1], decrementing REG[ARG-0] each time, until REG[ARG-0] equals 0 |
| `Break`     | 1 | None    | Break out of SCOPE[ARG-0] |
| `Scope`     | 1 | ARG-0   | Enter SCOPE[ARG-0] |
| `Define`    | 2 | ARG-1   | Define this position as the starting point of function ARG-0 with its contents defined as SCOPE[ARG-1]. The function body is skipped after being defined; when called, the function automatically returns when SCOPE[ARG-1] is exited.  |
| `Call`      | 1 | None    | Call function ARG-0, which must already have been defined by a \code{Define} instruction. |
| `Push`      | 2 | None    | Push REG[ARG-0] onto STK[ARG-1] |
| `Pop`       | 2 | None    | Pop top value of STK[ARG-0] onto REG[ARG-1] |
| `CopyVal`   | 2 | None    | REG[ARG-1] = REG[ARG-0] |
| `ScopeReg`  | 1 | None    | Backup the value in REG[ARG-0]. When the current scope is exited, it will be restored. |
| `CopyInst`  | 0 | None    | Copy the next instruction (for self-replication). |
| `GetLen`    | 1 | None    | Set REG[ARG-0] to the current genome length. |
| `DivideSelf`| 0 | None    | If the requisite number of instructions have been copied, trigger division (producing an offspring). |
| `Input`     | 1 | None    | Load the next input value into REG[ARG-0] |
| `Output`    | 1 | None    | Append REG[ARG-0] to the output buffer |

## Ancestral genomes

The files containing the ancestral genome used to seed initial populations for each experiment are included in the [GitHub repository](https://github.com/amlalejini/directed-digital-evolution) [@git_repo_archive].

Each ancestral genome is 100 instructions long.
The ancestral genome used for our genetic programming experiments does nothing except define a root scope.
The ancestral genome used for our directed evolution experiments copies itself and then divides, producing an offspring.

### Ancestral genome for genetic programming experiment

```
Scope 0
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
```

### Ancestral genome for directed evolution experiments

```
Scope 0
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
Nop
GetLen 15
Countdown 15 1
CopyInst 0
Scope 0
DivideSelf
```