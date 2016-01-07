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

notice assert_type(Integer[0], 1)
notice assert_type('Integer[1]', 1)
notice assert_type('String[4, 4]', 'foo') |$expected, $actual| {
    warning "expected $expected but was given $actual"
    'bar'
}
