/*  setTrueBold.c - chooses how bold renders in the 320x192x16 mode:
    a thickened glyph in the current color, or the separate bold color at
    normal weight. See setTrueBold() in hirestxt.h.

    This file is in the public domain.
*/

#include "hirestxt_private.h"


void setTrueBold(BOOL trueBold)
{
    trueBoldMode = trueBold;
}
