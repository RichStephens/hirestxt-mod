/*  showOS9PMode4Screen.c

    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#include "hirestxt_private.h"


// Returns the address of the 256x192 graphics screen.
// Does not clear the screen.
//
byte *
showOS9PMode4Screen(void)
{
    // Source: OS-9 Operating System User's Guide:
    // For Use with OS-9 Level Oneand OS-9 Level Two
    //
    // The terminal from which this is run must already be in a VDG screen mode.
    //
    putstr("\x0F\x00\x01\x13", 4);  // 256x192x2, white on black (\x01); Clear Screen (\x13)
    static byte *screenBuffer = 0;
    if (screenBuffer == 0)
    {
        // This must only be done once, otherwise the process ends with "Error #207 - Memory Full".
        byte errorCode;
        asm
        {
            pshs    y           ; protect global variable segment pointer
            lda     #1
            ldb     #$12        ; SS.DSTAT (Get Display Status)
            os9     $8D         ; I$GSTT
            puls    y
            bcs     @error
            clrb
@error
            stb     :errorCode
            stx     :screenBuffer
        }
        if (errorCode != 0)
            return NULL;
    }

    return screenBuffer;
}
