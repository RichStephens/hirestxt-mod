/*  writeString.c

    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#include "hirestxt_private.h"


void writeString(const char *str)
{
    for (;;)
    {
        byte ch = *str;
        if (!ch)
            break;
        ++str;

        writeChar(ch);
    }
}
