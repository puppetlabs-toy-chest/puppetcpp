# Split by string
notice ''.split(' ')
notice 'foo bar baz      '.split(' ')
notice 'foo / bar / baz'.split(' / ')
notice 'each'.split('')
notice 'ஸ்றீனிவாஸ ராமானுஜன் ஐயங்கார்'.split('ன் ')
notice 'ஸ்றீனிவாஸ ராமானுஜன் ஐயங்கார்'.split('')
notice 'χஸ்それχஸ்は私をχஸ்χஸ்傷つχஸ்'.split('χஸ்')
notice 'χஸ்それχஸ்は私をχஸ்χஸ்傷つχஸ்'.split('ன்')
notice "no\u0303! nõ?!".split('o\u0303')
notice "no\u0303! nõ?!".split('õ')
notice "no\u0303! nõ?!".split('\u0303')
notice "no\u0303! nõ?!".split('o')

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
