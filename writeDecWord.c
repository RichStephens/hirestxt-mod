/*  writeDecWord.c

    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#include "hirestxt_private.h"


void writeDecWord(word w)
{
    char buf[11];
    writeString(dwtoa(buf, 0, w));
}
