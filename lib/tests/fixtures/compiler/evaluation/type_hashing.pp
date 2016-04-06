unless {Any => true}[Any] {
    fail incorrect
}

type Foo = Integer
unless {Foo => true, Integer => false}[Foo] {
    fail incorrect
}

unless {Boolean => true}[Boolean] {
    fail incorrect
}

unless {Callable[Integer, default, default, Callable[String]] => true}[Callable[Integer, default, default, Callable[String]]] {
    fail incorrect
}

unless {CatalogEntry => true}[CatalogEntry] {
    fail incorrect
}

unless {Class => true}[Class] {
    fail incorrect
}
unless {Class[foo] => true}[Class[foo]] {
    fail incorrect
}

unless {Collection[5, 10] => true}[Collection[5, 10]] {
    fail incorrect
}

unless {Data => true}[Data] {
    fail incorrect
}

unless {Default => true}[Default] {
    fail incorrect
}

unless {Enum => true}[Enum] {
    fail incorrect
}

unless {Enum[foo, bar] => true}[Enum[foo, bar]] {
    fail incorrect
}

unless {Float => true}[Float] {
    fail incorrect
}

unless {Float[10, 11] => true}[Float[10, 11]] {
    fail incorrect
}

unless {Hash => true}[Hash] {
    fail incorrect
}

unless {Hash[String, Integer, 0, 10] => true}[Hash[String, Integer, 0, 10]] {
    fail incorrect
}

unless {Integer => true}[Integer] {
    fail incorrect
}

unless {Integer[0, 100] => true}[Integer[0, 100]] {
    fail incorrect
}

unless {Iterable => true}[Iterable] {
    fail incorrect
}
unless {Iterable[String] => true}[Iterable[String]] {
    fail incorrect
}

unless {Iterator => true}[Iterator] {
    fail incorrect
}
unless {Iterator[String] => true}[Iterator[String]] {
    fail incorrect
}

unless {NotUndef => true}[NotUndef] {
    fail incorrect
}
unless {NotUndef[Boolean] => true}[NotUndef[Boolean]] {
    fail incorrect
}

unless {Numeric => true}[Numeric] {
    fail incorrect
}

unless {Optional => true}[Optional] {
    fail incorrect
}
unless {Optional[String] => true}[Optional[String]] {
    fail incorrect
}

unless {Pattern => true}[Pattern] {
    fail incorrect
}
unless {Pattern[/foo/] => true}[Pattern[/foo/]] {
    fail incorrect
}

unless {Regexp => true}[Regexp] {
    fail incorrect
}
unless {Regexp[/foo/] => true}[Regexp[/foo/]] {
    fail incorrect
}

unless {Resource => true}[Resource] {
    fail incorrect
}
unless {Resource[foo, bar] => true}[Resource[foo, bar]] {
    fail incorrect
}

unless {Runtime => true}[Runtime] {
    fail incorrect
}
unless {Runtime[foo, bar] => true}[Runtime[foo, bar]] {
    fail incorrect
}

unless {Scalar => true}[Scalar] {
    fail incorrect
}

unless {String => true}[String] {
    fail incorrect
}
unless {String[0, 10] => true}[String[0, 10]] {
    fail incorrect
}

unless {Struct => true}[Struct] {
    fail incorrect
}
unless {Struct[{foo => String}] => true}[Struct[{foo => String}]] {
    fail incorrect
}

unless {Tuple => true}[Tuple] {
    fail incorrect
}
unless {Tuple[String, Integer, 2, 2] => true}[Tuple[String, Integer, 2, 2]] {
    fail incorrect
}

unless {Type => true}[Type] {
    fail incorrect
}
unless {Tuple[Integer[0, 10]] => true}[Tuple[Integer[0, 10]]] {
    fail incorrect
}

unless {Undef => true}[Undef] {
    fail incorrect
}

unless {Variant => true}[Variant] {
    fail incorrect
}
unless {Variant[String, Integer] => true}[Variant[String, Integer]] {
    fail incorrect
}
if {Variant[String, Integer] => true}[Variant[Integer, String]] {
    fail incorrect
}
