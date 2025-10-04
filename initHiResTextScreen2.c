/*  initHiResTextScreen2.c

    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#include "hirestxt_private.h"


static word useWordOrDefault(word value, word defaultValue)
{
    return value != 0 ? value : defaultValue;
}


void initHiResTextScreen2(const struct HiResTextScreenInit2 *init)
{
    byte textColWidthInBits = (init->init.numColumns == 42 ? 6 : 5);
    hiResTextConfig.numPixelsPerRow = useWordOrDefault(init->numPixelsPerRow, 256);
    hiResTextConfig.hiResWidth = (byte) (hiResTextConfig.numPixelsPerRow / textColWidthInBits);  // required by moveCursor()
    hiResTextConfig.numPixelsRowPerScreen = (byte) useWordOrDefault(init->numPixelsRowPerScreen, 192);
    hiResTextConfig.numBitsPerPixel = (byte) useWordOrDefault(init->numBitsPerPixel, 1);
    hiResTextConfig.fgColorMask = fillNybbles(init->fgColor);
    hiResTextConfig.fgBoldColorMask = fillNybbles(init->fgBoldColor);
    hiResTextConfig.bgColorMask = fillNybbles(init->bgColor);

    hiResTextCommonInit(&init->init);
}
