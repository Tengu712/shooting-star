# Fireball

## Outline

A small game engine.

## Dependencies

When linking with Fireball, following dependencies are required:

* Linux
  * xcb
  * vulkan

## Build

Run `make` command like below and link `libfireball.a` as you like.

For example, if you use Ubuntu and gcc:

```sh
$ make build_linux
$ gcc -o main main.c -L. -lfireball -lxcb -lvulkan
```