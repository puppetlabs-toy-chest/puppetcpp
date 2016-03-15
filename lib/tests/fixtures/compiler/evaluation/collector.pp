File<| title == '/tmp/bar'|>

define foo($param) {
    notice $param
}

@file { '/tmp/bar':
    content => @(CONTENT)
        hello
        world
        |- CONTENT
}

@file { nope:
}

@foo { foo:
    param => foo
}

@@foo { bar:
    param => bar
}

@foo { baz:
    param => baz
}

Foo<| param == foo or param == bar |>

file { before: }
file { after: }

File<| title == before |> -> File<| title == after |>
