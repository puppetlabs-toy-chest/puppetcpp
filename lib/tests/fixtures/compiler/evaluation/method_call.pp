function test1() {
    notice hi
}

function test2($name = peter) {
    notice $name
}

function test3($param, $optional = peter, *$captures) {
    notice $param, $optional, $captures
}

test1()
test2()
john.test2()
1.test3()
1.test3(john)
1.test3(john, foo, bar, baz)
