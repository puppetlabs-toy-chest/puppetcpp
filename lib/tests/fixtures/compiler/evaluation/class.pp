class foo {
    notice $name
    $x = $y = $name
}

class bar inherits foo {
    notice $name
    $x = $name
}

notice $foo::x, $foo::y

include bar
include bar

unless Class[bar] =~ Class {
    fail incorrect
}

class baz(String $foo, String $bar = baz) {
    notice $foo, $bar
    $x = $name
}

class { baz:
    foo => bar
}

notice $foo::x, $foo::y
notice $bar::x, $bar::y
notice $baz::foo, $baz::bar, $baz::x

stage { foo:
}

class in::stage::foo {
    notice $stage
}

class { in::stage::foo:
    stage => foo
}
