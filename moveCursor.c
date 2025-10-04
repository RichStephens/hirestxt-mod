/*  moveCursor.c

    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#include "hirestxt_private.h"


void moveCursor(byte x, byte y)
{
    if (x >= hiResTextConfig.hiResWidth)
        return;
    if (y >= HIRESHEIGHT)
        return;

    hiResTextConfig.textPosX = x;
    hiResTextConfig.textPosY = y;
}
