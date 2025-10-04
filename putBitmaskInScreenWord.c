/*  putBitmaskInScreenWord.c

    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#include "hirestxt_private.h"


void putBitmaskInScreenWord(byte asciiCode, word *screenWord,
                            const byte *charBitmask, word charWordShifts,
                            word mask)
{
    word charWord, invMask;
    byte row;
    asm
    {
; Cache an inverted mask if inverting colors is needed.
        ldd     :mask
        coma
        comb
        std     :invMask
; Initialize for() loop counter.
        clrb                ; B caches :row
        stb     :row        ; stb is 2 cycles less than clr

;;;; Start of loop ;;;;
;
@writeCharAt_for
;
; if (asciiCode)
        tst     :asciiCode
        beq     @writeCharAt_nul
;
; charWord = ((word) charBitmask[row]) << 8;
        ldx     :charBitmask
        lda     b,x             ; B caches :row
        clrb                    ; D is now charWord
;
; if (boldMode)
        tst     :boldMode
        beq     @noBold
;
; charWord = ~(~charWord | (~charWord >> 1));
        coma
        comb
        pshs    b,a             ; save ~charWord
        lsra
        rorb                    ; >> 1
        ora     ,s+
        orb     ,s+
        coma
        comb
@noBold
; D is charWord.
; charWord >>= charWordShifts;
        ldx     :charWordShifts       ; counter
        beq     @writeCharAt_shifts_done
@writeCharAt_shift_loop               ; shift D right X times
        lsra
        rorb
        leax    -1,x
        bne     @writeCharAt_shift_loop
@writeCharAt_shifts_done
        tst     :inverseVideoMode       ; global boolean
        beq     @noInvert
        eora    :invMask
        eorb    :invMask[1]
@noInvert
        std     :charWord
;
; word d = *screenWord;  ; read screen bits (big endian word read)
        ldd     [:screenWord]
;
; d &= mask;
; d |= charWord;
        anda    :mask
        andb    :mask[1]         ; adds one to offset applied on U
        ora     :charWord
        orb     :charWord[1]
        bra     @writeCharAt_after_if
;
; else: d ^= invMask;
@writeCharAt_nul
        ldd     [:screenWord]
        eora    :invMask
        eorb    :invMask[1]
;
@writeCharAt_after_if
;
; *screenWord = d;
; screenWord += 16;
        ldx     :screenWord
        std     ,x
        leax    BYTES_PER_PIXEL_ROW,x   ; N.B.: X still contains :screenWord, so increment it
        stx     :screenWord
;
        ldb     :row
        incb
        stb     :row
        cmpb    #PIXEL_ROWS_PER_TEXT_ROW
        blo     @writeCharAt_for
    }
}
