notice [1, 2, 3, 4].map |$x| {
    if $x == 1 {
        next foo
    }
    if $x == 2 {
        next(bar)
    }
    if $x == 3 {
        next()
    }
    next
}

