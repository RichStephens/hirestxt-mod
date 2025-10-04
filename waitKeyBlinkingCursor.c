/*  waitKeyBlinkingCursor.c

    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#include "hirestxt_private.h"


byte waitKeyBlinkingCursor(void)
{
    if (!hiResTextConfig.inkeyFuncPtr)
        return 3;
    byte key;
    while (!(key = (*hiResTextConfig.inkeyFuncPtr)()))
        animateCursor();
    removeCursor();
    return key;
}
