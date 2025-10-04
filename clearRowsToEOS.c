/*  clearRowsToEOS.c

    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#include "hirestxt_private.h"


void clearRowsToEOS(byte byteToClearWith, byte textRow)
{
    if (textRow >= HIRESHEIGHT)
        return;

    byte *buffer = hiResTextConfig.textScreenBuffer;  // CMOC 0.1.77 does not support struct fields in inline asm, so use local copy
    byte *clearedBufferStart = buffer
                                + textRow
                                  * (hiResTextConfig.numPixelsPerRow
                                        * hiResTextConfig.numBitsPerPixel
                                        / 8)
                                  * PIXEL_ROWS_PER_TEXT_ROW;
    byte *bufferEnd = buffer + hiResTextConfig.numPixelsPerRow
                                    / 8  // done now to avoid overflow when doing 320 * 192 * 4
                                    * hiResTextConfig.numPixelsRowPerScreen
                                    * hiResTextConfig.numBitsPerPixel;
    asm
    {
        ldx     :clearedBufferStart
        lda     :byteToClearWith
        tfr     a,b
@loop   ; Clear 32 bytes:
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
        cmpx    :bufferEnd
        blo     @loop
    }
}
