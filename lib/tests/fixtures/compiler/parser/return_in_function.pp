function foo() {
    [1, 2, 3].each |$x| {
        if $x == 2 {
            return
        }
        notice $x
    }
}
