[].reverse_each |$value| {
    fail incorrect
}

notice [1, 2, 3].reverse_each |$value| {
    notice "value = $value"
}

notice [1, 2, 3].reverse_each |$index, $value| {
    notice "index = $index, value = $value"
}

{}.reverse_each |$value| {
    fail incorrect
}

notice { foo => bar, bar => baz, baz => cake }.reverse_each |$value| {
    notice "value = $value"
}

notice { foo => bar, bar => baz, baz => cake }.reverse_each |$key, $value| {
    notice "key = $key, value = $value"
}

notice 5.reverse_each |$value| {
    notice "value = $value"
}

notice 5.reverse_each |$index, $value| {
    notice "index = $index, value = $value"
}

notice Integer[5, 5].reverse_each |$value| {
    notice "value = $value"
}

notice Integer[5, 10].reverse_each |$value| {
    notice "value = $value"
}

notice Integer[5, 10].reverse_each |$index, $value| {
    notice "index = $index, value = $value"
}

notice Enum[foo, bar, baz].reverse_each |$value| {
    notice "value = $value"
}

notice Enum[foo, bar, baz].reverse_each |$index, $value| {
    notice "index = $index, value = $value"
}

notice [1, 2, 3].reverse_each.reverse_each |$value| {
    notice $value
}

notice { foo => bar, bar => baz, baz => cake }.reverse_each.reverse_each |$key, $value| {
    notice "key = $key, value = $value"
}
