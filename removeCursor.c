/*  removeCursor.c

    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#include "hirestxt_private.h"


void removeCursor(void)
{
    if (hiResTextConfig.hiResTextCursorPresent)
    {
        invertPixelsAtCursor();
        hiResTextConfig.hiResTextCursorPresent = FALSE;
    }
}
