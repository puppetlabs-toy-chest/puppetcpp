[].each |$x| {
    file { foo:
        bar => if true { next }
    }
}
