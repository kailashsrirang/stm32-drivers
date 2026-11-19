# stm32-drivers

## Host-side servo unit tests

Initialize the Unity test framework submodule once after cloning, then run:

```sh
git submodule update --init --recursive
make test-unit
```

`make test-unit` compiles the real servo driver with the host C compiler and a
fake timer compare function. It runs without a board and does not change the
normal firmware build or flash hardware.
