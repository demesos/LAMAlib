.include "LAMAlib.inc"

        lowercase_mode
        print_wrapped "The turm calculation is an iterative process where a starting number is repeatedly multiplied by a series of integers, then divided back down, creating a tower-like structure of computations. This is the turm calculation for the number 5 going up to a height of 7:\n\n"

        let v=5
        println (v)
        for A,2,to,7
            store A
            ldx #0
            stax mult
            let v*=mult
            println (v)
            restore A
        next
        for A,2,to,7
            store A
            ldx #0
            stax div
            let v/=div
            println (v)
            restore A
        next
        rts
