/*  hirestxt.h - 51x24 black-on-green PMODE 4 text screen.

    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.

    Quick Guide:
        - Call initHiResTextScreen() first.
        - End with a call to closeHiResTextScreen().
        - #define HIRESTEXT_NO_VT52 to avoid compiling the VT52 code.
*/

#ifndef _hirestxt_h_
#define _hirestxt_h_

#include <coco.h>


// Pointer to a function that writes a character at position (x, y).
// If asciiCode is zero, the colors that (x, y) must be inverted.
//
typedef void (*WriteCharAtFuncPtr)(byte x, byte y, byte asciiCode);


// Initializer to be used when calling initHiResTextScreen().
//
// Useful for 51x24 and 42x24 text screens on a PMODE 4 screen.
//
// To have a 64x24 text screen in the CoCo 3's 320x192x16 mode,
// call initHiResTextScreen2() instead.
//
// numColumns: Number of characters per text row, in PMODE 4: 42 or 51.
//             If using a 320x192x16 mode, pass 51 to get 64-column screen.
//             In such a mode, passing 42 is not supported.
//
// writeCharAtFuncPtr: Either writeCharAt_42cols, writeCharAt_51cols
//                     or writeCharAt_320x16.
//                     Must be consistent with numColumns.
//
// textScreenBuffer: Address of a 256x192x2 or 320x192x16 graphics buffer.
//
// redirectPrintf: If true, printf() will print to the hi-resolution screen
//                 until closeHiResTextScreen() is called.
//
// timer: Address of a word that gets incremented 60 times per second.
//        Used to animate the cursor.
//
// cursorAnimationLowTimerByteMask: Byte to be ANDed with the low byte of the word
//                                  pointed to be 'timer'. The cursor is only displayed
//                                  when the result of this AND differs from
//                                  cursorAnimationLowTimerByteMask. A value of 0x30
//                                  with a 60 Hz timer will show the cursor during
//                                  75% of the cycle.
//                                  Pass 0 to use the default value.
//
// inkeyFuncPtr: Function that polls the keyboard and returns a non-zero
//               character code if a key has been pressed, or zero if no key
//               has been pressed.
//               Under ECB, this field can be left null: inkey() will be used then.
//
// bellFuncPtr: Function that plays a short bell sound.
//              Called when character '\a' is sent to the screen.
//              Allowed to be null, when no sound is wanted.
//
struct HiResTextScreenInit
{
    byte numColumns;
    WriteCharAtFuncPtr writeCharAtFuncPtr;
    byte *textScreenBuffer;
    BOOL redirectPrintf;
    word *timer;
    byte cursorAnimationLowTimerByteMask;
    byte (*inkeyFuncPtr)();
    void (*bellFuncPtr)(void);
};


// Initializer to be used when calling initHiResTextScreen2().
// Introduced by hirestxt 0.5.0.
// Use this to specify the use of a 64x24 text screen in the CoCo 3's 320x192x16 mode.
//
struct HiResTextScreenInit2
{
    struct HiResTextScreenInit init;

    // New with version 0.5.x:
    word numPixelsPerRow;        // if 0, 256 is assumed; must be divisible by 8
    byte numPixelsRowPerScreen;  // if 0, 192 is assumed (CAUTION: Only 192 is supported by this version)
    byte numBitsPerPixel;        // if 0, 1 is assumed
    byte fgColor;                // default foreground color (0..15); only used in 4-bit pixel modes
    byte fgBoldColor;            // bold foreground color (0..15); only used in 4-bit pixel modes
    byte bgColor;                // background color (0..15); only used in 4-bit pixel modes
};


enum
{
    HIRESHEIGHT                   = 24u,    // number of text lines
    PIXEL_COLS_PER_SCREEN         = 256u,   // assuming PMODE 4
    PIXEL_ROWS_PER_SCREEN         = 192u,   // assuming PMODE 4
    PIXEL_ROWS_PER_TEXT_ROW       = 8u,
    BYTES_PER_PIXEL_ROW           = 32u,    // assuming PMODE 4
};


// Call this first, with a non null pointer to an initializer struct.
//
// See struct HiResTextScreenInit above.
//
// To have a 64x24 text screen in the CoCo 3's 320x192x16 mode,
// call initHiResTextScreen2() instead.
//
// Does not keep a reference of the HiResTextScreenInit object.
//
// Redirects printf(), putchar(), etc. to this hi-res text screen
// if init->redirectPrintf is true.
//
// The caller must clear the screen to green before starting to
// write characters.
//
void initHiResTextScreen(const struct HiResTextScreenInit *init);


// Use this to specify the use of a 64x24 text screen in the CoCo 3's
// 320x192x16 mode.
//
void initHiResTextScreen2(const struct HiResTextScreenInit2 *init);


// Call this last.
// Unhooks printf() from this hi-res text screen, if it was redirected
// by initHiResTextScreen().
//
void closeHiResTextScreen(void);


// Writes a PRINTABLE 4x8 character at column x and row y of a 51x24 text screen.
// x: 0..hiResWidth-1.
// y: 0..HIRESHEIGHT-1.
// asciiCode: MUST be in the range 32..127 or 160..255, except if 0,
// which means invert colors at position (x, y).
// Uses textScreenBuffer, frameByteAddrTable[], frameBitOffsetTable[] and
// frameMaskTable[].
//
// Does NOT advance the cursor.
//
void writeCharAt(byte x, byte y, byte asciiCode);


// Writes a character at position (x, y) on a 42x24 text screen
// in a 256x192x2 graphics mode.
//
void writeCharAt_42cols(byte x, byte y, byte asciiCode);


// Writes a character at position (x, y) on a 42x24 text screen
// in a 256x192x2 graphics mode.
//
void writeCharAt_51cols(byte x, byte y, byte asciiCode);


// Writes a character at position (x, y) on a 64x24 text screen
// in a 320x192x16 graphics mode.
//
void writeCharAt_320x16(byte x, byte y, byte asciiCode);


void invertPixelsAtCursor(void);


// Scrolls the 51x24 screen one text row up.
// Fills the bottom text row with set pixels.
//
void scrollTextScreenUp(void);


// Places the cursor at column x (0..hiResWidth) of row y (0..HIRESHEIGHT-1).
// Does nothing if x or y are out of range.
//
void moveCursor(byte x, byte y);


#ifndef HIRESTEXT_NO_VT52
void initVT52(void);
#endif


// Clear text rows from textRow (0..HIRESHEIGHT-1) to the end of the screen.
// Does nothing if textRow is out of range.
//
void clearRowsToEOS(byte byteToClearWith, byte textRow);

// Clear rowsToClear text rows starting from textRow (0..HIRESHEIGHT-1).
// Does nothing if textRow or rowsToClear is out of range.
//
void clearRowsN(byte byteToClearWith, byte textRow, byte rowsToClear);

// Puts the cursor in the upper left position.
//
#define home() (moveCursor(0, 0))


// Returns the column number where the cursor is.
// This column number is allowed to be equal to the width of the text screen.
// When this many characters have been written on a line, the cursor is
// considered to be not on the next line, but past the last displayed column
// of the original line.
// Then when a '\n' is written, the cursor goes to column 0 of the next line.
// This avoids having the '\n' insert an empty line.
// See how '\n' is processed by writeChar().
//
byte getCursorColumn(void);


// Returns the row number where the cursor is.
//
byte getCursorRow(void);


// Fills the screen with spaces (does not move the cursor).
//
void clear(void);

// Fills n rows of the screen with spaces (does not move the cursor).
//
void clear(void);

void clearn(byte n);


// Homes the cursor and clears the screen.
//
void clrscr(void);

// Homes the cursor and clears N rows from the start of the screen
//
void clrscrn(byte n);


// Writes a character at the current cursor position and advances the cursor.
// Scrolls the screen up one row if the cursor would go off the bottom.
// Ignores non-printable characters.
// str: Supports \a, \b, \t, \n, \f, \r.
//
void writeChar(byte ch);


// Calls writeChar() for each character in the given string,
// up to and excluding the terminating '\0'.
//
void writeString(const char *str);


// row: 0..HIRESHEIGHT-1
// line: String of at most hiResWidth characters.
// Leaves the cursor at the end of the line.
//
void writeCenteredLine(byte row, const char *line);


// Writes unsigned word 'w' to the screen in decimal.
//
void writeDecWord(word w);


// Writes spaces from the cursor to the end of the current line.
// Does NOT move the cursor.
//
void clrtoeol(void);


// Writes spaces from the cursor to the end of the screen.
// Does NOT move the cursor.
//
void clrtobot(void);


// Removes the cursor if present (it is typically displayed
// by animateCursor()).
//
void removeCursor(void);


// To be called periodically.
// Call removeCursor() when the animation must stop.
//
void animateCursor(void);


// Returns ASCII code of pressed key (using inkey(), which calls Color Basic).
// Uses textPosX, textPosY.
// Invokes the function given in field inkeyFuncPtr of the HiResTextScreenInit
// object that was passed to initHiResTextScreen(), or inkey() (declared by
// <coco.h>) if that field was null.
//
byte waitKeyBlinkingCursor(void);


// Pass TRUE to start writing characters in inverted colors,
// FALSE to go back to normal colors.
//
void setInverseVideoMode(BOOL invert);


// Pass TRUE to start writing characters in bold,
// FALSE to go back to non-bold.
//
// N.B.: Inverting bold characters is not supported in a 4-bit pixel mode.
//
void setBoldMode(BOOL bold);


// Change the text colors when using the 320x192x16 graphics mode.
// color: 0..15.
//
void setForegroundColor(byte color);
void setForegroundBoldColor(byte color);
void setBackgroundColor(byte color);


///////////////////////////////////////////////////////////////////////////////


#ifndef HIRESTEXT_NO_VT52

// Resets the VT52 state machine.
//
void initVT52(void);


#endif  /* HIRESTEXT_NO_VT52 */


// Calls writeChar() and moveCursor() to execute the VT52 sequence
// provided by the calls to this function.
//
// Source: http://bitsavers.trailing-edge.com/pdf/dec/terminal/gigi/EK-0GIGI-RC-001_GIGI_Programming_Reference_Card_Sep80.pdf
//
void processConsoleOutChar(byte ch);


#ifdef OS9


// Shows the PMODE 4 screen when the standard output is a VDG terminal.
// Does not clear the screen, which has 6144 bytes.
// Returns the address of the 256x192 graphics screen, upon success.
// Returns NULL if the system call failed.
// To restore the text mode, write character 14 to the standard output,
// e.g., with putchar(14).
//
byte *showOS9PMode4Screen(void);


// Shows the text mode and frees the graphics memory acquired by showOS9PMode4Screen().
//
void quitOS9Graphics(void);


#ifndef OMIT_OS9_TIMER


// Initializes a timer system that maintains a 16-bit unsigned counter.
// This counters wraps to 0 when it passes 65535.
//
// ticksPerIncrement: How many sixtieths of a second before the timer is incremented by 1.
//                    Pass 1 to have a counter similar to Color Basic's TIMER.
//
// Call OS9Timer_shutdown() when finished with the use of the timer.
//
// CAUTION: OS-9 F$Sleep call gets stopped by the interrupts caused by this timer system.
//          This means that CMOC's delay() function is affected too.
//          Delays may be achieved with OS9Timer_delay() instead.
//
byte OS9Timer_init(unsigned ticksPerIncrement);


// Releases the resources used by this timer system.
//
byte OS9Timer_shutdown(void);


// Returns the address of the periodically incremented counter.
//
word *OS9Timer_getTimerAddress(void);


#endif  /* ndef OMIT_OS9_TIMER */


#endif  /* OS9 */


#endif  /* _hirestxt_h_ */
