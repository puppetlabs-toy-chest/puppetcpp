File<| |>

Foo::Bar<| title == foo |>

Bar <| title == foo and (other == [1, 2, 3] or something != { foo => bar }) |>

Yay <| cake != lie |>


File<<| |>>

Foo::Bar<<| title == foo |>>

Bar <<| title == foo and (other == [1, 2, 3] or something != { foo => bar }) |>>

Yay <<| cake != lie and foo == default |>>

File <| foo == bar |> {
    attr => baz,
    jam +> cake
}

File <<| foo == bar |>> {
    attr => baz,
    jam +> cake
}
