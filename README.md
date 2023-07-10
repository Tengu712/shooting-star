# Shooting Star

## Outline

A small frontend framework for game development. This framework is supported on Windows and Linux.

## Quick Start

Install dependencies:

* Linux
  * X11
  * vulkan
* Windows
  * user32
  * xinput
  * vulkan-1

Create a project with Cargo. Then, add a following line to Cargo.toml:

```
[dependencies]
sstar = { version = "0.1.9", features=["with-default-shaders"] }
```

Write main.rs as follow (this is a program that creates a window and clears it default color):

```rust
use sstar::app::SStarApp;

fn main() {
    let mut app = SStarApp::new("Sample Program", 640.0, 480.0, 10);
    while app.update() {
        app.flush();
    }
    app.terminate();
}
```

## Vertex Shader Interface

If you use a custom vertex shader, you should use following definitions:

```c
#version 450

layout(push_constant) uniform PushConstant {
    vec4 scl;
    vec4 rot;
    vec4 trs;
    vec4 col;
    vec4 uv;
    ivec4 param;
} constant;

layout(binding=0) uniform UniformBuffer {
    mat4 view;
    mat4 perse;
    mat4 ortho;
};

layout(location=0) in vec3 in_pos;
layout(location=1) in vec2 in_uv;
```

## License

This software (shooting-star) is published under the CC0 public domain.
However, some dependencies may be linked when building an app using this software.
See [LICENSE-ALL](./LICENSE-ALL) in detail.
