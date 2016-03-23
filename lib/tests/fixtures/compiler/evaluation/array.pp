notice []

$a = [1, foo, [], { a => b }, undef, /baz/]
notice $a

unless $a == [1, foo, [], { a => b }, undef, /baz/] {
    fail incorrect
}

if $a != [1, foo, [], { a => b }, undef, /baz/] {
    fail incorrect
}

unless $a =~ Array {
    fail incorrect
}

unless $a[2] == [] {
    fail incorrect
}

unless $a[1, 4] == [foo, [], { a => b }, undef] {
    fail incorrect
}

unless $a[-1] == /baz/ {
    fail incorrect
}

unless $a[-6, -2] == [1, foo, [], { a => b }, undef] {
    fail incorrect
}

$b = { [1, 2, 3] => foo }
unless $b[[1, 2, 3]] == foo {
    fail incorrect
}

$c = [*[1, *[2], 3]]
notice $c

unless $c == [1, 2, 3] {
    fail incorrect
}

unless $c =~ Array[Integer] {
    fail incorrect
}

notice *[foo]
