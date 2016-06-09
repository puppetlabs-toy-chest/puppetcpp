# Check for ASCII case insensitivty
if foo != 'Foo' {
    fail incorrect
}
if 'Foo' != foo {
    fail incorrect
}

# Check for inequality
if foo == bar {
    fail incorrect
}
if bar == foo {
    fail incorrect
}

# Check for Unicode case insensitivty
if 'Τάχιστη' != 'ΤΆΧΙΣΤΗ' {
    fail incorrect
}

# Check empty tables
unless '' == "" {
    fail incorrect
}

# Check normalization
unless 'ñ' == "n\u0303" {
    fail incorrect
}

unless "N\u0303" == 'ñ' {
    fail incorrect
}
