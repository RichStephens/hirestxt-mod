/*  getCursorRow.c

    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#include "hirestxt_private.h"


byte getCursorRow(void)
{
    return hiResTextConfig.textPosY;
}
