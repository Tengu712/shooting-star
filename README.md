# Shooting Star

## Outline

A small frontend framework for game development. This framework is supported on Windows and Linux.

## Usage

Look at [docs/usage.md](./docs/usage.md).

## Build

Following commands are required:

* ninja
* gcc
* rustc
* glslc

Following dependencies are required:

* Linux
  * xcb
  * vulkan
* Windows
  * vulkan-1

Use a subtool [dev](./subtools/dev) to build this project:

```
$ git clone <this-repository>
$ cd Shooting-Star
$ rustc -o ./dev ./subtools/dev/dev.rs
$ ./dev init
$ ./dev build
```
