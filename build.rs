#[cfg(target_os = "linux")]
fn main() {
    println!("cargo:rustc-link-search=native=./tpl");
    println!("cargo:rustc-link-lib=tpl");
    println!("cargo:rustc-link-lib=X11");
    println!("cargo:rustc-link-lib=vulkan");
}

#[cfg(target_os = "windows")]
fn main() {
    println!("cargo:rustc-link-search=native=./tpl");
    println!("cargo:rustc-link-lib=tpl");
    println!("cargo:rustc-link-lib=user32");
    println!("cargo:rustc-link-lib=Xinput");
    println!("cargo:rustc-link-lib=vulkan-1");
}
