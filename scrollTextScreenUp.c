/*  scrollTextScreenUp.c

    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#include "hirestxt_private.h"


void scrollTextScreenUp(void)
{
    word bytesPerPixelRow = hiResTextConfig.numPixelsPerRow * hiResTextConfig.numBitsPerPixel / 8;
    word *end = (word *) (hiResTextConfig.textScreenBuffer
                            + bytesPerPixelRow
                                * (hiResTextConfig.numPixelsRowPerScreen - PIXEL_ROWS_PER_TEXT_ROW));
                // start of last row in PMODE 4: 32 bytes * (192 rows - 8)
    byte *buffer = hiResTextConfig.textScreenBuffer;
    const byte *readPtr = buffer + bytesPerPixelRow * PIXEL_ROWS_PER_TEXT_ROW;

    word wordToClearWith = (hiResTextConfig.numBitsPerPixel == 4
                                ? (hiResTextConfig.bgColorMask | ((word) hiResTextConfig.bgColorMask << 8))
                                : 0xFFFFu);
    asm
    {
        ldx     :buffer

        pshs    y
        ldy     :readPtr            // cannot refer to global vars after this

@scrollTextScreen_loop1:
        ldd     ,y++
        std     ,x++
        ldd     ,y++
        std     ,x++
        ldd     ,y++
        std     ,x++
        ldd     ,y++
        std     ,x++
        ldd     ,y++
        std     ,x++
        ldd     ,y++
        std     ,x++
        ldd     ,y++
        std     ,x++
        ldd     ,y++
        std     ,x++
;
        cmpx    :end                // uses U to refer to 'end'
        blo     @scrollTextScreen_loop1

        sty     :end                // points to end of screen buffer

        puls    y                   // restore global data segment ptr

        ldd     :wordToClearWith
@scrollTextScreen_loop2:
        std     ,x++
        std     ,x++
        std     ,x++
        std     ,x++
        std     ,x++
        std     ,x++
        std     ,x++
        std     ,x++
        std     ,x++
        std     ,x++
        std     ,x++
        std     ,x++
        std     ,x++
        std     ,x++
        std     ,x++
        std     ,x++
        cmpx    :end                // compare to end of screen buffer
        blo     @scrollTextScreen_loop2
    }
}
