# myshell — A Simple UNIX Shell in C

A basic UNIX shell implemented in C that supports command execution, I/O redirection, piping, command chaining, history, and signal handling.

Built for **CSE321 (Operating Systems) — Term Project 1** at BRAC University.

## Features

- **Command prompt** (`sh> `) that reads and executes standard Linux commands via `fork`/`execvp`
- **I/O redirection**: `<` (input), `>` (output, truncate), `>>` (output, append), implemented with `dup2`
- **Piping**: `command1 | command2 | ... | commandN`, any number of stages
- **Sequential execution**: `cmd1 ; cmd2 ; cmd3`
- **Conditional execution**: `cmd1 && cmd2` (stops the chain if a command fails)
- **Command history**: type `history` to list previously executed commands
- **Signal handling**: `Ctrl+C` interrupts the currently running child process without killing the shell itself

## Build

```bash
make
```

Produces the `myshell` binary. Requires `gcc` on a POSIX system (Linux/macOS/WSL).

## Usage

```bash
./myshell
```

Then use it like a regular shell:
