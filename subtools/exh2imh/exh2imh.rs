use std::fs::File;
use std::io::{BufReader, BufWriter, BufRead, Write};
use std::process::exit;

fn main() {
    let args = std::env::args().collect::<Vec<String>>();
    if args.len() < 3 {
        println!("exh2imh <in-filename> <out-filename>");
        return;
    }
    let f_in = match File::open(args[1].clone()) {
        Ok(n) => n,
        Err(_) => {
            eprintln!("error: '{}' not found", args[1].clone());
            exit(1);
        }
    };
    let f_out = match File::create(args[2].clone()) {
        Ok(n) => n,
        Err(e) => {
            eprintln!(
                "error: '{}' not created : {}",
                args[1].clone(),
                e.to_string()
            );
            exit(1);
        }
    };
    let buf_reader = BufReader::new(f_in);
    let mut buf_writer = BufWriter::new(f_out);
    for line in buf_reader.lines() {
        let mut line = line.unwrap();
        if line.starts_with("#define") {
            line = line.replace("__declspec(dllexport) ", "");
        }
        line = line.replace("EXPORT", "CALLCONV");
        buf_writer.write_all(line.as_bytes()).unwrap();
        buf_writer.write_all(b"\n").unwrap();
    }
}