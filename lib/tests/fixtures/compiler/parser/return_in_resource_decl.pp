function foo {
    file { foo:
        bar => if true { return }
    }
}
