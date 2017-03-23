[].each |$x| {
    File {
        bar => if true { next }
    }
}
