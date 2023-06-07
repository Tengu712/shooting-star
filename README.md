# Shooting Star

## Outline

A small frontend framework for game development. This framework is supported on Windows and Linux.

## Preparation

Install dependencies:

* Linux
  * X11
  * vulkan
* Windows
  * user32
  * xinput
  * vulkan-1

Write shaders:

```c
/* vertex shader */
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
/* omit outputs */
void main() {
    /* omit */
}
```
```c
/* fragment shader */
#version 450
layout(binding=1) uniform sampler2D diffuse_map;
/* omit inputs */
/* omit outputs */
void main() {
    /* omit */
}
```

Compile shaders to `shader.vert.spv` and `shader.frag.spv` at runtime root directory:

```
$ glslc -o <runtime-root>/shader.vert.spv <vertex-shader>
$ glslc -o <runtime-root>/shader.frag.spv <fragment-shader>
```

## Example

Here is an example that creates a window and clears it default color:

```rust
use sstar::{vulkan::*, window::*};

fn main() {
    let mut window_app = WindowApp::new("Sample Program", 640, 480);
    let vulkan_app = VulkanApp::new(&window_app, 10);
    while window_app.do_events() {
        vulkan_app.render(None, &[]).unwrap();
    }
    vulkan_app.terminate();
    window_app.terminate();
}
```

## License

This software (shooting-star) is published under the CC0 public domain.
However, some dependencies may be linked when building an app using this software.
See [LICENSE-ALL](./LICENSE-ALL) in detail.
