unless true {
    fail incorrect
}

unless false or true {
    fail incorrect
}

unless true or false {
    fail incorrect
}

$a = unless true {
    fail incorrect
} else {
    correct
}
if $a != correct {
    fail incorrect
}

$b = unless true {
    fail incorrect
} else {
    correct
}
if $b != correct {
    fail incorrect
}
