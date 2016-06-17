class foo {
}

class bar::baz::jam {

}

class baz() {}

class baz($foo) {
    each($foo) |$x| {
        notice $x
    }
}

class baz($foo, $bar) {

}

class ntp(Integer $x, String[0, 1] $y = 'X') {

}

class ntp2(Integer $x, String[0, 1] $y = 'X') inherits bar::baz::jam {

}

class ntp3(Integer $x, String[0, 1] $y = 'X') inherits bar::baz::jam {
    notice hi
    $two = 1 + 1
    $three = $two + 1
}
