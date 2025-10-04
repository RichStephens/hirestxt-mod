/*  writeChar.c

    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#include "hirestxt_private.h"


static void goToNextRowIfPastEndOfRow(void)
{
    if (hiResTextConfig.textPosX < hiResTextConfig.hiResWidth)  // if not past end of current row
        return;

    hiResTextConfig.textPosX = 0;
    ++hiResTextConfig.textPosY;
    if (hiResTextConfig.textPosY < HIRESHEIGHT)  // if not past bottom of screen
        return;

    scrollTextScreenUp();
    hiResTextConfig.textPosY = HIRESHEIGHT - 1;
}


void writeChar(byte ch)
{
    if (ch == '\a')
    {
        if (hiResTextConfig.bellFuncPtr)
            (*hiResTextConfig.bellFuncPtr)();
    }
    else if (ch == '\b')
    {
        // Non-destructive backspace. At (0, 0), does not scroll screen up.
        //
        if (hiResTextConfig.textPosX > 0)
            --hiResTextConfig.textPosX;
        else if (hiResTextConfig.textPosY > 0)
        {
            hiResTextConfig.textPosX = hiResTextConfig.hiResWidth - 1;
            --hiResTextConfig.textPosY;
        }
    }
    else if (ch == '\t')
    {
        // If past end of current row, start by putting cursor at start of next row.
        // Scroll screen up one row if needed.
        // Then tab processing can start at column 0.
        //
        goToNextRowIfPastEndOfRow();

        byte newX = (hiResTextConfig.textPosX | 7) + 1;
        if (newX > hiResTextConfig.hiResWidth)  // tab at 48..50 leads to 56: clamp; similarly in 42x24 mode
            newX = hiResTextConfig.hiResWidth;
        for (byte numSpaces = newX - hiResTextConfig.textPosX; numSpaces; --numSpaces)
            writeChar(' ');
    }
    else if (ch == '\n')
    {
        // Note that textPosX may be hiResWidth at this point.
        //
        hiResTextConfig.textPosX = 0;
        ++hiResTextConfig.textPosY;
        if (hiResTextConfig.textPosY >= HIRESHEIGHT)
        {
            scrollTextScreenUp();
            hiResTextConfig.textPosY = HIRESHEIGHT - 1;
        }
    }
    else if (ch == '\f')
    {
        clrscr();
    }
    else if (ch == '\r')
    {
        hiResTextConfig.textPosX = 0;
    }
    else if (ch >= ' ' && ch != 127)  // printable character:
    {
        // If past end of current row, put cursor at start of next row.
        // Scroll screen up one row if needed.
        //
        goToNextRowIfPastEndOfRow();

        // Write char at cursor, advance cursor, go to next line if needed.
        //
        (*hiResTextConfig.pfWriteCharAt)(hiResTextConfig.textPosX, hiResTextConfig.textPosY, ch);
        ++hiResTextConfig.textPosX;

        // textPosX may now be hiResWidth, which is not a writable column,
        // but we tolerate it for easier management of a newline that comes
        // after writing 51 (or 42) printable chars on a row.
    }
}
