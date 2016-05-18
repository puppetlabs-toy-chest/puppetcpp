# Integer tests
notice Integer(1)
notice Integer.new(2.6)
notice Integer(true)
notice new(Integer, false)
notice Integer.new('2')
notice Integer.new(' 0b11 ')
notice new(Integer, '0111')
notice Integer.new(' -0x1111 ')
notice Integer.new('11', 2)
notice Integer('11', 8)
notice Integer.new('11', 10)
notice Integer('11', 16)
notice Integer[100, 1000]('123')
type IntegerAlias = Integer
notice IntegerAlias.new('0xff')

# Optional tests
notice Optional[Integer](1)
notice Optional[Integer[10, default]](55)
type OptionalAlias = Optional[Integer]
notice OptionalAlias.new('0xff')

# NotUndef tests
notice NotUndef[Integer]('1')
notice NotUndef[Integer[default, 10]]('5', 8)
type NotUndefAlias = NotUndef[Integer]
notice NotUndefAlias.new('0xff')

# Boolean tests
notice Boolean(1)
notice Boolean.new(100)
notice new(Boolean, 0)
notice Boolean(1.0)
notice Boolean.new(100.0)
notice new(Boolean, 0.0)
notice Boolean(false)
notice Boolean.new(true)
notice Boolean('TrUe')
notice Boolean.new('yEs')
notice new(Boolean, 'y')
notice Boolean('False')
notice Boolean.new('NO')
notice new(Boolean, 'N')
type BooleanAlias = Boolean
notice BooleanAlias('y')

# Float tests
notice Float(1)
notice Float.new(2.6)
notice Float(true)
notice new(Float, false)
notice Float.new('2')
notice Float.new(' 0b11 ')
notice new(Float, '0111.2')
notice Float.new(' - 0x1111 ')
notice Float.new('11')
notice Float('+0X11')
notice Float[100, 1000](123.6)
notice Float("31.41e-1")
notice Float("314.1E-2")
type FloatAlias = Float
notice FloatAlias('123.456')

# Numeric tests
notice Numeric(1)
notice Numeric.new(2.6)
notice Numeric(true)
notice new(Numeric, false)
notice Numeric.new('2')
notice Numeric.new(' 0b11 ')
notice new(Numeric, '0111.2')
notice Numeric.new(' - 0x1111 ')
notice Numeric.new('11')
notice Numeric('+ 0X11')
notice Numeric("31.41e-1")
notice Numeric("314.1E-2")
type NumericAlias = Numeric
notice NumericAlias.new('0111')
notice NumericAlias('123.456')

# Array tests
notice Array[Integer](10)
notice Array[String].new(foo)
notice new(Array[Integer], Integer[0, 5])
notice Array[String](Enum[foo, bar, baz])
notice Array[Integer]([1, 2, 3])
notice Array[Tuple[String, String]].new({foo => bar, bar => baz})
notice Array[Integer]([1, 2, 3].reverse_each)
notice Array(undef, true)
notice Array(10, true)
notice Array.new(foo, true)
notice new(Array[Any], Integer[0, 5], true)
notice Array[Any](Enum[foo, bar, baz], true)
notice Array([1, 2, 3], true)
notice Array.new({foo => bar, bar => baz}, true)
notice Array([1, 2, 3].reverse_each, true)
type ArrayAlias = Array
notice ArrayAlias(5)

# Tuple tests
notice Tuple[Integer, 10, 10](10)
notice Tuple[String, 3, 3].new(foo)
notice new(Tuple[Integer, 6, 6], Integer[0, 5])
notice Tuple[String, 3, 3](Enum[foo, bar, baz])
notice Tuple[Integer, 3, 3]([1, 2, 3])
notice Tuple[Tuple[String, String], 2, 2].new({foo => bar, bar => baz})
notice Tuple[Integer, 3, 3]([1, 2, 3].reverse_each)
notice Tuple[Integer](10, true)
notice Tuple[String].new(foo, true)
notice new(Tuple[Type[Integer[0, 5]]], Integer[0, 5], true)
notice Tuple[Type[Enum]](Enum[foo, bar, baz], true)
notice Tuple[Integer, 3, 3]([1, 2, 3], true)
notice Tuple[Hash].new({foo => bar, bar => baz}, true)
notice Tuple[Integer, 3, 3]([1, 2, 3].reverse_each, true)
type TupleAlias = Tuple[Integer, 5, 5]
notice TupleAlias.new(5)
