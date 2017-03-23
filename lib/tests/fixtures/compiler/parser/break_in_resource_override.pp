[].each |$x| {
    File[foo] {
        bar => if true { break }
    }
}
