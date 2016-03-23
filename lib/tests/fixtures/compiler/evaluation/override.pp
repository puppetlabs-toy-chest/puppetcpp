define foo($foo, $bar) {
}

Foo[foo] {
    foo => bar,
    bar +> foo
}

foo { foo:
}

class bar {
    foo { bar:
        bar => []
    }
}

class baz inherits bar {
    Foo[bar] {
        foo => bar,
        bar +> foo
    }
}

include baz
