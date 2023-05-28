use std::io::{stdout, BufWriter, Write};
use std::process::exit;

/// A function to print an error message and exit with the error code 1.
#[inline(always)]
pub fn ss_error(msg: &str) -> ! {
    let out = stdout();
    let mut out = BufWriter::new(out.lock());
    write!(out, "[ error ] {}\n", msg).unwrap();
    out.flush().unwrap();
    exit(1);
}

/// A function to print a warning message.
#[inline(always)]
pub fn ss_warning(msg: &str) {
    let out = stdout();
    let mut out = BufWriter::new(out.lock());
    write!(out, "[ warn  ] {}\n", msg).unwrap();
    out.flush().unwrap();
}

/// A function to print a information message.
#[inline(always)]
pub fn ss_info(msg: &str) {
    let out = stdout();
    let mut out = BufWriter::new(out.lock());
    write!(out, "[ info  ] {}\n", msg).unwrap();
    out.flush().unwrap();
}

/// A function to print a debug message.
#[inline(always)]
pub fn ss_debug(msg: &str) {
    let out = stdout();
    let mut out = BufWriter::new(out.lock());
    write!(out, "[ debug ] {}\n", msg).unwrap();
    out.flush().unwrap();
}
