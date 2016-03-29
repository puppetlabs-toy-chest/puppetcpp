type Foo = Invalid
type Invalid = Variant[Foo]
notice 0 =~ Invalid
