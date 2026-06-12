/*  setHiResTextBuffer.c - 51x24 black-on-green PMODE 4 text screen.

    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.

*/

#include "hirestxt_private.h"


void setHiResTextBuffer(byte *newTextScreenBuffer)
{
    hiResTextConfig.textScreenBuffer = newTextScreenBuffer;
}
