# Iterator tests

notice Iterator
notice Iterator[String]

notice [1, 2, 3].reverse_each
notice { foo => bar, bar => baz, baz => cake }.reverse_each

if undef =~ Iterator {
    fail incorrect
}

unless [1, 2, 3].reverse_each =~ Iterator {
    fail incorrect
}

unless [1, 2, 3].reverse_each =~ Iterator[Integer] {
    fail incorrect
}

if [1, 2, 3].reverse_each =~ Iterator[String] {
    fail incorrect
}

unless [1, 2, 3].reverse_each == [1, 2, 3].reverse_each {
    fail incorrect
}

if [1, 2, 3].reverse_each != [1, 2, 3].reverse_each {
    fail incorrect
}
