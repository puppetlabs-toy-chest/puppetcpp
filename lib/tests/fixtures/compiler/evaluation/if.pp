if false {
    fail incorrect
}

if true and false {
    fail incorrect
}

if false and true {
    fail incorrect
}

$a = if false {
    fail incorrect
} else {
    correct
}
if $a != correct {
    fail incorrect
}

$b = if false {
    fail incorrect
} elsif true {
    correct
} else {
    fail incorrect
}
if $b != correct {
    fail incorrect
}

$c = if true {
    correct
} elsif true {
    fail incorrect
} else {
    fail incorrect
}
if $c != correct {
    fail incorrect
}
