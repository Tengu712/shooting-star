#[cfg(target_os = "linux")]
fn main() {
    println!("cargo:rustc-link-lib=X11");
    println!("cargo:rustc-link-lib=vulkan");
}
