fn main() {
    let args = std::env::args().collect::<Vec<String>>();
    if args.len() < 3 {
        println!("cpin <in-filepath> <out-filepath>");
        return;
    }
    match std::fs::copy(&args[1], &args[2]) {
        Ok(_) => println!("cpin: copied {} to {}", args[1], args[2]),
        Err(e) => eprintln!("cpin error: failed to copied {} to {} : {}", args[1], args[2], e.to_string()),
    }
}
