file { foo:
    bar => 1,
    baz => $baz
}

user {
    foo:
        foo => 0
    ;
    bar:
        bar => 1
    ;
    default:
        baz => 2
}

Resource[foo] { bar:
    * => {
        foo => bar,
    },
    if => true,
    notice => lol
}

@virtual { foo: }

@@exported { foo: }

class { 'my_class':
  something => undef
}
