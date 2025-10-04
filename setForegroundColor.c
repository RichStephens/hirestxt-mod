/*  setForegroundColor.c

    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#include "hirestxt_private.h"


void setForegroundColor(byte color)
{
    hiResTextConfig.fgColorMask = fillNybbles(color);
}
