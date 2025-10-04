/*  initHiResTextScreen.c

    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#include "hirestxt_private.h"


void initHiResTextScreen(const struct HiResTextScreenInit *init)
{
    hiResTextConfig.hiResWidth = (init->numColumns == 42 ? 42 : 51);  // required by moveCursor()
    hiResTextConfig.numPixelsPerRow = 256;
    hiResTextConfig.numPixelsRowPerScreen = 192;
    hiResTextConfig.numBitsPerPixel = 1;

    hiResTextCommonInit(init);
}
