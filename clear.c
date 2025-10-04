/*  clear.c

    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#include "hirestxt_private.h"


void clear(void)
{
    byte byteToClearWith = (hiResTextConfig.numBitsPerPixel == 4
                                ? hiResTextConfig.bgColorMask
                                : 0xFFu);
    clearRowsToEOS(byteToClearWith, 0);
}
