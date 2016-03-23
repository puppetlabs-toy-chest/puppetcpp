$a = 1234
notice $a
unless $a == 1234 {
    fail incorrect
}
if $a != 1234 {
    fail incorrect
}
unless $a =~ Integer {
    fail incorrect
}

unless $a =~ Integer[1234, 1234] {
    fail incorrect
}
if $a =~ Integer[0, 1233] {
    fail incorrect
}
if $a =~ Integer[1235] {
    fail incorrect
}

unless $a =~ Numeric {
    fail incorrect
}

$b = 0xabcd
notice $b
if $b != 43981 {
    fail incorrect
}

$c = 0777
notice $c
if $c != 511 {
    fail incorrect
}

$d = 10e100
notice $d
if $d != 10e100 {
    fail incorrect
}

unless $d =~ Float {
    fail incorrect
}

unless $d =~ Numeric {
    fail incorrect
}

$e = 123.123
notice $e

unless $e =~ Float {
    fail incorrect
}

unless $e =~ Float[100.0, 200.0] {
    fail incorrect
}
if $e =~ Float[0.0, 98.0] {
    fail incorrect
}
if $e =~ Float[200.0] {
    fail incorrect
}

unless $e =~ Numeric {
    fail incorrect
}
