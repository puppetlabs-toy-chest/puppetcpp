[].map |$value| {
    fail incorrect
}

notice [1, 2, 3].map |$value| {
    notice "value = $value"
    $value + 1
}

notice [1, 2, 3].map |$index, $value| {
    notice "index = $index, value = $value"
    $value + 1
}

{}.map |$value| {
    fail incorrect
}

notice { foo => bar, bar => baz, baz => cake }.map |$value| {
    notice "value = $value"
    $value[0]
}

notice { foo => bar, bar => baz, baz => cake }.map |$key, $value| {
    notice "key = $key, value = $value"
    $key
}

notice 5.map |$value| {
    notice "value = $value"
    $value + 1
}

notice 5.map |$index, $value| {
    notice "index = $index, value = $value"
    $value + 1
}

notice Integer[5, 5].map |$value| {
    notice "value = $value"
    $value + 1
}

notice Integer[5, 10].map |$value| {
    notice "value = $value"
    $value + 1
}

notice Integer[5, 10].map |$index, $value| {
    notice "index = $index, value = $value"
    $value + 1
}

notice Enum[foo, bar, baz].map |$value| {
    notice "value = $value"
    "value = $value"
}

notice Enum[foo, bar, baz].map |$index, $value| {
    notice "index = $index, value = $value"
    "value = $value"
}

notice [1, 2, 3].reverse_each.map |$value| {
    notice "value = $value"
    $value - 1
}

notice [1, 2, 3].reverse_each.map |$index, $value| {
    notice "index = $index, value = $value"
    $value - 1
}

notice { foo => bar, bar => baz, baz => cake }.reverse_each.map |$value| {
    notice "value = $value"
    $value[0]
}

notice { foo => bar, bar => baz, baz => cake }.reverse_each.map |$key, $value| {
    notice "key = $key, value = $value"
    $key
}
