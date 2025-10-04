/*  init.c

    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#include "hirestxt_private.h"


HiResTextConfig hiResTextConfig;

BOOL inverseVideoMode = FALSE;
BOOL boldMode = FALSE;


#if defined(_COCO_BASIC_) || defined(DRAGON)

static void hiResTextBasicShortSound(void)
{
    sound(1, 1);
}

#endif


// Called by PUTCHR, which is called by printf() et al.
// The character to print is in A.
// It is sent to writeChar() (the 51x24 screen) instead of
// the regular CoCo screen.
//
// MUST preserve B, X, U and Y.
//
void asm hiResTextConsoleOutHook(void)
{
    asm
    {
        pshs    x,b

        // CMOC's printf() converts \n to \r for Color Basic's PUTCHR,
        // but processConsoleOutChar() expects \n to mean newline.
        //
        cmpa    #13
        bne     @notCR
        lda     #10
@notCR:
        pshs    a
        clr     ,-s     // push argument as a word
        lbsr    processConsoleOutChar
        leas    2,s

        puls    b,x
    }
}


void hiResTextCommonInit(const struct HiResTextScreenInit *init)
{
    hiResTextConfig.pfWriteCharAt = init->writeCharAtFuncPtr;

    moveCursor(0, 0);

    hiResTextConfig.textScreenBuffer = init->textScreenBuffer;

    #ifndef HIRESTEXT_NO_VT52
    initVT52();
    #endif

    hiResTextConfig.hiResTextCursorPresent = FALSE;
    if (init->redirectPrintf)
        hiResTextConfig.oldCHROUT = setConsoleOutHook(hiResTextConsoleOutHook);
    else
        hiResTextConfig.oldCHROUT = NULL;
    
    hiResTextConfig.hiResTextTimerPtr = init->timer;
    hiResTextConfig.inkeyFuncPtr = init->inkeyFuncPtr;

    if (init->cursorAnimationLowTimerByteMask == 0)
        hiResTextConfig.cursorAnimationLowTimerByteMask = 0x30;
    else
        hiResTextConfig.cursorAnimationLowTimerByteMask = init->cursorAnimationLowTimerByteMask;

    hiResTextConfig.bellFuncPtr = init->bellFuncPtr;

    #if defined(_COCO_BASIC_) || defined(DRAGON)
    if (!hiResTextConfig.inkeyFuncPtr)
        hiResTextConfig.inkeyFuncPtr = inkey;
    if (!hiResTextConfig.bellFuncPtr)
        hiResTextConfig.bellFuncPtr = hiResTextBasicShortSound;
    #endif

    inverseVideoMode = FALSE;
    boldMode = FALSE;
}
