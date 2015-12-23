@(EMPTY)
EMPTY

@(TEST)
this
is
a
heredoc
TEST

@(FIRST) 'hello' @(SECOND) 'world' @(THIRD)
first
FIRST
second
SECOND
third
THIRD

@(JSON:json)
{
  "hello": "world"
}
JSON

@(FORMAT)
first: \\\t\s\r\n\u263A\$\
second!
FORMAT

@(FORMAT/t)
first: \\\t\s\r\n\u263A\$\
second!
FORMAT

@( FORMAT/ts )
first: \\\t\s\r\n\u263A\$\
second!
FORMAT

@(FORMAT/tsr)
first: \\\t\s\r\n\u263A\$\
second!
FORMAT

@(FORMAT/tsrn )
first: \\\t\s\r\n\u263A\$\
second!
FORMAT

@(FORMAT/ tsrnu )
first: \\\t\s\r\n\u263A\$\
second!
FORMAT

@(   FORMAT   / tsrnu$ )
first: \\\t\s\r\n\u263A\$\
second!
FORMAT

@( FORMAT  /tsrnu$L)
first: \\\t\s\r\n\u263A\$\
second!
FORMAT

@( FORMAT/    )
first: \\\t\s\r\n\u263A\$\
second!
FORMAT

@( "this is interpolated" )
$hello \$world
this is interpolated

@( "this is interpolated" / )
$hello \$world
this is interpolated

@( "this is interpolated" / $ )
$hello \$world
this is interpolated

@( "this is interpolated" / t )
$hello \$world
this is interpolated

@(NOT)
this is NOT the end
NOT

@(TEXT)
this is one line
-TEXT

@(ALIGNED)
    this text
     is
      aligned
    | ALIGNED

@(TRIMALIGNED)
    this text
     is
      aligned
    |- TRIMALIGNED

@("ALLOFTHEABOVE" : json / t$)
    this \$text
     is
      aligned
     |- ALLOFTHEABOVE
