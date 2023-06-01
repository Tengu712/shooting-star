# Shooting Star

## Outline

A small frontend framework for game development. This framework is supported on Windows and Linux.

## Build

Following commands are required:

* bindgen (install by `cargo install bindgen-cli`)
* cargo
* clang
* glslc

Following dependencies are required:

* Linux
  * X11
  * vulkan
* Windows
  * user32
  * xinput
  * vulkan-1

Run following commands to build in Linux:

```
$ ./build.sh
$ cargo build
```

or in Windows:

```
> powershell -NoProfile -ExecutionPolicy Unrestricted .\build.ps1
> cargo build
```
