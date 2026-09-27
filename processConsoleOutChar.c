/*  processConsoleOutChar.c

    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#include "hirestxt_private.h"


#ifndef HIRESTEXT_NO_VT52

// VT52 graphics set for '^'..'~', drawn from the ISO-8859-1 glyphs where one
// exists and plain ASCII otherwise. 0 is the full block.
//
static const byte vt52GraphicsChars[] =
{
    ' ', ' ', ' ', 0,                       // ^ _ ` a: blank, blank, reserved, full block
    185, 179, '5', '7',                     // b c d e: 1/ 3/ 5/ 7/
    176, 177, '>', 183, 247, 'v',           // f..k: degree, +-, right arrow, ellipsis, divide, down arrow
    175, '-', '-', '-', '-', '-', '-', '_', // l..s: scan lines 1 (top) to 8 (bottom)
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',  // t..}: subscripts
    182,                                    // ~: pilcrow
};


static void writeVT52GraphicsChar(byte ch)
{
    ch = vt52GraphicsChars[ch - '^'];
    if (ch)
    {
        writeChar(ch);
        return;
    }
    inverseVideoMode = !inverseVideoMode;
    writeChar(' ');
    inverseVideoMode = !inverseVideoMode;
}

#endif  /* HIRESTEXT_NO_VT52 */


// Calls writeChar() and moveCursor() to execute the VT52 sequence
// provided by the calls to this function.
//
// Source: http://bitsavers.trailing-edge.com/pdf/dec/terminal/gigi/EK-0GIGI-RC-001_GIGI_Programming_Reference_Card_Sep80.pdf
//
void processConsoleOutChar(byte ch)
{
#ifndef HIRESTEXT_NO_VT52
    if (hiResTextConfig.vt52State == VT52_IGNORE_NEXT)
    {
        --hiResTextConfig.vt52NumBytesToIgnore;
        if (!hiResTextConfig.vt52NumBytesToIgnore)
            hiResTextConfig.vt52State = VT52_TEXT;
        return;
    }
#endif

    removeCursor();

#ifndef HIRESTEXT_NO_VT52
    if (hiResTextConfig.vt52State == VT52_TEXT)
    {
        if (ch == 27)  // if Escape char
            hiResTextConfig.vt52State = VT52_GOT_ESC;
        else if (hiResTextConfig.vt52Graphics && ch >= '^' && ch <= '~')
            writeVT52GraphicsChar(ch);
        else
#endif
            writeChar(ch);
        return;
#ifndef HIRESTEXT_NO_VT52
    }

    if (hiResTextConfig.vt52State == VT52_GOT_ESC)
    {
        // Most common commands should be tested first.
        //
        switch (ch)
        {
        case 'Y':  // direct cursor address (expecting 2 more bytes)
            hiResTextConfig.vt52State = VT52_WANT_LINE;
            return;
        case 'K':  // erase to end of line
            clrtoeol();
            hiResTextConfig.vt52State = VT52_TEXT;
            return;
        case 'D':  // cursor left
            if (hiResTextConfig.textPosX)
                --hiResTextConfig.textPosX;
            hiResTextConfig.vt52State = VT52_TEXT;  // end of sequence
            return;
        case 'H':  // cursor home
            home();
            hiResTextConfig.vt52State = VT52_TEXT;
            return;
        case 'J':  // erase to end of screen
            clrtobot();
            hiResTextConfig.vt52State = VT52_TEXT;
            return;
        case 'E':  // clear screen
            home();
            clrtobot();
            hiResTextConfig.vt52State = VT52_TEXT;
            return;
        case 'A':  // cursor up
            if (hiResTextConfig.textPosY)
                --hiResTextConfig.textPosY;
            hiResTextConfig.vt52State = VT52_TEXT;  // end of sequence
            return;
        case 'B':  // cursor down
            if (hiResTextConfig.textPosY < HIRESHEIGHT - 1)
                ++hiResTextConfig.textPosY;
            hiResTextConfig.vt52State = VT52_TEXT;  // end of sequence
            return;
        case 'C':  // cursor right
            if (hiResTextConfig.textPosX < hiResTextConfig.hiResWidth - 1)
                ++hiResTextConfig.textPosX;
            hiResTextConfig.vt52State = VT52_TEXT;  // end of sequence
            return;
        case 'I':  // reverse line feed
            if (hiResTextConfig.textPosY)
                --hiResTextConfig.textPosY;
            else
                scrollTextScreenDown();
            hiResTextConfig.vt52State = VT52_TEXT;  // end of sequence
            return;
        case 'S':  // mysterious sequence: ESC S O (used by vi...)
            hiResTextConfig.vt52State = VT52_IGNORE_NEXT;
            hiResTextConfig.vt52NumBytesToIgnore = 1;
            return;
        case 'F':  // enter graphics mode
            if (hiResTextConfig.vt52SetOriginalFont && !hiResTextConfig.vt52Graphics)
            {
                hiResTextConfig.vt52FontWasOriginal = (*hiResTextConfig.vt52SetOriginalFont)(TRUE);
                hiResTextConfig.vt52Graphics = TRUE;
            }
            hiResTextConfig.vt52State = VT52_TEXT;
            return;
        case 'G':  // exit graphics mode
            if (hiResTextConfig.vt52Graphics)
            {
                (*hiResTextConfig.vt52SetOriginalFont)(hiResTextConfig.vt52FontWasOriginal);
                hiResTextConfig.vt52Graphics = FALSE;
            }
            hiResTextConfig.vt52State = VT52_TEXT;
            return;
        case 'p':  // switch on inverse video text
            setInverseVideoMode(TRUE);
            hiResTextConfig.vt52State = VT52_TEXT;  // end of sequence
            return;
        case 'q':  // switch off inverse video text
            setInverseVideoMode(FALSE);
            hiResTextConfig.vt52State = VT52_TEXT;  // end of sequence
            return;
        }

        // Any other sequence is not supported. Return to text mode.
        //
        //writeString("\n\n*** INVALID VT52 COMMAND\n");
        hiResTextConfig.vt52State = VT52_TEXT;
        return;
    }

    if (hiResTextConfig.vt52State == VT52_WANT_LINE)
    {
        hiResTextConfig.vt52Line = ch - 32;
        hiResTextConfig.vt52State = VT52_WANT_COL;
        return;
    }

    if (hiResTextConfig.vt52State == VT52_WANT_COL)
    {
        // As on a VT52: an out-of-range line keeps the current one, and an
        // out-of-range column goes to the last one.
        byte col = ch - 32;
        if (hiResTextConfig.vt52Line < HIRESHEIGHT)
            hiResTextConfig.textPosY = hiResTextConfig.vt52Line;
        hiResTextConfig.textPosX = (col < hiResTextConfig.hiResWidth
                                    ? col : hiResTextConfig.hiResWidth - 1);
    }

    hiResTextConfig.vt52State = VT52_TEXT;
#endif  /* HIRESTEXT_NO_VT52 */
}
