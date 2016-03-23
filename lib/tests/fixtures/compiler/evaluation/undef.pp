$a = undef
notice $a
unless $a == undef {
    fail incorrect
}

if $a != undef {
    fail incorrect
}

unless $a =~ Undef {
    fail incorrect
}

$b = { undef => foo }
unless $b[undef] == foo {
    fail incorrect
}

