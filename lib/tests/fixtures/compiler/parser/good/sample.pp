$undef = undef
$array = [3,2,1]
$default = default
$boolean = false or true
$number = 123.456
$name = '¯\_(ツ)_/¯'
$title = "${name}"
$splats = {
    foo => baz
}
$type = File

file {
    $title:
        ensure => present,
        * => $splats
    ;
    other:
    ;
    default:
        foo => $number
}

if $boolean {
    File[$title] -> File[other]
} elsif !boolean {
    fail wrong
} else {
    fail wrong
}

unless $boolean {
    fail wrong
}

@user { foo:
    attribute => something
    ;
    bar:
}

User<| title == foo and attribute == something |>

class foo {
    $title.notice
}

class bar::baz(String $param = included_from_class) inherits foo {
    Resource[$type] { $param:
    }
}

include bar::baz

define my_type(String[10, 20] $param) {
    Resource[$type] { $param:
        original_title => $title
    }
}

my_type { my_title:
    param => included_from_dt
}


if $type ? {
    File => correct,
    default => wrong
} != correct {
    fail wrong
}

case $name {
    /¯\\_\(ツ\)_\/¯/: {
        notice lol
    }
    default: {
        wrong.fail
    }
}

# Test various functions
alert(hi)

if assert_type(Integer[0], 1) != 1 {
    fail incorrect
}
if assert_type(Integer[0], 10) |$expected, $actual| {
    incorrect
} != 10 {
    fail incorrect
}
if assert_type('Integer[1]', 1) != 1 {
    fail incorrect
}
if assert_type('String[4, 4]', 'foo') |$expected, $actual| {
    warning "expected $expected but was given $actual"
    correct
} != correct {
    fail incorrect
}

class contained {
}
class required {
}
class container {
    contain contained
    require required
}
include container

crit(hi)

debug(hi)

unless defined('$type') {
    fail incorrect
}
if defined('$notdefined') {
    fail incorrect
}

if Integer[0, 5].filter |$x| {
    $x < 3
} != [0, 1, 2] {
    fail incorrect
}
if [1, 2, 3].filter |$x| {
    $x < 2
} != [1] {
    fail incorrect
}
if [1, 2, 3].filter |$i, $x| {
    $i + $x < 2
} != [1] {
    fail incorrect
}
if { a => b, c => d, e => f }.filter |$x| {
    $x[1] == d
} != { c => d } {
    fail incorrect
}

info(hi)

$template = @(TEMPLATE)
    <%| $param |-%>
    <%= $param -%>
    |- TEMPLATE
if inline_epp($template, { param => correct }) != correct {
    fail incorrect
}
if inline_epp("<%= $default %>") != 'default' {
    fail incorrect
}

if Integer[0, 5].map |$x| {
    $x < 3
} != [true, true, true, false, false, false] {
    fail incorrect
}
if [1, 2, 3].map |$x| {
    $x < 2
} != [true, false, false] {
    fail incorrect
}
if [1, 2, 3].map |$i, $x| {
    $i + $x < 2
} != [true, false, false] {
    fail incorrect
}
if { a => b, c => d, e => f }.map |$x| {
    $x[1]
} != [b, d, f] {
    fail incorrect
}

notice hi

@file { 'virt':
}

realize File[virt]

if Integer[0, 5].reduce |$memo, $x| {
    $memo + $x
} != 0 + 1 + 2 + 3 + 4 + 5 {
    fail incorrect
}
if [1, 2, 3].reduce |$memo, $x| {
    $memo + $x
} != 1 + 2 + 3 {
    fail incorrect
}
if { a => b, c => d, e => f }.reduce('') |$memo, $x| {
    "$memo${x[0]}"
} != ace {
    fail incorrect
}

if split(abc, '') != [a, b, c] {
    fail incorrect
}
if split('a b c', ' ') != [a, b, c] {
    fail incorrect
}

if split(abc, //) != [a, b, c] {
    fail incorrect
}
if split('a b c', / /) != [a, b, c] {
    fail incorrect
}

if split(abc, Regexp) != [a, b, c] {
    fail incorrect
}
if split('a b c', Regexp[' ']) != [a, b, c] {
    fail incorrect
}

tag silly_tag

unless tagged(silly_tag) {
    fail incorrect
}

if tagged(not_a_tag) {
    fail incorrect
}

if versioncmp('2.6-1', '2.4.5') <= 0 {
    fail incorrect
}
if versioncmp('2.4.5a', '2.6.5') >= 0 {
    fail incorrect
}
if versioncmp('1.2.3', '1.2.3') != 0 {
    fail incorrect
}

warning(hi)

with(1, '2', [3], { 4 => 5}) |$a, $b, $c, $d| {
    unless $a == 1 {
        fail incorrect
    }
    unless $b == '2' {
        fail incorrect
    }
    unless $c == [3] {
        fail incorrect
    }
    unless $d == { 4 => 5 } {
        fail incorrect
    }
}

class class_ref_param($param1, $param2 = "${param1}bar") {
}

class { class_ref_param:
    param1 => foo
}

unless $class_ref_param::param2 == 'foobar' {
    fail incorrect
}

define dt_ref_param($param1, $param2 = "${param1}bar") {
    notice $param2
}

dt_ref_param { foo:
    param1 => foo
}

class param_math_scope($a = foo =~ /foo/, $b = $0) {
}

include param_math_scope

unless $param_math_scope::a == true and $param_math_scope::b == undef {
    fail incorrect
}

# Operator tests
if 2 / 2 != 1 {
    fail incorrect
}
if 1 / 2 != 0 {
    fail incorrect
}
if 1 / 2.0 != 0.5 {
    fail incorrect
}
if 2.0 / 1.0 != 2.0 {
    fail incorrect
}
if 1.0 / 5.0 != 0.2 {
    fail incorrect
}
if 1.0 / 5 != 0.2 {
    fail incorrect
}

if 1 > 2 {
    fail incorrect
}
unless 2 > 1 {
    fail incorrect
}
if 1 > 1 {
    fail incorrect
}
if 3.0 > 4 {
    fail incorrect
}
unless 4 > 3.0 {
    fail incorrect
}
if 3.0 > 3 {
    fail incorrect
}
if 3.0 > 4.0 {
    fail incorrect
}
unless 4.0 > 3.0 {
    fail incorrect
}
if 3.0 > 3.0 {
    fail incorrect
}
if bar > foo {
    fail incorrect
}
unless foo > bar {
    fail incorrect
}
if foo > foo {
    fail incorrect
}
if String > String[0, 0] {
    fail incorrect
}
unless String[0, 0] > String {
    fail incorrect
}
if String[0, 0] > String[0, 0] {
    fail incorrect
}

if 1 >= 2 {
    fail incorrect
}
unless 2 >= 1 {
    fail incorrect
}
unless 1 >= 1 {
    fail incorrect
}
if 3.0 >= 4 {
    fail incorrect
}
unless 4 >= 3.0 {
    fail incorrect
}
unless 3.0 >= 3 {
    fail incorrect
}
if 3.0 >= 4.0 {
    fail incorrect
}
unless 4.0 >= 3.0 {
    fail incorrect
}
unless 3.0 >= 3.0 {
    fail incorrect
}
if bar >= foo {
    fail incorrect
}
unless foo >= bar {
    fail incorrect
}
unless foo >= foo {
    fail incorrect
}
if String >= String[0, 0] {
    fail incorrect
}
unless String[0, 0] >= String {
    fail incorrect
}
unless String[0, 0] >= String[0, 0] {
    fail incorrect
}

unless foo in foobar {
    fail incorrect
}
if bar in baz {
    fail incorrect
}
unless /.*oo.*/ in foobar {
    fail incorrect
}
if /.*oo.*/ in baz {
    fail incorrect
}
unless String in [1, foo, 3] {
    fail incorrect
}
if String in [1, 2, 3] {
    fail incorrect
}
unless foo in [1, foo, 3] {
    fail incorrect
}
if foo in [1, 2, 3] {
    fail incorrect
}
unless String in { 1 => 2, foo => bar, 5 => 6 } {
    fail incorrect
}
if String in { 1 => 2, 3 => 4, 5 => 6 } {
    fail incorrect
}
unless foo in { 1 => 2, foo => bar, 5 => 6 } {
    fail incorrect
}
if foo in { 1 => 2, 3 => 4, 5 => 6 } {
    fail incorrect
}
if 5 in 2 {
    fail incorrect
}

unless 1 << 3 == 8 {
    fail incorrect
}
unless -8 << -3 == -1 {
    fail incorrect
}
unless 8 << -3 == 1 {
    fail incorrect
}
unless -1 << 3 == -8 {
    fail incorrect
}
unless [1, 2] << 3 == [1, 2, 3] {
    fail incorrect
}

unless 1 < 2 {
    fail incorrect
}
if 2 < 1 {
    fail incorrect
}
if 1 < 1 {
    fail incorrect
}
unless 3.0 < 4 {
    fail incorrect
}
if 4 < 3.0 {
    fail incorrect
}
if 3.0 < 3 {
    fail incorrect
}
unless 3.0 < 4.0 {
    fail incorrect
}
if 4.0 < 3.0 {
    fail incorrect
}
if 3.0 < 3.0 {
    fail incorrect
}
unless bar < foo {
    fail incorrect
}
if foo < bar {
    fail incorrect
}
if foo < foo {
    fail incorrect
}
unless String < String[0, 0] {
    fail incorrect
}
if String[0, 0] < String {
    fail incorrect
}
if String[0, 0] < String[0, 0] {
    fail incorrect
}

unless 1 <= 2 {
    fail incorrect
}
if 2 <= 1 {
    fail incorrect
}
unless 1 <= 1 {
    fail incorrect
}
unless 3.0 <= 4 {
    fail incorrect
}
if 4 <= 3.0 {
    fail incorrect
}
unless 3.0 <= 3 {
    fail incorrect
}
unless 3.0 <= 4.0 {
    fail incorrect
}
if 4.0 <= 3.0 {
    fail incorrect
}
unless 3.0 <= 3.0 {
    fail incorrect
}
unless bar <= foo {
    fail incorrect
}
if foo <= bar {
    fail incorrect
}
unless foo <= foo {
    fail incorrect
}
unless String <= String[0, 0] {
    fail incorrect
}
if String[0, 0] <= String {
    fail incorrect
}
unless String[0, 0] <= String[0, 0] {
    fail incorrect
}

if false and false {
    fail incorrect
}
if false and true {
    fail incorrect
}
if true and false {
    fail incorrect
}
unless true and true {
    fail incorrect
}

if !true {
    fail incorrect
}
unless !false {
    fail incorrect
}

if false or false {
    fail incorrect
}
unless false or true {
    fail incorrect
}
unless true or false {
    fail incorrect
}
unless true or true {
    fail incorrect
}
