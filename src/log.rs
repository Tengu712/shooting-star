use std::io::{stdout, BufWriter, Write};
use std::process::exit;

/// A function to print an error message and exit with the error code 1.
pub fn ss_error(msg: &str) -> ! {
    let out = stdout();
    let mut out = BufWriter::new(out.lock());
    write!(out, "[ error ] {}", msg).unwrap();
    out.flush().unwrap();
    exit(1);
}
