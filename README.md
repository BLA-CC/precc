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

