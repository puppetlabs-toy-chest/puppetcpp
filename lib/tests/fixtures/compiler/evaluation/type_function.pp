function display_type($x) {
    notice type($x, detailed)
    notice type($x, reduced)
    notice type($x, generalized)

    if type($x) != type($x, detailed) {
        fail "unexpected default behavior of the type function: `${type($x)}` vs. `${type($x, detailed)}`"
    }
}

type Foo = Variant[Foo, Array[Foo], Integer]

# Basic value tests
display_type(1)
display_type(2.0)
display_type(true)
display_type(falses)
display_type('foo')
display_type(/bar/)
display_type(undef)
display_type(default)
display_type([1, 2, 3])
display_type({ foo => bar, bar => baz })
display_type(Tuple[Variant[String[1, 2], Integer[3, 4]], Regexp])
display_type([1, 2, 3].reverse_each)
display_type({foo => 1, bar => /foo/ }.reverse_each)
display_type(Foo)

# Empty array and hash
display_type([])
display_type({})

# Common type tests
display_type([1, 2, 3])
display_type([1.0, 2.0, 3.0])
display_type([foo, bar, cake])
display_type([1, 2.0, 3])
display_type([Type, foo, 5])
display_type([[1], [2, 3.0], [4]])
display_type([[], {}])
display_type([Class, Class])
display_type([File, File])
display_type([File, Notify])
display_type([Pattern[foo], Pattern[bar], Pattern[foo, baz, cake]])
display_type([Enum[foo], Enum[bar], Enum[foo, baz, cake]])
display_type([Variant[String], Variant[Integer], Variant[String, Regexp, Type]])
display_type([/foo/, /bar/, /baz/])
display_type([Callable[Integer, Callable[Numeric]], Callable[Numeric, Callable[Integer]]])
display_type([Runtime['C++', 'foo'], Runtime['C++', 'bar']])
display_type([1, 2.0])
display_type([1, 2.0, foo, true, /bar/])
display_type([1, 2.0, foo, true, /bar/, [1], { foo => bar }, undef])
display_type([1, 2.0, foo, true, /bar/, [1], { foo => bar }, undef, default])

# Hashes with all string keys
display_type({ foo => bar, bar => 1 })

# Hashes with non-string keys
display_type({ 1 => 2, 3.0 => 4.0 })

# Hashes without string keys that unwrap to a single key and value type
display_type({ true => foo, false => bar })

# Hashes that infer variants for key and value type
display_type({ foo => 1, 1 => /two/, /bar/ => three })
