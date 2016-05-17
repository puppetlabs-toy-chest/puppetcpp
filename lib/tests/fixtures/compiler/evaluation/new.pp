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
