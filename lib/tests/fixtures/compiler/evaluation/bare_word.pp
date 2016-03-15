notice a-b
notice _foo-bar_-baz
notice _

$a = a-b
notice $a
unless $a == a-b {
    fail incorrect
}

if $a != a-b {
    fail incorrect
}

unless $a =~ String {
    fail incorrect
}

$b = { a-b => foo }
unless $b[a-b] == foo {
    fail incorrect
}

$c = _foo-bar_-baz
notice $c

unless $c == _foo-bar_-baz {
    fail incorrect
}

$d = { _ => foo }
notice $d[_]

unless $d[_] == foo {
    fail incorrect
}
