; checks the keyboard for keypresses of W, A ,S, D and Space
; output is a byte in A in the same format as a joystick value 

.include "../LAMAlib-macros16.inc"

.export _readWASDspace_sr := readWASDspace

result=sm+1

readWASDspace:
  lda $DC01
  and #$0f		;test for joystick 1 activity
  cmp #$0f
  beq cont		;joystick 1 is disturbing, return no key
    lda #$1f
    rts
cont:

  poke $DC00, $FB
  lda $DC01
  and #$04		;test for key "d"
  asl			;shift it to 8
  sta result
  poke $DC00, $FD
  lda $DC01
  tax
  and #$20		;test for key "s"
  lsr
  lsr
  lsr
  lsr			;shift it to 2
  ora result
  sta result 
  txa			;restore value
  and #$02		;test for key "w"
  lsr			;shift it to 1
  ora result
  sta result
  txa			;restore value
  and #$04		;test for key "a"
  ora result
  sta result
  poke $DC00, $7F
  lda $DC01
  and #$10		;test for space
sm:  ora #00
  rts 
