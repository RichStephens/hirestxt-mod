/*  clearRowsN.c
    By Pierre Sarrazin <http://sarrazip.com/>
    Modified by RichStephens' helper to accept a rows-to-clear argument.
    This file is in the public domain.
*/

#include "hirestxt_private.h"

/*
 * clearRowsN:
 *   Clear 'rowsToClear' text rows starting at 'textRow' using byteToClearWith.
 *   'rowsToClear' is an int (can be > 255) but will be clamped so that
 *   clearing never goes past HIRESHEIGHT (assumed 24 rows, indexed 0..23).
 */

void clearRowsN(byte byteToClearWith, byte textRow, byte rowsToClear)
{
    if (textRow >= HIRESHEIGHT)
        return;

    if (rowsToClear <= 0)
        return;

    /* Clamp rowsToClear so we never pass the last row (HIRESHEIGHT - 1) */
    if (textRow + rowsToClear > HIRESHEIGHT)
        rowsToClear = HIRESHEIGHT - textRow;

    byte *buffer = hiResTextConfig.textScreenBuffer;  // CMOC 0.1.77 does not support struct fields in inline asm, so use local copy

    /* bytes per text row:
       (numPixelsPerRow * numBitsPerPixel / 8) * PIXEL_ROWS_PER_TEXT_ROW
       matches what original code used to compute clearedBufferStart */
    unsigned int bytesPerTextRow =
        (hiResTextConfig.numPixelsPerRow
            * hiResTextConfig.numBitsPerPixel
            / 8)
        * PIXEL_ROWS_PER_TEXT_ROW;

    byte *clearedBufferStart = buffer
                                + textRow * bytesPerTextRow;

    /* original bufferEnd calculation (full screen end) preserved */
    byte *bufferEnd = buffer + hiResTextConfig.numPixelsPerRow
                                    / 8  /* done now to avoid overflow when doing 320 * 192 * 4 */
                                    * hiResTextConfig.numPixelsRowPerScreen
                                    * hiResTextConfig.numBitsPerPixel;

    /* Calculate requested end pointer safely within 16-bit address space */
    unsigned int requestedBytes = (unsigned int) rowsToClear * bytesPerTextRow;
    byte *requestedEnd = clearedBufferStart + requestedBytes;

    if (requestedEnd > bufferEnd)
        requestedEnd = bufferEnd;

    /* If start already beyond or equal to requestedEnd, nothing to do */
    if (clearedBufferStart >= requestedEnd)
        return;

    /* Inline asm loop same as original, but uses requestedEnd as buffer end */
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
        cmpx    :requestedEnd
        blo     @loop
    }
}


