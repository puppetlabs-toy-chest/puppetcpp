[1, 2, 3, 4, 5].each |$x| {
    if $x == 3 {
        break
    }
    notice $x
}

notice [1, 2, 3, 4, 5].filter |$x| {
    if $x == 3 {
        break
    }
    $x % 2 != 0
}

notice hello.map |$x| {
    if $x == 'l' {
        break
    }
    $x
}

notice Integer[6, 10].reduce(5) |$memo, $value| {
    if $memo > 30 {
        break
    }
    notice "memo = $memo, value = $value"
    $memo + $value
}

[1, 2, 3, 4, 5].reverse_each |$x| {
    if $x == 3 {
        break
    }
    notice $x
}

Integer[0, 100].step(5) |$x| {
    if $x > 30 {
        if $x < 40 {
            break
        }
    }
    notice $x
}

[1, 2, 3, 4, 5].each |$x| {
    notice $x.with |$y| {
        if $y == 3 {
            notice "breaking for $y"
            break
        }
        "not breaking for $y"
    }
}

# Check for break in return (should break instead of return)
function test {
    [1].each |$x| {
        return if true { break }
    }
    'did not return'
}

notice test()

# None of the following tests should output anything

# Check that break can be used in an array literal
[1].each |$x| {
    notice [1, 2, if true { break }]
}

# Check that break can be used in a hash literal
[1].each |$x| {
    notice { if true { break } => foo }
}
[1].each |$x| {
    notice { foo => if true { break } }
}

# Check for break in access expression
[1].each |$x| {
    notice [1][if true { break }]
}

# Check for break in case
[1].each |$x| {
    notice case foo {
        if true { break }: {}
    }
}

# Check for break in case
[1].each |$x| {
    notice case true {
        true: { if true { break } }
    }
}

# Check for break in if
[1].each |$x| {
    notice if if true { break } {
    }
}

# Check for break in elsif
[1].each |$x| {
    notice if false {
    } elsif if true { break } {
    }
}

# Check for break in selector
[1].each |$x| {
    notice true ? {
        if true { break } => foo
    }
}
[1].each |$x| {
    notice true ? {
        true => if true { break }
    }
}

# Check for break in unary expression
[1].each |$x| {
    notice -if true { break }
}

# Check for break in unless expression
[1].each |$x| {
    notice unless unless false { break } {
    }
}

# Check for break in a binary expression
[1].each |$x| {
    if true { break } + 1
}
[1].each |$x| {
    1 + if true { break }
}

# Check for break in next (should break instead of next)
[1, 2, 3].each |$x| {
    if $x == 1 {
        next if true { break }
    }
    notice wrong
}
