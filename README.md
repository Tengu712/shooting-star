# Shooting Star

## Outline

A small frontend framework for game development. This framework is supported on Windows and Linux.

## Usage

Look at [docs/usage.md](./docs/usage.md).

## Build

Following commands are required:

* cargo (or rustc)
* bindgen (install by `cargo install bindgen-cli`)
* clang (needed for bindgen)

Following dependencies are required:

* Linux
  * X11
  * vulkan
* Windows
  * xinput
  * vulkan-1

```
$ bindgen tpl.h -o src/tpl.rs
$ cargo test
$ cargo build
```
