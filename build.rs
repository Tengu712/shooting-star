use std::env;
use std::fs::File;
use std::io::Write;
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

const VERTEX_SHADER: &str = include_str!("./src/shader.vert");
const FRAGMENT_SHADER: &str = include_str!("./src/shader.frag");

fn to_regx(s: &str) -> String {
    s.lines()
        .filter(|n| !n.starts_with('#'))
        .collect::<Vec<&str>>()
        .join("|")
}

fn main() {
    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());

    // get allowlists
    let allowlist_function = to_regx(COMMON_FUNCTION) + "|" + &to_regx(OS_FUNCTION);
    let allowlist_type = to_regx(COMMON_TYPE) + "|" + &to_regx(OS_TYPE);
    let allowlist_var = to_regx(COMMON_VAR) + "|" + &to_regx(OS_VAR);

    // tpl
    println!("cargo:rerun-if-changed=./src/tpl.h");
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

    // stb libraries
    cc::Build::new()
        .file("./src/tpl.c")
        .include("./src")
        .compile("tpl");

    // link libraries
    println!("cargo:rustc-link-lib=tpl");
    #[cfg(target_os = "linux")]
    {
        println!("cargo:rustc-link-lib=X11");
        println!("cargo:rustc-link-lib=vulkan");
    }
    #[cfg(target_os = "windows")]
    {
        println!("cargo:rustc-link-lib=user32");
        println!("cargo:rustc-link-lib=Xinput");
        println!("cargo:rustc-link-lib=vulkan-1");
    }

    // default shaders
    let compiler = shaderc::Compiler::new().unwrap();
    let vert_shader = compiler
        .compile_into_spirv(
            VERTEX_SHADER,
            shaderc::ShaderKind::Vertex,
            "shader.vert",
            "main",
            None,
        )
        .unwrap();
    let frag_shader = compiler
        .compile_into_spirv(
            FRAGMENT_SHADER,
            shaderc::ShaderKind::Fragment,
            "shader.frag",
            "main",
            None,
        )
        .unwrap();
    let mut file_vert_shader = File::create(out_path.join("shader.vert.spv")).unwrap();
    file_vert_shader
        .write_all(vert_shader.as_binary_u8())
        .unwrap();
    let mut file_frag_shader = File::create(out_path.join("shader.frag.spv")).unwrap();
    file_frag_shader
        .write_all(frag_shader.as_binary_u8())
        .unwrap();
}
