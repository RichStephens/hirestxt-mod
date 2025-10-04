/*  writeCharAt_42cols.c

    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#include "hirestxt_private.h"

#include "font5x8.h"


// See writeCharAt_51cols.c for details on this system.
// A frame in 42x24 mode is a 3-byte (24-bit) region.
// 4 characters (6 pixels wide each) fit in a frame.
//
static const byte frameByteAddrTable_42cols[4]  = { 0, 0, 1, 2 };
static const byte frameBitOffsetTable_42cols[4] = { 0, 6, 4, 2 };
static const word frameMaskTable_42cols[4]      = { 0x03ff, 0xfc0f, 0xf03f, 0xc0ff };


void putBitmaskInScreenWord(byte asciiCode, word *screenWord,
                            const byte *charBitmask, word charWordShifts,
                            word mask);


void writeCharAt_42cols(byte x, byte y, byte asciiCode)
{
#if 0 // Original (tested) code in C:
    const byte frameCol = x % 4;
    word *screenWord = (word *) (hiResTextConfig.textScreenBuffer + ((word) y * 256) + x / 4 * 3 + frameByteAddrTable_42cols[frameCol]);
        // 256 = 8 rows per character, times 32 bytes per pixel row.
        // 4 = 4 chars par frame. 3 = 3 bytes per frame.

    const byte charWordShifts = frameBitOffsetTable_42cols[frameCol];
    const word mask = frameMaskTable_42cols[frameCol];

    // In charBitmask, only the high 5 bits of each byte are significant.
    // The others must be 0.
    //
    const byte *charBitmask = font5x8 + (((word) asciiCode - (asciiCode < 128 ? 32 : 64)) << 3);

    for (byte row = 0; row < PIXEL_ROWS_PER_TEXT_ROW; ++row)
    {
        word d = *screenWord;  // read screen bits (big endian word read)
        if (asciiCode)
        {
            word charWord = ((word) charBitmask[row]) << 8;  // load into A, reset B; high nybble of D now init
            if (boldMode)
                charWord = ~(~charWord | (~charWord >> 1));
            charWord >>= charWordShifts;
            d &= mask;
            if (inverseVideoMode)
                charWord ^= ~mask;
            d |= charWord;
        }
        else
        {
            d ^= ~mask;  // invert colors
        }
        *screenWord = d;

        screenWord += 16;  // point to next row (32 bytes down)
    }
#else  // Equivalent code in assembler:
    byte frameCol;
    word *screenWord;
    byte *charBitmask;
    word charWordShifts;
    byte *textScreenBuffer = hiResTextConfig.textScreenBuffer;

    asm
    {
        ldb     :x
        andb    #3                      ; % 4
        stb     :frameCol

; word *screenWord = (word *) (textScreenBuffer + ((word) y * 256) + x / 4 * 3 + frameByteAddrTable_42cols[frameCol]);
        ldb     :frameCol
        leax    :frameByteAddrTable_42cols
        ldb     b,x
        pshs    b

        ldb     :x
        lsrb
        lsrb                            ; x / 4
        lda     #3
        mul

        addb    ,s+
        adca    #0

        adda    :y                      ; add y * 256 to D
        addd    :textScreenBuffer
        std     :screenWord

; byte *charBitmask = font5x8 + (((word) asciiCode - (asciiCode < 128 ? 32 : 64)) << 3);
        ldb     :asciiCode
        bpl     @writeCharAt_sub32      ; if 0..127
        subb    #64                     ; assuming B in 160..255
        bra     @writeCharAt_sub_done
@writeCharAt_sub32
        subb    #32
@writeCharAt_sub_done
        clra                            ; D = result of subtraction; shift this 3 bits left
        lslb
        rola
        lslb
        rola
        lslb
        rola
        leax    :font5x8
        leax    d,x
        stx     :charBitmask

; word charWordShifts = frameBitOffsetTable_42cols[frameCol];
        ldb     :frameCol
        leax    :frameBitOffsetTable_42cols
        ldb     b,x
        clra
        std     :charWordShifts

; word mask = frameMaskTable_42cols[frameCol];
        ldb     :frameCol
        lslb                            ; index in array of words
        leax    :frameMaskTable_42cols
        ldd     b,x

; Call putBitmaskInScreenWord(byte asciiCode, word *screenWord, const byte *charBitmask, word charWordShifts, word mask)
;
        pshs    b,a
        ldx     :charWordShifts
        ldd     :charBitmask
        pshs    x,b,a
        ldx     :screenWord
        ldb     :asciiCode
        pshs    x,b,a                   ; A is garbage: does not matter
        lbsr    putBitmaskInScreenWord
        leas    10,s
    }
#endif
}
