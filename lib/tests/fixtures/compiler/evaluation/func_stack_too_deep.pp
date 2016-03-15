function foo($counter) {
    if $counter == 0 {
        fail done
    }
    foo($counter - 1)
}

foo(10000)
