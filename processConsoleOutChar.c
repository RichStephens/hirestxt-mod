/*  processConsoleOutChar.c

    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#include "hirestxt_private.h"


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
        case 'B':  // cursor up
            if (hiResTextConfig.textPosY < HIRESHEIGHT - 1)
                ++hiResTextConfig.textPosY;
            hiResTextConfig.vt52State = VT52_TEXT;  // end of sequence
            return;
        case 'C':  // cursor right
            if (hiResTextConfig.textPosX < hiResTextConfig.hiResWidth - 1)
                ++hiResTextConfig.textPosX;
            hiResTextConfig.vt52State = VT52_TEXT;  // end of sequence
            return;
        case 'S':  // mysterious sequence: ESC S O (used by vi...)
            hiResTextConfig.vt52State = VT52_IGNORE_NEXT;
            hiResTextConfig.vt52NumBytesToIgnore = 1;
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
        // G (select ASCII char set) is not supported.
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
        moveCursor(ch - 32, hiResTextConfig.vt52Line);

    hiResTextConfig.vt52State = VT52_TEXT;
#endif  /* HIRESTEXT_NO_VT52 */
}
