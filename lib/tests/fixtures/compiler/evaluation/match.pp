notice case foo {
    /f(o)o/: {
        notice "\$0 = $0"
        notice "\$1 = $1"
        correct
    }
    default: {
        fail incorrect
    }
}

notice case bar {
    foo: {
        fail incorrect
    }
    default: {
        correct
    }
}

notice case 5 {
    Integer[5, 10]: {
        correct
    }
}

notice case foo {
    *[bar, foo, baz]: { correct }
    default: { incorrect}
}

notice case [green, [2], undef] {
  [/ee/, [Integer[0,10]], default]: { notice "\$0 = $0"; correct }
  default: { fail incorrect }
}

notice case [green, [2], undef, extra] {
  [/ee/, [Integer[0,10]], default]: { fail incorrect }
  default: { correct }
}

notice case [green, [2], undef] {
  *[/ee/, [Integer[0,10]], default]: { notice "\$0 = $0"; correct }
  default: { fail incorrect }
}

notice case {foo => green, bar => [2], baz => undef, extra => ok} {
    {foo => /ee/, bar => [Integer[0, 10]], baz => default}: { notice "\$0 = $0"; correct }
    default: { fail incorrect }
}

notice case {foo => green, bar => [2], baz => undef} {
    {foo => /ee/, bar => [Integer[0, 10]], baz => default, extra => wrong}: { fail incorrect }
    default: { correct }
}

notice foo ? {
    /f(o)o/ => correct,
    default => incorrect
}

notice bar ? {
    /f(o)o/ => incorrect,
    default => correct
}

notice 5 ? {
    Integer[5, 10] => correct
}

notice foo ? {
    *[bar, foo, baz] => correct,
    default => incorrect
}

notice [green, [2], undef] ? {
  [/ee/, [Integer[0,10]], default] => correct,
  default => incorrect
}

notice [green, [2], undef, extra] ? {
  [/ee/, [Integer[0,10]], default] => incorrect,
  default => correct
}

notice [green, [2], undef] ? {
  *[/ee/, [Integer[0,10]], default] => correct,
  default => incorrect
}

notice {foo => green, bar => [2], baz => undef, extra => ok} ? {
    {foo => /ee/, bar => [Integer[0, 10]], baz => default} => correct,
    default => incorrect
}

notice {foo => green, bar => [2], baz => undef} ? {
    {foo => /ee/, bar => [Integer[0, 10]], baz => default, extra => wrong} => incorrect,
    default => correct
}
