$foo = bar

notice case $foo {
    foo: { fail wrong }
    bar: { correct }
    default: { fail wrong }
}

notice case $foo {
    foo: { fail wrong }
    default: { correct }
}

notice case $foo {
    [bar]: { fail wrong }
    default: { correct }
}

notice case $foo {
    *[bar]: { correct }
    default: { fail wrong }
}
