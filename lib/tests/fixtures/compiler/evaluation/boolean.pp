$a = true
notice $a

unless $a {
    fail incorrect
}

if !$a {
    fail incorrect
}

unless $a =~ Boolean {
    fail incorrect
}

$b = false
notice $b

unless !$b {
    fail incorrect
}

if $b {
    fail incorrect
}

unless $b =~ Boolean {
    fail incorrect
}

notice true or fail(wrong)
notice false and fail(wrong)
