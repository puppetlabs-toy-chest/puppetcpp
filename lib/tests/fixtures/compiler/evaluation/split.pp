# Split by string
notice ''.split(' ')
notice 'foo bar baz      '.split(' ')
notice 'foo / bar / baz'.split(' / ')
notice 'each'.split('')

# Split by regex
notice ''.split(/foo/)
notice 'each'.split(//)
notice 'foo bar'.split(/ /)
notice 'foo 123 bar 4567 baz 89'.split(/\s*\d+\s*/)
notice 'foo 123 bar 4567 baz 89'.split(/\s*(\d+)\s*/)

# Split by Regexp type
notice ''.split(Regexp[/foo/])
notice 'each'.split(Regexp[//])
notice 'foo bar'.split(Regexp[/ /])
notice 'foo 123 bar 4567 baz 89'.split(Regexp[/\s*\d+\s*/])
notice 'foo 123 bar 4567 baz 89'.split(Regexp[/\s*(\d+)\s*/])
