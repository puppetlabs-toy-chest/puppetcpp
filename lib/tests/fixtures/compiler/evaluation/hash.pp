notice {}

$a = { a => 1, 5 => foo, [] => [], b => { a => b }, yep => nope, /baz/ => cake }
notice $a

unless $a == { a => 1, 5 => foo, [] => [], b => { a => b }, yep => nope, /baz/ => cake } {
    fail incorrect
}

if $a != { a => 1, 5 => foo, [] => [], b => { a => b }, yep => nope, /baz/ => cake } {
    fail incorrect
}

unless $a[not-there] == undef {
    fail incorrect
}

unless $a =~ Hash[Any, Any] {
    fail incorrect
}

unless $a[a] == 1 {
    fail incorrect
}

unless $a[5] == foo {
    fail incorrect
}

unless $a[[]] == [] {
    fail incorrect
}

unless $a[b] == { a => b } {
    fail incorrect
}

unless $a[yep] == nope {
    fail incorrect
}

unless $a[/baz/] == cake {
    fail incorrect
}

unless $a[a, b, yep] == [1, {a => b}, nope] {
    fail incorrect
}

$b = *$a
notice $b
unless $b == [[a, 1], [5, foo], [[], []], [b, {a => b}], [yep, nope], [/baz/, cake]] {
    fail incorrect
}

