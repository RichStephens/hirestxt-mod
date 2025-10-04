/*  clrtoeol.c

    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#include "hirestxt_private.h"


void clrtoeol(void)
{
    for (byte x = hiResTextConfig.textPosX; x != hiResTextConfig.hiResWidth; ++x)
        (*hiResTextConfig.pfWriteCharAt)(x, hiResTextConfig.textPosY, ' ');
}
