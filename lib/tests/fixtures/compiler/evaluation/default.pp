$a = default
notice $a
unless $a == default {
    fail incorrect
}

if $a != default {
    fail incorrect
}

unless $a =~ Default {
    fail incorrect
}

$b = { default => foo }
unless $b[default] == foo {
    fail incorrect
}

