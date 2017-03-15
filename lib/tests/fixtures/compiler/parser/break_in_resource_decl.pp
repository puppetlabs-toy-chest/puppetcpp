[].each |$x| {
    file { foo:
        bar => if true { break }
    }
}
