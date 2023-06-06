use std::env;
use std::path::PathBuf;

const COMMON_FUNCTION: &str = include_str!("./src/tpl/common/function.txt");
const COMMON_TYPE: &str = include_str!("./src/tpl/common/type.txt");
const COMMON_VAR: &str = include_str!("./src/tpl/common/var.txt");

#[cfg(target_os = "linux")]
const OS_FUNCTION: &str = include_str!("./src/tpl/linux/function.txt");
#[cfg(target_os = "linux")]
const OS_TYPE: &str = include_str!("./src/tpl/linux/type.txt");
#[cfg(target_os = "linux")]
const OS_VAR: &str = include_str!("./src/tpl/linux/var.txt");

#[cfg(target_os = "windows")]
const OS_FUNCTION: &str = include_str!("./src/tpl/windows/function.txt");
#[cfg(target_os = "windows")]
const OS_TYPE: &str = include_str!("./src/tpl/windows/type.txt");
#[cfg(target_os = "windows")]
const OS_VAR: &str = include_str!("./src/tpl/windows/var.txt");

fn to_regx(s: &str) -> String {
    s.lines()
        .filter(|n| !n.starts_with('#'))
        .collect::<Vec<&str>>()
        .join("|")
}

#[cfg(target_os = "linux")]
fn linux() {
    println!("cargo:rustc-link-lib=X11");
    println!("cargo:rustc-link-lib=vulkan");
    cc::Build::new()
        .file("./src/tpl.c")
        .include("./src")
        .compile("libtpl.a");
}

#[cfg(target_os = "windows")]
fn windows() {
    println!("cargo:rustc-link-lib=user32");
    println!("cargo:rustc-link-lib=Xinput");
    println!("cargo:rustc-link-lib=vulkan-1");
    cc::Build::new()
        .file("./src/tpl.c")
        .include("./src")
        .compile("./tpl.lib");
}

fn main() {
    let allowlist_function = to_regx(COMMON_FUNCTION) + "|" + &to_regx(OS_FUNCTION);
    let allowlist_type = to_regx(COMMON_TYPE) + "|" + &to_regx(OS_TYPE);
    let allowlist_var = to_regx(COMMON_VAR) + "|" + &to_regx(OS_VAR);

    println!("cargo:rerun-if-changed=./src/tpl.h");
    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());
    bindgen::Builder::default()
        .header("./src/tpl.h")
        .allowlist_function(allowlist_function)
        .allowlist_type(allowlist_type)
        .allowlist_var(allowlist_var)
        .parse_callbacks(Box::new(bindgen::CargoCallbacks))
        .generate()
        .expect("failed to convert tpl.h to tpl.rs")
        .write_to_file(out_path.join("bindings.rs"))
        .expect("failed to write tpl.rs");

    println!("cargo:rustc-link-lib=tpl");
    #[cfg(target_os = "linux")]
    linux();
    #[cfg(target_os = "windows")]
    windows();
}
