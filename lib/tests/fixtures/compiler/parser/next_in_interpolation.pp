[].each |$x| {
    # legal
    notice "${ 'foo'.each |$x| { if $x == 'o' { next } $x } }"

    # not legal
    "${ if true { next } }"
}
