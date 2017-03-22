function foo {
    File[foo] {
        bar => if true { return }
    }
}
