/*  initHiResTextScreen2.c

    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#include "hirestxt_private.h"


byte fillNybbles(byte nybble)
{
    return nybble | (nybble << 4);
}
