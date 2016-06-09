# This will test calculating column and length with Unicode characters
# Note: there should be 16 "squigglies" in the "error pointer"
#       5 for the string literal (5 graphemes)
#       1 for the closing single quote
#       1 for the dot
#       5 for the "split" function name
#       1 for the open paren
#       2 for the empty string literal
#       1 for the closing paren
# Please note: this may not display correctly even with a mono-spaced font as
# handling Tamil is quite difficult for fonts
notice 'ஸ்றீனிவாஸ'.split('').split('')
