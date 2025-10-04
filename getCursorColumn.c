/*  getCursorColumn.c

    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#include "hirestxt_private.h"


byte getCursorColumn(void)
{
    return hiResTextConfig.textPosX;
}
