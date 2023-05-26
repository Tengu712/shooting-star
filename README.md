# Shooting Star

## Outline

A small frontend framework for game development. This framework is supported on Windows and Linux.

## Usage

Look at [docs/usage.md](./docs/usage.md).

## Build

Following commands are required:

* make (to create tpl.rs)
* cargo (to build)

Following subcommands are required:

* glslc
* clang (needed for bindgen)
* bindgen (install by `cargo install bindgen-cli`)

Following dependencies are required:

* Linux
  * X11
  * vulkan
* Windows
  * user32
  * xinput
  * vulkan-1

```
$ make
$ cargo build
```
