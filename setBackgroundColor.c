/*  setBackgroundColor.c

    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#include "hirestxt_private.h"


void setBackgroundColor(byte color)
{
    hiResTextConfig.bgColorMask = fillNybbles(color);
}
