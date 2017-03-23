function foo($x) {
    if $x == 1 {
        return one
    }
    if $x == 2 {
        return two
    }
    return none
}

notice foo(1)
notice foo(2)
notice foo(3)

function bar($x) {
    $x.with |$y| {
        if $x == 1 {
            return one
        }
    }
    if $x == 2 {
        [1].each |$x| {
            $x.with |$y| {
                return two
            }
        }
    }
    if $x == 2 {
        $x.with |$y| {
            [$y].each |$x| {
                $x.filter |$y| {
                    return two
                }
            }
        }
    }
    if $x == 3 {
        [$x].map |$x| {
            return three
        }
    }
    if $x == 4 {
        [1, 2, 3].reduce |$memo, $x| {
            return four
        }
    }
    if $x == 5 {
        [$x].reverse_each |$x| {
            return five
        }
    }
    if $x == 6 {
        [$x].step(1) |$x| {
            return six
        }
    }
    if $x == 7 {
        { foo => bar }.filter |$x| {
            return seven
        }
    }
    none
}

notice bar(1)
notice bar(2)
notice bar(3)
notice bar(4)
notice bar(5)
notice bar(6)
notice bar(7)

class klass($x) {
    notice 'before return in class'
    if $x {
        return
    }
    notice 'after return in class'
}

class { klass:
    x => true
}

define defined_type($x) {
    notice 'before return in defined type'
    $a = $x.with |$y| {
        if $y {
            return
        }
    }
    notice 'after return in defined type'
}

defined_type { foo:
    x => true
}

defined_type { bar:
    x => false
}

# None of the following tests should output anything

# Check that return can be used in an array literal
function test1 {
    [1].each |$x| {
        notice [1, 2, if true { return }]
    }
}
test1()

# Check that return can be used in a hash literal
function test2 {
    [1].each |$x| {
        notice { if true { return } => foo }
    }
}
test2()
function test3 {
    [1].each |$x| {
        notice { foo => if true { return } }
    }
}
test3()

# Check for return in access expression
function test4 {
    [1].each |$x| {
        notice [1][if true { return }]
    }
}
test4()

# Check for return in case
function test5 {
    [1].each |$x| {
        notice case foo {
            if true { return }: {}
        }
    }
}
test5()

# Check for return in case
function test6 {
    [1].each |$x| {
        notice case true {
            true: { if true { return } }
        }
    }
}
test6()

# Check for return in if
function test7 {
    [1].each |$x| {
        notice if if true { return } {
        }
    }
}
test7()

# Check for return in elsif
function test8 {
    [1].each |$x| {
        notice if false {
        } elsif if true { return } {
        }
    }
}
test8()

# Check for return in selector
function test9 {
    [1].each |$x| {
        notice true ? {
            if true { return } => foo
        }
    }
}
test9()
function test10 {
    [1].each |$x| {
        notice true ? {
            true => if true { return }
        }
    }
}
test10()

# Check for return in unary expression
function test11 {
    [1].each |$x| {
        notice -if true { return }
    }
}
test11()

# Check for return in unless expression
function test12 {
    [1].each |$x| {
        notice unless unless false { return } {
        }
    }
}
test12()

# Check for return in a binary expression
function test13 {
    [1].each |$x| {
        if true { return } + 1
    }
}
test13()
function test14 {
    [1].each |$x| {
        1 + if true { return }
    }
}
test14()

# Check for return in return
function test15 {
    [1].each |$x| {
        if $x == 1 {
            return if true { return }
        }
    }
}
test15()
