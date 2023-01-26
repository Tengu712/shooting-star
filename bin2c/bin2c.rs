use std::fs::File;
use std::io::{BufReader, BufWriter, Read, Write};
use std::process::exit;

fn main() {
    let args = std::env::args().collect::<Vec<String>>();
    if args.len() != 4 {
        println!("bin2c <in-filename> <out-filename> <symbol>");
        return;
    }
    println!("compile '{}' to '{}'", args[1].clone(), args[2].clone());
    println!("  data array name: {}_data", args[3].clone());
    println!("  data array size: {}_size", args[3].clone());
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
    let mut cnt = 0;
    buf_writer.write_all(b"char ").unwrap();
    buf_writer.write_all(args[3].clone().as_bytes()).unwrap();
    buf_writer.write_all(b"_data[] = { ").unwrap();
    for b_res in buf_reader.bytes() {
        let b = match b_res {
            Ok(n) => n,
            Err(e) => {
                eprintln!("error: failed to read a byte : {}", e.to_string());
                exit(1);
            }
        };
        buf_writer.write_all(b.to_string().as_bytes()).unwrap();
        buf_writer.write_all(b", ").unwrap();
        cnt += 1;
    }
    buf_writer.write_all(b"};\nint ").unwrap();
    buf_writer.write_all(args[3].clone().as_bytes()).unwrap();
    buf_writer.write_all(b"_size = ").unwrap();
    buf_writer.write_all(cnt.to_string().as_bytes()).unwrap();
    buf_writer.write_all(b";\n").unwrap();
    println!("succeeded");
}
