if [].reduce |$memo, $value| {
    fail incorrect
} {
    fail incorrect
}

notice [1, 2, 3].reduce |$memo, $value| {
    notice "memo = $memo, value = $value"
    $memo + $value
}

notice [2, 3].reduce(1) |$memo, $value| {
    notice "memo = $memo, value = $value"
    $memo + $value
}

if {}.reduce |$memo, $value| {
    fail incorrect
} {
    fail incorrect
}

notice { foo => bar, bar => baz, baz => cake }.reduce |$memo, $value| {
    notice "memo = $memo, value = $value"
    $memo << $value[0] << $value[1]
}

notice { bar => baz, baz => cake }.reduce([foo, bar]) |$memo, $value| {
    notice "memo = $memo, value = $value"
    $memo << $value[0] << $value[1]
}

notice 5.reduce |$memo, $value| {
    notice "memo = $memo, value = $value"
    $memo + $value
}

notice Integer[5, 5].reduce |$memo, $value| {
    fail incorrect
}

notice Integer[5, 5].reduce(1) |$memo, $value| {
    notice "memo = $memo, value = $value"
    $memo + $value
}

notice Integer[5, 10].reduce |$memo, $value| {
    notice "memo = $memo, value = $value"
    $memo + $value
}

notice Integer[6, 10].reduce(5) |$memo, $value| {
    notice "memo = $memo, value = $value"
    $memo + $value
}

notice Enum[foo, bar, baz].reduce |$memo, $value| {
    notice "memo = $memo, value = $value"
    "$memo$value"
}

notice Enum[bar, baz].reduce(foo) |$memo, $value| {
    notice "memo = $memo, value = $value"
    "$memo$value"
}

notice [1, 2, 3].reverse_each.reduce |$memo, $value| {
    notice "memo = $memo, value = $value"
    $memo + $value
}

notice [1, 2].reverse_each.reduce(3) |$memo, $value| {
    notice "memo = $memo, value = $value"
    $memo + $value
}

notice { foo => bar, bar => baz, baz => cake }.reverse_each.reduce |$memo, $value| {
    notice "value = $value"
    $memo << $value[0] << $value[1]
}

notice { foo => bar, bar => baz }.reverse_each.reduce([baz, cake]) |$memo, $value| {
    notice "key = $key, value = $value"
    $memo + $value
}
