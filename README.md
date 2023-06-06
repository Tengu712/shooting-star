# Shooting Star

## Outline

A small frontend framework for game development. This framework is supported on Windows and Linux.

## Example

Here is an example that creates a window and clears it default color:

```rust
use sstar::{
    vulkan::*,
    window::*,
};

fn main() {
    let mut window_app = WindowApp::new("Sample Program\0", 640, 480);
    let vulkan_app = VulkanApp::new(&window_app, 10);
    while window_app.do_events() {
        vulkan_app.render(None, &[]).unwrap();
    }
    vulkan_app.terminate();
    window_app.terminate();
}
```

## Dependencies

Following dependencies are required:

* Linux
  * X11
  * vulkan
* Windows
  * user32
  * xinput
  * vulkan-1

## License

This software (shooting-star) is published under the CC0 public domain.
However, some dependencies may be linked when building an app using this software.
See [LICENSE-ALL](https://github.com/Tengu712/shooting-star/LICENSE-ALL) in detail.
