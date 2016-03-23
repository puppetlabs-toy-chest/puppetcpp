[].each |$value| {
    fail incorrect
}

notice [1, 2, 3].each |$value| {
    notice "value = $value"
}

notice [1, 2, 3].each |$index, $value| {
    notice "index = $index, value = $value"
}

{}.each |$value| {
    fail incorrect
}

notice { foo => bar, bar => baz, baz => cake }.each |$value| {
    notice "value = $value"
}

notice { foo => bar, bar => baz, baz => cake }.each |$key, $value| {
    notice "key = $key, value = $value"
}

notice 5.each |$value| {
    notice "value = $value"
}

notice 5.each |$index, $value| {
    notice "index = $index, value = $value"
}

notice Integer[5, 5].each |$value| {
    notice "value = $value"
}

notice Integer[5, 10].each |$value| {
    notice "value = $value"
}

notice Integer[5, 10].each |$index, $value| {
    notice "index = $index, value = $value"
}

notice Enum[foo, bar, baz].each |$value| {
    notice "value = $value"
}

notice Enum[foo, bar, baz].each |$index, $value| {
    notice "index = $index, value = $value"
}

notice [1, 2, 3].reverse_each.each |$value| {
    notice "value = $value"
}

notice [1, 2, 3].reverse_each.each |$index, $value| {
    notice "index = $index, value = $value"
}

notice { foo => bar, bar => baz, baz => cake }.reverse_each.each |$value| {
    notice "value = $value"
}

notice { foo => bar, bar => baz, baz => cake }.reverse_each.each |$key, $value| {
    notice "key = $key, value = $value"
}
