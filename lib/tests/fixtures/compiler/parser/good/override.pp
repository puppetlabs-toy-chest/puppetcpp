File[foo] {
}

File[foo] {
    bar => baz,
    jam +> cake
}

File[foo, bar] {
    baz => yep
}

$foo {
    foo => undef
}

$foo[bar] {
    baz => 1,
    if => true,
    notice +> lol
}
