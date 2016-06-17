define foo {
}

define bar::baz::jam {

}

define baz() {}

define baz($foo) {
    each($foo) |$x| {
        notice $x
    }
}

define baz($foo, $bar) {

}

define ntp(Integer $x, String[0, 1] $y = 'X') {

}

define ntp3(Integer $x, String[0, 1] $y = 'X') {
    notice hi
    $two = 1 + 1
    $three = $two + 1
}
