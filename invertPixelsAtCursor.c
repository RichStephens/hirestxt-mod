/*  invertPixelsAtCursor.c

    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#include "hirestxt_private.h"


void invertPixelsAtCursor(void)
{
    byte x = hiResTextConfig.textPosX;
    byte y = hiResTextConfig.textPosY;

    if (x >= hiResTextConfig.hiResWidth)  // logical position that gets mapped to start of next line
    {
        x = 0;
        ++y;
        if (y >= HIRESHEIGHT)
            y = HIRESHEIGHT - 1;
    }

    (*hiResTextConfig.pfWriteCharAt)(x, y, 0);
}
