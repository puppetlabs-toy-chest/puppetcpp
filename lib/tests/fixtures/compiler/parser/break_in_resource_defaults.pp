[].each |$x| {
    File {
        bar => if true { break }
    }
}
