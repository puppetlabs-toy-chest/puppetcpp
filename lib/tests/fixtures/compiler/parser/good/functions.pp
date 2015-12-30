function foo {
}

function bar::baz::jam {

}

function baz() {}

function baz($foo) {
    each($foo) |$x| {
        notice $x
    }
}

function baz($foo, $bar) {

}

function something(Integer $x, String[0, 1] $y = 'X', *$rest) {
    notice hi
    $two = 1 + 1
    $three = $two + 1
}
