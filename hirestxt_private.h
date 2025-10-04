/*  hirestxt_private.h - 51x24 black-on-green PMODE 4 text screen.

    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.

*/

#include "hirestxt.h"


typedef struct HiResTextConfig
{
    byte hiResWidth;             // characters per row
    word numPixelsPerRow;        // if 0, 256 is assumed
    byte numPixelsRowPerScreen;  // if 0, 192 is assumed (N.B.: Only 192 is supported by this version)
    byte numBitsPerPixel;        // if 0, 1 is assumed

    // These colors masks contain two identical nybbles.
    // They are only used in 4-bpp modes.
    byte fgColorMask;            // default foreground color byte
    byte fgBoldColorMask;        // used when boldMode is non-zero
    byte bgColorMask;            // background color byte

    byte textPosX;
    byte textPosY;
    byte *textScreenBuffer;
    BOOL hiResTextCursorPresent;
    word *hiResTextTimerPtr;  // address of a word that gets incremented 60 times per second
    ConsoleOutHook oldCHROUT;
    void (*pfWriteCharAt)(byte x, byte y, byte asciiCode);  // if asciiCode is 0, inverts char at (x, y)
    byte (*inkeyFuncPtr)();
    byte cursorAnimationLowTimerByteMask;
    void (*bellFuncPtr)(void);

    #ifndef HIRESTEXT_NO_VT52
    byte vt52State;  // 0..HIRESHEIGHT-1, unlike VT52 spec which starts at 1.
    byte vt52Line;  // 0..HIRESHEIGHT-1, unlike VT52 spec which starts at 1.
    byte vt52NumBytesToIgnore;  // Number of coming bytes that will be ignored (see VT52_IGNORE_NEXT).
    #endif  /* HIRESTEXT_NO_VT52 */
} HiResTextConfig;


extern HiResTextConfig hiResTextConfig;


// When TRUE, writing characters writes them in inverted colors.
//
extern BOOL inverseVideoMode;

// When TRUE, writing characters writes them in bold.
//
extern BOOL boldMode;


#ifndef HIRESTEXT_NO_VT52


// VT52 state machine states.
//
enum
{
    VT52_TEXT           = 0,  // Chars interpreted as text to display, except Escape.
    VT52_GOT_ESC        = 1,  // Escape has just been received.
    VT52_WANT_LINE      = 2,  // Y command waiting for line character.
    VT52_WANT_COL       = 3,  // Y command waiting for column character.
    VT52_IGNORE_NEXT    = 4,  // Ignore next byte(s), depending on 'vt52NumBytesToIgnore'.
};


void hiResTextConsoleOutHook(void);


#if defined(_COCO_BASIC_) || defined(DRAGON)

void hiResTextBasicShortSound(void);

#endif


void hiResTextCommonInit(const struct HiResTextScreenInit *init);


byte fillNybbles(byte nybble);


#endif  /* HIRESTEXT_NO_VT52 */
