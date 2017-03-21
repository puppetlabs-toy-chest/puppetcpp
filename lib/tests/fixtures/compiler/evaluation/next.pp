[1, 2, 3, 4, 5].each |$x| {
    if $x == 3 {
        next
    }
    notice $x
}

notice [1, 2, 3, 4, 5].filter |$x| {
    if $x == 4 {
        next true
    }
    $x % 2 != 0
}

notice hello.map |$x| {
    if $x == 'l' {
        next 'lol'
    }
    $x
}

notice Integer[6, 10].reduce(5) |$memo, $value| {
    notice "memo = $memo, value = $value"
    if $memo == 5 {
        next $memo + $value * 2
    }
    $memo + $value
}

[1, 2, 3, 4, 5].reverse_each |$x| {
    if $x == 3 {
        next
    }
    notice $x
}

Integer[0, 100].step(5) |$x| {
    if $x >= 30 {
        if $x <= 80 {
            next
        }
    }
    notice $x
}

[1, 2, 3, 4, 5].each |$x| {
    $x.with |$y| {
        if $y == 3 {
            next
        }
    }
    notice $x
}

# None of the following tests should output anything

# Check that next can be used in an array literal
[1].each |$x| {
    notice [1, 2, if true { next }]
}

# Check that next can be used in a hash literal
[1].each |$x| {
    notice { if true { next } => foo }
}
[1].each |$x| {
    notice { foo => if true { next } }
}

# Check for next in access expression
[1].each |$x| {
    notice [1][if true { next }]
}

# Check for next in case
[1].each |$x| {
    notice case foo {
        if true { next }: {}
    }
}

# Check for next in case
[1].each |$x| {
    notice case true {
        true: { if true { next } }
    }
}

# Check for next in if
[1].each |$x| {
    notice if if true { next } {
    }
}

# Check for next in elsif
[1].each |$x| {
    notice if false {
    } elsif if true { next } {
    }
}

# Check for next in selector
[1].each |$x| {
    notice true ? {
        if true { next } => foo
    }
}
[1].each |$x| {
    notice true ? {
        true => if true { next }
    }
}

# Check for next in unary expression
[1].each |$x| {
    notice -if true { next }
}

# Check for next in unless expression
[1].each |$x| {
    notice unless unless false { next } {
    }
}

# Check for next in a binary expression
[1].each |$x| {
    if true { next } + 1
}
[1].each |$x| {
    1 + if true { next }
}

# Check for next in next
[1,].each |$x| {
    if $x == 1 {
        next if true { next }
    }
}
