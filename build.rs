fn main() {
    println!("cargo:rustc-link-lib=static=clipboard");
    println!("cargo:rustc-link-lib=X11");
    println!("cargo:rustc-link-lib=Xfixes");

    cc::Build::new()
        .file("src/c/clipboard.c")
        .compile("clipboard");
}