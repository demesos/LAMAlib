# LAMAlib
## Lightweight Assembler MAcro library for cc65

Contains macros for 16 bit operations and easier screen output, for example:

```
.include "LAMAlib.inc"

clc
ldax #$1234  ; load a 16 bit value into registers A/X
adcax #$2345 ; add another 16 bit value
stax $C000    ; store result

set_cursor_pos 10,0
print "The result is ",($C000) ;print the result to the screen

poke 198,0
do
 lda 198
loop while eq
rts
```

Please see the full documentation in `lamalibdoc.html`

