/*  setForegroundBoldColor.c

    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#include "hirestxt_private.h"


void setForegroundBoldColor(byte color)
{
    hiResTextConfig.fgBoldColorMask = fillNybbles(color);
}
