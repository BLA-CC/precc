# precc

Compiler of a small language that supports programs that look like the following.

```c
void main() {
    int x;
    x = 10;
    y = 20;

    int z;
    z = x + y * 10;

    bool w;
    w = true;

    return;
}
```

Predecessor of a proper yet-to-exist programming language *BLACC*.

## Compiling

### Requirements

- C Compiler
- flex
- bison

Just running `make` should suffice to generate the `precc` executable.

## Running

For now, the executable waits until *stdin* reaches *EOF* to then begin with parsing and semantic analysis.
A few plain-text files can be found in `examples/` to try the compiler, they're meant to be piped to *stdin* for convenience, here's a way to achieve this.

```sh
./precc < examples/basic.txt
```
