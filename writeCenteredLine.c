/*  writeCenteredLine.c

    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#include "hirestxt_private.h"


void writeCenteredLine(byte row, const char *line)
{
    moveCursor((byte) ((hiResTextConfig.hiResWidth - strlen(line)) / 2), row);
    writeString(line);
}
