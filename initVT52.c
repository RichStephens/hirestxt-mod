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
}


#endif  /* HIRESTEXT_NO_VT52 */
