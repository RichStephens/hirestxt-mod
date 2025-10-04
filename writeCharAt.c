/*  writeCharAt.c

    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#include "hirestxt_private.h"


void writeCharAt(byte x, byte y, byte asciiCode)
{
    (*hiResTextConfig.pfWriteCharAt)(x, y, asciiCode);
}
