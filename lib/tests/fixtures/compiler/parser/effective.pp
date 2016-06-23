$a = 1 + 2
$b = if true { 1 + 1 }
foo.each |$x| {
    notice $x
}
$c = foo.each |$x| {}[0]

if true {
    notice hi
    2 + 2
}

class foo {
    $a = 1 + 1
}
include foo

define bar {
    $b = 2 + 2
}

bar { baz:
}

file { '/tmp': }

notice 'statement call'

node default {

}

$z = -($y = 5)

function foo {
    10 - 5
}

notice foo()

Integer.new('5')

File {
    foo => bar
}

File['/tmp'] {
    baz => jam
}

Foo produces Bar {}
Bar produces Foo {}
application foo {}
site {}

File['/tmp'] -> Baz<||>

File<||>

type MyInteger = Integer
