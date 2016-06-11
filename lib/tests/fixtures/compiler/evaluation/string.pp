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

if 'ฉันกินกระจกได้'[1, 4] != 'นกินก' {
    fail incorrect
}

unless 'ฉันกินกระจกได้'[-1] == 'ด้' {
    fail incorrect
}

unless 'foObArBaz' == 'foobarbaz' {
    fail incorrect
}

if 'foObArBaz' != 'FOOBARBAZ' {
    fail incorrect
}

unless 'çöğiü' == 'ÇÖĞIÜ' {
    fail incorrect
}

unless 'ฉันกินกระจกได' != 'ராமானுஜன்' {
    fail incorrect
}

unless 'それは私を傷つけません' == 'それは私を傷つけません' {
    fail incorrect
}

unless "NO\u0303" == "nõ" {
    fail incorrect
}

unless a < z {
    fail incorrect
}

if a < a or z < a {
    fail incorrect
}

unless a <= z and a <= a {
    fail incorrect
}

if z <= a {
    fail incorrect
}

unless z > a {
    fail incorrect
}

if z > z or a > z {
    fail incorrect
}

unless z >= a and z >= z {
    fail incorrect
}

if a >= z {
    fail incorrect
}

unless foo in 'FoOBar' {
    fail incorrect
}

if baz in 'FOOBAR' {
    fail incorrect
}

unless 'ÇÖĞIÜ' in '私をçöğiüけま' {
    fail incorrect
}

if 'ฉัน' in '私をçöğiüけま' {
    fail incorrect
}

unless "NO\u0303" in "nõ" {
    fail incorrect
}

if 'O' in "NO\u0303" {
    fail incorrect
}

if "\u0303" in "NO\u0303" {
    fail incorrect
}
