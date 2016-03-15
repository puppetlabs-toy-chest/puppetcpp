notice //

$a = /(foo)(bar)/
notice $a

unless $a == /(foo)(bar)/ {
    fail incorrect
}

if $a != /(foo)(bar)/ {
    fail incorrect
}

if foobar =~ $a {
    notice $0, $1, $2
    unless $0 == foobar {
        fail incorrect
    }

    unless $1 == foo {
        fail incorrect
    }

    unless $2 == bar {
        fail incorrect
    }
} else {
    fail incorrect
}

if $0 or $1 or $2 {
    fail incorrect
}


$b = { $a => foo }
unless $b[$a] == foo {
    fail incorrect
}
