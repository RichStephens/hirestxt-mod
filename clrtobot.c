/*  clrtobot.c

    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#include "hirestxt_private.h"


void clrtobot(void)
{
    byte bgColorMask = hiResTextConfig.numBitsPerPixel == 4
                                    ? hiResTextConfig.bgColorMask
                                    : 0xFF;
    byte dy;
    if (hiResTextConfig.textPosX > 0)
    {
        clrtoeol();
        dy = 1;
    }
    else
        dy = 0;
    clearRowsToEOS(bgColorMask, hiResTextConfig.textPosY + dy);
}
