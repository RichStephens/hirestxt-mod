/*  initVT52.c

    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#include "hirestxt_private.h"


#ifndef HIRESTEXT_NO_VT52


void initVT52(void)
{
    hiResTextConfig.vt52State = VT52_TEXT;
    hiResTextConfig.vt52Line = 0;
    hiResTextConfig.vt52NumBytesToIgnore = 0;
    if (hiResTextConfig.vt52Graphics)
        (*hiResTextConfig.vt52SetOriginalFont)(hiResTextConfig.vt52FontWasOriginal);
    hiResTextConfig.vt52Graphics = FALSE;
}


#endif  /* HIRESTEXT_NO_VT52 */
