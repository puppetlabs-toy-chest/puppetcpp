define foo($param) {
    notice "declaring file resource with param: $param"

    file { "/tmp/foo/$param":
        ensure => file
    }
}

define nested($param) {
    notice "title is ${title} and name is $name"
    foo { $param: param => nested }
}

foo { bar:
    param => bar
}

nested { baz:
    param => baz
}

Foo[bar] -> Foo[baz]
