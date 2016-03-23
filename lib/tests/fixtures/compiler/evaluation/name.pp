$a = foo
notice $a
unless $a == foo {
    fail incorrect
}

if $a != foo {
    fail incorrect
}

unless $a =~ String {
    fail incorrect
}

$b = { foo => bar }
unless $b[foo] == bar {
    fail incorrect
}

$c = foo::bar::baz
notice $c

unless $c == foo::bar::baz {
    fail incorrect
}

$d = ::foo
notice $d

unless $d == ::foo {
    fail incorrect
}

$e = ::foo_baz::bar
notice $e

unless $e == ::foo_baz::bar {
    fail incorrect
}

$f = { foo_bar => foo }
notice $f[foo_bar]

unless $f[foo_bar] == foo {
    fail incorrect
}
