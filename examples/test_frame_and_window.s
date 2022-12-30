.include "LAMAlib.inc"

.import _frame_upper_left
.import _frame_upper_right
.import _frame_lower_left
.import _frame_lower_right
.import _frame_vertical
.import _frame_horizontal
.import _frame_fillchar
.import _frame_color

.import _window_x1,_window_y1,_window_x2,_window_y2

        for X,1,to,4
        store X

        switch X

        case 1
          break
        case 2
          poke _window_x1,30
          poke _window_y1,20
          poke _window_x2,37
          poke _window_y2,22
          break
        case 3
          poke _window_x1,30
          poke _window_y1,20
          poke _window_x2,38
          poke _window_y2,23
          break
        case 4
          poke _window_x1,1
          poke _window_y1,1
          poke _window_x2,38
          poke _window_y2,23
          break
        endswitch


        draw_frame

        enable_chrout2window

        for Y,1,to,15
          for X,1,to,40
            txa
            and #$0f
            ora #$40
            jsr $FFD2
          next
          lda #$0d
          jsr $FFD2
        next

        waitkey

        restore X
        next

        disable_chrout2window

        rts