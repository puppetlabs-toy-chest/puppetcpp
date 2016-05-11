node 'foo' {

}

node foo {

}

node foo.bar.com {

}

node foo, 'bar', foo.bar.com {

}

node /^foo(.*)bar$/ {

}

node default {
    notice hi
}

node 'foo', bar, default, foo.bar.com {
    foo()
    $a = 1 + 1
}
