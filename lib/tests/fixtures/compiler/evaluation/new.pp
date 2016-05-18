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
