[].filter |$value| {
    fail incorrect
}

notice [1, 2, 3].filter |$value| {
    notice "value = $value"
    $value < 3
}

notice [1, 2, 3].filter |$index, $value| {
    notice "index = $index, value = $value"
    $value < 3
}

{}.filter |$value| {
    fail incorrect
}

notice { foo => bar, bar => baz, baz => cake }.filter |$value| {
    notice "value = $value"
    $value[0] == bar
}

notice { foo => bar, bar => baz, baz => cake }.filter |$key, $value| {
    notice "key = $key, value = $value"
    $key == bar
}

notice 5.filter |$value| {
    notice "value = $value"
    $value < 2
}

notice 5.filter |$index, $value| {
    notice "index = $index, value = $value"
    $value < 2
}

notice Integer[5, 5].filter |$value| {
    notice "value = $value"
    $value != 5
}

notice Integer[5, 10].filter |$value| {
    notice "value = $value"
    $value < 7
}

notice Integer[5, 10].filter |$index, $value| {
    notice "index = $index, value = $value"
    $value < 7
}

notice Enum[foo, bar, baz].filter |$value| {
    notice "value = $value"
    $value =~ /^b/
}

notice Enum[foo, bar, baz].filter |$index, $value| {
    notice "index = $index, value = $value"
    $value =~ /^b/
}

notice [1, 2, 3].reverse_each.filter |$value| {
    notice "value = $value"
    $value > 1
}

notice [1, 2, 3].reverse_each.filter |$index, $value| {
    notice "index = $index, value = $value"
    $value > 1
}

notice { foo => bar, bar => baz, baz => cake }.reverse_each.filter |$value| {
    notice "value = $value"
    $value[0] =~ /^b/
}

notice { foo => bar, bar => baz, baz => cake }.reverse_each.filter |$key, $value| {
    notice "key = $key, value = $value"
    $key =~ /^b/
}
