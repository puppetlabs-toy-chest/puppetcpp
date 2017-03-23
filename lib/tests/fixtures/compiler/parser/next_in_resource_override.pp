[].each |$x| {
    File[foo] {
        bar => if true { next }
    }
}
