/*  animateCursor.c

    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#include "hirestxt_private.h"


// Uses hiResTextConfig.
//
void
animateCursor(void)
{
    // Look at bits 4 and 5 (by default) of the timer word.
    // Values 0..2 of this tayste mean displayed cursor, 3 means no cursor.
    // Resetting the timer word at 0 will display the cursor.
    //
    BOOL currentCursorState = ((byte) *hiResTextConfig.hiResTextTimerPtr & hiResTextConfig.cursorAnimationLowTimerByteMask) != hiResTextConfig.cursorAnimationLowTimerByteMask;
    if (currentCursorState != hiResTextConfig.hiResTextCursorPresent)
    {
        invertPixelsAtCursor();
        hiResTextConfig.hiResTextCursorPresent = currentCursorState;
    }
}
