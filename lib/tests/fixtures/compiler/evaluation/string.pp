$a = 'hello
  world'
notice $a

unless $a == "hello\n  world" {
    fail incorrect
}

if $a != "hello\n  world" {
    fail incorrect
}

unless $a =~ String {
    fail incorrect
}

$variable = world

$b = "hello $variable!"
notice $b
unless $b == "hello world!" {
    fail incorrect
}

unless $b =~ String {
    fail incorrect
}

$c = "hello ${variable}!"
notice $c
unless $c == "hello world!" {
    fail incorrect
}

unless $c[2] == 'l' {
    fail incorrect
}

unless $c[1, 4] == 'ello' {
    fail incorrect
}

unless $c[-1] == '!' {
    fail incorrect
}

unless $c[-6, -2] == 'world' {
    fail incorrect
}

unless $c =~ String {
    fail incorrect
}

unless "1 + 1 = ${1 + 1}" == "1 + 1 = 2" {
  fail incorrect
}

$array = [1, 2, 3]
unless "\$array[1] == ${array[1]}!" == '$array[1] == 2!' {
    fail incorrect
}
unless "\$array[1] == ${$array[1]}!" == '$array[1] == 2!' {
    fail incorrect
}

unless "filtered: ${array.filter|$x| { x == 2 }}!" != "filtered: [2]!" {
    fail incorrect
}
