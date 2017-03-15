[].each |$x| {
    # legal
    notice "${ 'foo'.each |$x| { if $x == 'o' { break } $x } }"

    # not legal
    "${ if true { break } }"
}
