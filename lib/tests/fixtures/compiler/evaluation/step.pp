[].step(4) |$value| {
    fail incorrect
}

notice [1, 2, 3].step(1) |$value| {
    notice "value = $value"
}

notice [1, 2, 3, 4, 5].step(2) |$value| {
    notice "value = $value"
}

notice [1, 2, 3, 4, 5].step(5) |$value| {
    notice "value = $value"
}

notice [1, 2, 3].step(2) |$index, $value| {
    notice "index = $index, value = $value"
}

{}.step(3) |$value| {
    fail incorrect
}

notice { foo => bar, bar => baz, baz => cake }.step(2) |$value| {
    notice "value = $value"
}

notice { foo => bar, bar => baz, baz => cake }.step(2) |$key, $value| {
    notice "key = $key, value = $value"
}

notice 100.step(25) |$value| {
    notice "value = $value"
}

notice 100.step(25) |$index, $value| {
    notice "index = $index, value = $value"
}

notice Integer[5, 5].step(2) |$value| {
    notice "value = $value"
}

notice Integer[5, 10].step(2) |$value| {
    notice "value = $value"
}

notice Integer[5, 10].step(2) |$index, $value| {
    notice "index = $index, value = $value"
}

notice Enum[foo, bar, baz].step(2) |$value| {
    notice "value = $value"
}

notice Enum[foo, bar, baz].step(2) |$index, $value| {
    notice "index = $index, value = $value"
}

notice 1000.step(100).step(3) |$value| {
    notice $value
}

notice { foo => bar, bar => baz, baz => cake }.step(1).step(2) |$key, $value| {
    notice "key = $key, value = $value"
}
