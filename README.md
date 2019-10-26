# LAMAlib
## Lightweight Assembler MAcro library for cc65

Contains macros for 16 bit operations and easier screen output, for example:

```
clc
ldax #$1234  ; load a 16 bit value into registers A/X
adcax #$2345 ; add another 16 bit value
stax $C000    ; store result

print "The result is ",($C000) ;print the result to the screen
```

Please see the full documentation in `lamalibdoc.html`

