# Iterable tests

notice Iterable
notice Iterable[String]

if undef =~ Iterable {
    fail incorrect
}

unless [1, 2, 3] =~ Iterable {
    fail incorrect
}

unless [1, 2, 3] =~ Iterable[Integer] {
    fail incorrect
}

if [1, 2, 3] =~ Iterable[String] {
    fail incorrect
}

unless { foo => 1, bar => 2, baz => 3 } =~ Iterable {
    fail incorrect
}

unless { foo => 1, bar => 2, baz => 3 } =~ Iterable[Tuple[String, Integer]] {
    fail incorrect
}

if { foo => 1, bar => 2, baz => 3 } =~ Iterable[Tuple[Integer, Integer]] {
    fail incorrect
}

unless 5 =~ Iterable {
    fail incorrect
}

unless 1 =~ Iterable {
    fail incorrect
}

unless 0 =~ Iterable {
    fail incorrect
}

if -5 =~ Iterable {
    fail incorrect
}

unless 5 =~ Iterable[Integer[0, 4]] {
    fail incorrect
}

if 5 =~ Iterable[Integer[1, 4]] {
    fail incorrect
}

if 5 =~ Iterable[Integer[0, 5]] {
    fail incorrect
}

unless Integer[10, 100] =~ Iterable {
    fail incorrect
}

unless Integer[-10, 10] =~ Iterable {
    fail incorrect
}

if Integer[default, 10] =~ Iterable {
    fail incorrect
}

if Integer[10, default] =~ Iterable {
    fail incorrect
}

unless Integer[0, 10] =~ Iterable[Integer[0, 10]] {
    fail incorrect
}

unless Enum[foo, bar] =~ Iterable {
    fail incorrect
}

unless Enum[foo, bar] =~ Iterable[Enum[foo, bar]] {
    fail incorrect
}

# TODO: add check for iterators in the future
