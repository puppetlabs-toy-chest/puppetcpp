function foo() {
    "${ 'foo'.each |$x| { if $x == 'o' { return } $x } }"
}
