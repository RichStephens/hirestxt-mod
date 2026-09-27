/*  scrollTextScreenDown.c

    This file is in the public domain.
*/

#include "hirestxt_private.h"


void scrollTextScreenDown(void)
{
    word bytesPerPixelRow = hiResTextConfig.numPixelsPerRow * hiResTextConfig.numBitsPerPixel / 8;
    byte *buffer = hiResTextConfig.textScreenBuffer;
    byte *end = buffer + bytesPerPixelRow * hiResTextConfig.numPixelsRowPerScreen;
    const byte *readPtr = end - bytesPerPixelRow * PIXEL_ROWS_PER_TEXT_ROW;

    byte scrollFillMask = (screenInverted ? hiResTextConfig.fgColorMask
                                          : hiResTextConfig.bgColorMask);
    word wordToClearWith = (hiResTextConfig.numBitsPerPixel == 4
                                ? (scrollFillMask | ((word) scrollFillMask << 8))
                                : (screenInverted ? 0x0000u : 0xFFFFu));
    asm
    {
        ldx     :end

        pshs    y
        ldy     :readPtr            // cannot refer to global vars after this

@scrollTextScreenDown_loop1:
        ldd     ,--y
        std     ,--x
        ldd     ,--y
        std     ,--x
        ldd     ,--y
        std     ,--x
        ldd     ,--y
        std     ,--x
        ldd     ,--y
        std     ,--x
        ldd     ,--y
        std     ,--x
        ldd     ,--y
        std     ,--x
        ldd     ,--y
        std     ,--x
;
        cmpy    :buffer
        bhi     @scrollTextScreenDown_loop1

        puls    y                   // restore global data segment ptr

        ldd     :wordToClearWith
@scrollTextScreenDown_loop2:
        std     ,--x
        std     ,--x
        std     ,--x
        std     ,--x
        std     ,--x
        std     ,--x
        std     ,--x
        std     ,--x
        std     ,--x
        std     ,--x
        std     ,--x
        std     ,--x
        std     ,--x
        std     ,--x
        std     ,--x
        std     ,--x
        cmpx    :buffer
        bhi     @scrollTextScreenDown_loop2
    }
}
