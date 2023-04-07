use std::fs::File;
use std::io::{BufWriter, Write};
use std::path::Path;
use std::process::{exit, Command, Stdio};

const SUBTOOLS: [&'static str; 3] = [
    "./subtools/bin2c/bin2c",
    "./subtools/exh2imh/exh2imh",
    "./subtools/cpin/cpin",
];
const RULE_OBJ: &'static [u8] = b"\
rule obj
    depfile = $out.d
    command = gcc -MMD -MF $out.d -Wall -fPIC -c -o $out $in
";
const RULE_DLL: &'static [u8] = b"\
rule dll
    command = gcc -Wall -fPIC -shared -o $out $in $flags
";
const RULE_RSC: &'static [u8] = b"\
rule rsc
    command = rustc -o $out $in
";
const RULE_GSC: &'static [u8] = b"\
rule gsc
    command = glslc -o $out $in
";
const RULE_B2C: &'static [u8] = b"\
rule b2c
    command = ./subtools/bin2c/bin2c $in $out $symbol
";
const RULE_E2I: &'static [u8] = b"\
rule e2i
    command = ./subtools/exh2imh/exh2imh $in $out
";
const RULE_CP: &'static [u8] = b"\
rule cp
    command = ./subtools/cpin/cpin $in $out
";
#[cfg(target_os = "windows")]
const BUILD_AFTER: &'static [u8] = b"\
build ./build/sstar.h: e2i ./src/sstar.h
build ./sample/sstar.h: cp ./build/sstar.h
build ./sample/sstar.dll: cp ./build/sstar.dll
build all: phony ./sample/sstar.h ./sample/sstar.dll
default all
";
#[cfg(target_os = "linux")]
const BUILD_AFTER: &'static [u8] = b"\
build ./build/sstar.h: e2i ./src/sstar.h
build ./sample/sstar.h: cp ./build/sstar.h
build ./sample/libsstar.so: cp ./build/libsstar.so
build all: phony ./sample/sstar.h ./sample/libsstar.so
default all
";
#[cfg(target_os = "windows")]
const RULE_SAMPLE: &'static [u8] = b"\
rule sample
    command = gcc -Wall -o $out $in ./sample/sstar.dll
";
#[cfg(target_os = "linux")]
const RULE_SAMPLE: &'static [u8] = b"\
rule sample
    command = gcc -Wall -o $out $in -L./sample/ -lsstar -Wl,-rpath='$$ORIGIN'
";

fn main() {
    let args = std::env::args().collect::<Vec<String>>();
    if args.len() < 2 {
        println!("dev <option>");
        println!("    init  : init workspace");
        println!("    gn    : generate ninja");
        println!("    clean : clean workspace");
        return;
    }
    if args[1] == "init" {
        run_init();
    } else if args[1] == "gn" {
        run_gn();
    } else if args[1] == "clean" {
        run_clean();
    } else {
        eprintln!("dev error: invalid option '{}'", args[1]);
        exit(1);
    }
}

fn run_init() {
    for subtool in SUBTOOLS {
        println!("dev: building {}", subtool);
        Command::new("rustc")
            .arg(String::from("-o"))
            .arg(subtool)
            .arg(String::from(subtool) + ".rs")
            .stderr(Stdio::inherit())
            .spawn()
            .unwrap();
    }
    generate_ninja().unwrap();
}

fn run_gn() {
    for subtool in SUBTOOLS {
        if !Path::new(subtool).is_file() {
            eprintln!("dev error: call `dev init` first.");
            exit(1);
        }
    }
    generate_ninja().unwrap();
}

fn run_clean() {
    for subtool in SUBTOOLS {
        if Path::new(subtool).is_file() {
            remove_file(subtool);
        }
    }
    let path_ninja_str = "./build.ninja";
    if Path::new(path_ninja_str).is_file() {
        println!("dev: ninja -t clean : ");
        Command::new("ninja")
            .arg("-t")
            .arg("clean")
            .stdout(Stdio::inherit())
            .stderr(Stdio::inherit())
            .output()
            .unwrap();
        remove_file(path_ninja_str);
    }
    let files_sample = match std::fs::read_dir("./sample") {
        Ok(n) => n,
        Err(e) => {
            eprintln!(
                "dev error: failed to get files in sample : {}",
                e.to_string()
            );
            exit(1);
        }
    };
    for file in files_sample {
        let path = file.unwrap().path();
        let path_str = path.to_str().unwrap();
        #[cfg(target_os = "windows")]
        if !path_str.ends_with(".exe") {
            continue;
        }
        #[cfg(target_os = "linux")]
        if !path_str.ends_with(".out") {
            continue;
        }
        remove_file(path_str);
    }
}

/// A function to generate ./build.ninja based on files in ./src .
fn generate_ninja() -> std::io::Result<()> {
    let file_out = match File::create("./build.ninja") {
        Ok(n) => n,
        Err(e) => {
            eprintln!("dev error: 'build.ninja' not created : {}", e.to_string());
            exit(1);
        }
    };
    let mut buf_writer = BufWriter::new(file_out);

    buf_writer.write_all(RULE_OBJ)?;
    buf_writer.write_all(RULE_DLL)?;
    buf_writer.write_all(RULE_RSC)?;
    buf_writer.write_all(RULE_GSC)?;
    buf_writer.write_all(RULE_B2C)?;
    buf_writer.write_all(RULE_E2I)?;
    buf_writer.write_all(RULE_CP)?;
    buf_writer.write_all(RULE_SAMPLE)?;

    let mut cfiles = Vec::new();
    let mut ofiles = Vec::new();
    let mut shaders = Vec::new();
    find_c(&mut cfiles, &mut shaders, "./src/");
    for cfile in &cfiles {
        let mut dest = cfile.replace("src", "tmp");
        dest.pop();
        dest.push('o');
        buf_writer.write_all(b"build ")?;
        buf_writer.write_all(dest.as_bytes())?;
        buf_writer.write_all(b": obj ")?;
        buf_writer.write_all(cfile.as_bytes())?;
        buf_writer.write_all(b"\n")?;
        ofiles.push(dest);
    }
    for shader in &shaders {
        let dest = shader.replace("src", "tmp");
        let spv = dest.clone() + ".spv";
        let c = dest.clone() + ".c";
        let o = dest.clone() + ".o";
        let (_, symbol) = dest.rsplit_once('/').unwrap();
        let symbol = symbol.replace('.', "_");
        // spv
        buf_writer.write_all(b"build ")?;
        buf_writer.write_all(spv.as_bytes())?;
        buf_writer.write_all(b": gsc ")?;
        buf_writer.write_all(shader.as_bytes())?;
        buf_writer.write_all(b"\n")?;
        // c
        buf_writer.write_all(b"build ")?;
        buf_writer.write_all(c.as_bytes())?;
        buf_writer.write_all(b": b2c ")?;
        buf_writer.write_all(spv.as_bytes())?;
        buf_writer.write_all(b"\n")?;
        buf_writer.write_all(b"    symbol = ")?;
        buf_writer.write_all(symbol.as_bytes())?;
        buf_writer.write_all(b"\n")?;
        // o
        buf_writer.write_all(b"build ")?;
        buf_writer.write_all(o.as_bytes())?;
        buf_writer.write_all(b": obj ")?;
        buf_writer.write_all(c.as_bytes())?;
        buf_writer.write_all(b"\n")?;
        ofiles.push(o);
    }
    #[cfg(target_os = "windows")]
    buf_writer.write_all(b"build ./build/sstar.dll: dll ")?;
    #[cfg(target_os = "linux")]
    buf_writer.write_all(b"build ./build/libsstar.so: dll ")?;
    buf_writer.write_all(ofiles.join(" $\n    ").as_bytes())?;
    buf_writer.write_all(b"\n")?;
    #[cfg(target_os = "windows")]
    buf_writer.write_all(b"    flags = -lvulkan-1")?;
    #[cfg(target_os = "linux")]
    buf_writer.write_all(b"    flags = -fvisibility=hidden -lm -lxcb -lvulkan")?;
    buf_writer.write_all(b"\n")?;
    buf_writer.write_all(BUILD_AFTER)?;

    let mut samples = Vec::new();
    find_c(&mut samples, &mut Vec::new(), "./sample/");
    #[cfg(target_os = "windows")]
    let ex = ".exe";
    #[cfg(target_os = "linux")]
    let ex = ".out";
    for sample in samples {
        let (_, name) = sample.rsplit_once('/').unwrap();
        let name = &name[0..(name.len() - 2)];
        let out = String::from("./sample/") + name + ex;
        buf_writer.write_all(b"build ")?;
        buf_writer.write_all(out.as_bytes())?;
        buf_writer.write_all(b": sample ")?;
        buf_writer.write_all(sample.as_bytes())?;
        buf_writer.write_all(b"\n")?;
        buf_writer.write_all(b"build ")?;
        buf_writer.write_all(name.as_bytes())?;
        buf_writer.write_all(b": phony ")?;
        buf_writer.write_all(out.as_bytes())?;
        buf_writer.write_all(b"\n")?;
    }

    println!("dev: created ./build.ninja");
    Ok(())
}

/// A recursive function to find all C files or GLSL files in `dir`.
fn find_c(cfiles: &mut Vec<String>, shaders: &mut Vec<String>, dir: &str) {
    let entries = match std::fs::read_dir(dir) {
        Ok(n) => n,
        Err(e) => {
            eprintln!(
                "dev error: failed to get files in {} : {}",
                dir,
                e.to_string()
            );
            exit(1);
        }
    };
    for entry in entries {
        let path = entry.unwrap().path();
        let path_str = path.to_str().unwrap();
        #[cfg(target_os = "windows")]
        if path_str.contains("linux") {
            continue;
        }
        #[cfg(target_os = "linux")]
        if path_str.contains("windows") {
            continue;
        }
        if path.is_dir() {
            find_c(cfiles, shaders, path_str);
        } else if path_str.ends_with(".c") {
            cfiles.push(String::from(path_str).replace('\\', "/"));
        } else if path_str.ends_with(".vert") || path_str.ends_with(".frag") {
            shaders.push(String::from(path_str).replace('\\', "/"));
        }
    }
}

/// A function to remove a file
fn remove_file(path: &str) {
    match std::fs::remove_file(path) {
        Ok(()) => println!("dev: removed {}", path),
        Err(e) => eprintln!("dev warning: failed to remove {} : {}", path, e.to_string(),),
    }
}
