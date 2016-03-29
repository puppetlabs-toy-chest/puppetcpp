# Check directly self-referential Variant aliases
type MixedScalar = Variant[Integer, String, MixedScalar]
unless 0 =~ MixedScalar {
    fail incorrect
}

unless foo =~ MixedScalar {
    fail incorrect
}

if // =~ MixedScalar {
    fail incorrect
}

# Check complex aliasing

type IntegerTree = Array[Variant[Integer, IntegerTree]]
type Mix = Variant[Integer, String, MixedTree]
type MixedTree = Array[Variant[Mix, MixedTree]]

if 0 =~ IntegerTree {
    fail incorrect
}

unless [0] =~ IntegerTree {
    fail incorrect
}

unless [0, [1, 2, [3]]] =~ IntegerTree {
    fail incorrect
}

if [0, false] =~ IntegerTree {
    fail incorrect
}

function integer_tree(IntegerTree $x) {
  notice $x
}
integer_tree( [1, 2, [42, 4], [[[ 5 ]]] ] )

function mixed(MixedTree $x) {
  notice $x
}
mixed( [1, 2, [hello, 4], [[[ 5, deep ]]] ] )

# This should error because a regex is not a valid type
mixed( [1, 2, [hello, 4], [[[ //, deep ]]] ] )

type Foo = String
type Bar = String[0, 100]
type Baz = Type[Foo]
type Cake = Foo

if Foo != String {
    fail incorrect
}

unless Cake == Foo {
    fail incorrect
}

unless String == Cake {
    fail incorrect
}
