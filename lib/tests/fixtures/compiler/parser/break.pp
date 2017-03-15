[1, 2, 3].each |$x| {
    if $x > 2 {
        break
    }
}

foo.map |$x| {
    if $x == 'o' {
        break()
    }
}
