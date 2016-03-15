file { foo:
    bar => 1,
    baz => [1, 2, 3]
}

user {
    foo:
        foo => 0
    ;
    bar:
        bar => 1
    ;
    default:
        bar => 2
}

unless User[foo][foo, bar] == [0, 2] {
    fail incorrect
}
unless User[bar][bar] == 1 {
    fail incorrect
}

Resource[file] { bar:
    * => {
        foo => bar,
    },
    if => true,
    notice => lol
}

unless File[bar][foo, 'if', 'notice'] == [bar, true, lol] {
    fail incorrect
}

@file { virtual: }

@@file { exported: }
