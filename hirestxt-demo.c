/*  hirestxt-demo.c - Example program that uses hirestxt.h.

    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.

    This demo uses VT52 sequences.
    The library must NOT have been compiled with HIRESTEXT_NO_VT52.
*/


#include "hirestxt.h"


#define USE_51x24
#define USE_42x24


enum
{
    BREAK = 3,
    IRQ_VECTOR = 0xFFF8,
    COCO3_SCREEN = 0x8000,
};


// Palette slot names for ucbLogoPaletteValues[].
//
enum
{
    C_BLACK,    C_BLUE,     C_GREEN,    C_CYAN,
    C_RED,      C_MAGENTA,  C_YELLOW,   C_WHITE,
    C_BROWN,    C_TAN,      C_FOREST,   C_AQUA,
    C_SALMON,   C_PURPLE,   C_ORANGE,   C_GREY
};


word *timerAddress;
BOOL isCoCo3;


static void
sleepTicks(word numTicks)
{
    word start = *timerAddress;
    while (*timerAddress - start < numTicks)
        ;
}


// use4x8Font: TRUE to use the 4x8 font (gives 51 columns in PMODE 4, 64 in 320x192 mode),
//             FALSE to use the 5x8 font (gives 42 columns in PMODE 4).
// Returns TRUE if the user wants to continue to the next demo.
// Returns FALSE if the user pressed the Break key.
//
static BOOL demoContents(BOOL use4x8Font, BOOL useCoCo3Screen)
{
    const byte cols = (use4x8Font ? (useCoCo3Screen ? 64 : 51) : 42);

    byte line = 0;
    char title[] = " Demo of " PACKAGE " " VERSION " ";  // these IDs must be defined by -D on compiler cmd line
    setInverseVideoMode(TRUE);
    writeCenteredLine(line++, title);
    setInverseVideoMode(FALSE);
    line++;

    writeCenteredLine(line++, "Latin-1 (ISO-8859-1) character set ");

    char desc[] =             "on a 51x24 software text screen    ";

    if (cols != 51)
        desc[5] = '0' + cols / 10, desc[6] = '0' + cols % 10;
    writeCenteredLine(line++, desc);
    line++;

    const byte leftCol = (cols - 35) / 2;

    // Display a 0, 1, 2, ..., E, F row at the top of the charactable table.
    //
    moveCursor(leftCol + 3, line);
    for (byte j = 0; j < 16; ++j)
    {
        writeChar(j < 10 ? '0' + j : 'A' + (j - 10));
        writeChar(' ');
    }
    line++;
    moveCursor(leftCol + 3, line);
    printf("- - - - - - - - - - - - - - - -");
    line++;

    // Display rows 2..7, 10..15 of character table (code points 32..127, 160..255).
    //
    for (byte i = 2; i < 16; ++i, ++line)
    {
        moveCursor(leftCol, line);
        printf("%X: ", i);
        if (i == 8 || i == 9)  // font has no chars to display in 128..159 range
            continue;
        byte highI = i << 4;
        for (byte j = 0; j < 16; ++j)
        {
            writeChar(highI | j);
            writeChar(' ');
        }
    }
    
    // Fill the leftmost and rightmost columns:
    //
    for (byte y = 0; y < 24; ++y)
    {
        writeCharAt(0, y, '*');
        writeCharAt(cols - 1, y, '*');
    }

    // Show a prompt two lines below the table:
    //
    line++;
    setBoldMode(TRUE);
    writeCenteredLine(line, "Press a key to continue: ");
    setBoldMode(FALSE);

    if (waitKeyBlinkingCursor() == BREAK)
        return FALSE;

    // Demonstrate printf() and automatic scrolling:
    //
    printf("\033E");  // clear screen
    for (byte i = 0; i < 26; ++i)
    {
        printf("%c\n", 'A' + i);
        sleepTicks(3);
    }

    printf("\n""The screen scrolls automatically\n"
               "when the cursor reaches the bottom.\n\n");

    writeString("Press a key to continue: ");
    if (waitKeyBlinkingCursor() == BREAK)
        return FALSE;

    // VT52 demo via printf() (writeString() does not go
    // through processVT52()). Here, we clear the screen
    // without clrscr().
    //
    printf("\x1BH");  // cursor to home
    printf("\x1BJ");  // erase to end of screen
    printf("The screen has just been cleared with the\n"
           "VT52 sequence <ESC> H <ESC> J.\n"
           "\n"
           "\x1BpThis part is inverted\x1Bq because\n"
           "of VT52 sequences <ESC> p and <ESC> q.");

    line = 7;
    byte col = 10;
    moveCursor(col, line);
    printf("This line starts at column %u\n"
           "of line %u because of moveCursor().\n", col, line);

    printf("\x1BY\x2B\x25This line starts at column 5\n"
           "of line 11 because of VT52 sequence\n"
           "<ESC> Y <row> <column>.\n");

    moveCursor(0, 15);
    printf("This line contains an ");
    setInverseVideoMode(TRUE);
    printf("inverted");
    setInverseVideoMode(FALSE);
    printf(" word\n"
           "and a ");
    setBoldMode(TRUE);
    printf("bold");
    setBoldMode(FALSE);
    printf(" one because of functions\n"
           "setInverseVideoMode() and setBoldMode().");

    line = 19;
    const char *prompt0 = "The cursor can blink over text.";
    const char *prompt1 = "Press a key to continue.";
    col = 0;
    moveCursor(col, line);
    writeString(prompt0);
    moveCursor(col, line + 1);
    writeString(prompt1);
    moveCursor(col + 21, line);  // move to 'o' of "over"

    if (waitKeyBlinkingCursor() == BREAK)
        return FALSE;

    // Demo the bell character.
    //
    clrscr();
    byte len = 26, x = (cols - len) / 2;
    moveCursor(x, 10);
    printf("A sound\a can be played when ");
    moveCursor(x, 11);
    printf("character 7 is printed.");
    moveCursor(x, 13);
    printf("Press a key to continue: ");

    if (waitKeyBlinkingCursor() == BREAK)
        return FALSE;

    if (useCoCo3Screen)
    {
        // Demo the colors on the CoCo 3.
        //
        setBackgroundColor(C_BLACK);
        setForegroundColor(C_YELLOW);
        clrscr();
        writeCenteredLine(1, " Functions setForegroundColor() and setBackgroundColor() allow");
        writeCenteredLine(2, " a choice among 16 colors. Here are some random combinations. ");
        writeCenteredLine(3, " The CoCo 3's palette allows further customization.           ");
        byte left = 2, top = 5;
        for (byte col = 0; col <= 7; ++col)
        {
            byte bgColor = col + 8;
            setBackgroundColor(bgColor);
            for (byte row = 0; row <= 7; ++row)
            {
                byte fgColor = row;
                setForegroundColor(fgColor);
                moveCursor(left + col * 8, top + row * 2);
                printf("%2u:%2u", fgColor, bgColor);
            }
        }

        setBackgroundColor(C_BLACK);
        setForegroundColor(C_YELLOW);
        writeCenteredLine(21, "Press a key to continue: ");

        if (waitKeyBlinkingCursor() == BREAK)
            return FALSE;
    }

    return TRUE;
}


#ifdef OS9  /* Keyboard support. */


#include "Keyboard.h"  /* bcontrol library */

static Keyboard theKeyboard;

// For HiResTextScreenInit2::inkeyFuncPtr.
//
static byte pollBControlKeyboard()
{
    byte ch = Keyboard_poll(&theKeyboard);
    if (ch == '!')  // patch b/c Ctrl-C/Ctrl-E/Break do not return code 3 on OS-9
        return BREAK;
    return ch;
}


// frequency: 0=low, 4095=high.
// duration: 0..255.
// amplitude: 0..63.
//
static void playTone(word frequency, byte duration, byte amplitude)
{
    asm
    {
        pshs    y       ; preserve global data pointer
        lda     :amplitude
        ldb     :duration
        tfr     d,x
        ldd     #$0198  ; 1 = path number, $98 = SS.Tone function number
        ldy     :frequency
        os9     $8E     ; I$SetStt (Set Status)
        puls    y
    }
}


static void playOS9Bell(void)
{
    playTone(3072, 8, 48);
}


#endif  /* OS9 */


#if defined(_COCO_BASIC_) || defined(DRAGON)


// Setup and show a PMODE 4 green/black graphics screen.
//
static void showPMode4ScreenAtBuffer(byte *screenBuffer)
{
    width(32);  // PMODE graphics will only appear from 32x16 (does nothing on CoCo 1&2)
    pmode(4, screenBuffer);
    pcls(255);
    screen(1, 0);  // green/black
}


#endif


#ifdef _COCO_BASIC_  /* CoCo 3 support. */


// The following keyboard system is rudimentary, because of the absence of Color Basic.
// It only supports the space and break keys.
// For more complete keyboard support, one could use the BControl library.
//
typedef struct KeyState
{
    byte charValue;      // ASCII value
    byte keyProbeValue;  // KEY_PROBE_* value from <coco.h>
    byte keyBitValue;    // KEY_BIT_* value from <coco.h>
    BOOL wasPressed;     // must be initialized to FALSE
} KeyState;


byte KeyState_process(KeyState *self)
{
    BOOL currenltyPressed = isKeyPressed(self->keyProbeValue, self->keyBitValue);
    BOOL justPressed = (!self->wasPressed && currenltyPressed);
    self->wasPressed = currenltyPressed;  // remember for next call
    return justPressed ? self->charValue : '\0';
}


KeyState spaceKey = { ' '  , KEY_PROBE_SPACE, KEY_BIT_SPACE, FALSE };
KeyState breakKey = { '\x3', KEY_PROBE_BREAK, KEY_BIT_BREAK, FALSE };


// The signature of this function must be of the type of HiResTextScreenInit2::inkeyFuncPtr.
//
byte pollCoCo3Keyboard()
{
    byte key = KeyState_process(&spaceKey);
    if (key)
        return key;
    key = KeyState_process(&breakKey);
    if (key)
        return key;
    return 0;
}


typedef interrupt void (*ISR)(void);


// Counter that gets incremented 60 times per second, until the graphics driver is initalized.
//
word timer;


static interrupt void
irqISR()
{
    asm
    {
_dskcon_irqService IMPORT
        ldb     $FF03
        bpl     @done           // do nothing if 63.5 us interrupt
        ldb     $FF02           // 60 Hz interrupt. Reset PIA0, port B interrupt flag.
;
        ldd     :timer
        addd    #1
        std     :timer
@done
    }
}


// Writes a JMP instruction at 'vector', followed by the address in newRoutine.
// Must be called while interrupts are disabled.
// vector: e.g., 0xFFF8 for IRQ.
//
void
setISR(void *vector, ISR newRoutine)
{
    byte *isr = * (byte **) vector;
    *isr = 0x7E;  // JMP extended
    * (ISR *) (isr + 1) = newRoutine;
}


// RGB approximation of the 16 colors specified by the Berkeley Logo User Manual
// for the SETPENCOLOR command:
//	 0  black	 1  blue	 2  green	 3  cyan
//	 4  red		 5  magenta	 6  yellow	 7  white
//	 8  brown	 9  tan		10  forest	11  aqua
//	12  salmon	13  purple	14  orange	15  grey
//
static const byte ucbLogoPaletteValues[16] =
{
     0,  9, 18, 27,
    36, 45, 54, 63,
    34, 53, 20, 29,
    60, 40, 38, 56,
};


static void
resetGraphicsPalette(void)
{
    // Initialize the palette. Assumes RGB.
    memcpy((void *) 0xFFB0, ucbLogoPaletteValues, sizeof(ucbLogoPaletteValues));
}


// Store the 320x192x16 screen buffer at absolute address $78000,
// i.e., over the Basic interpreter.
// Use addresses $8000..$FCFF (cf COCO3_SCREEN) to access that buffer.
// This involves redirecting the IRQ to this program's own service routine,
// since Basic's ISR will get overwritten.
// Clears the graphics screen.
// Sets the palette for an RGB screen.
//
// color: 0..15.
//
void showCoCo3GraphicsMode(byte color)
{
    disableInterrupts();
    setISR(IRQ_VECTOR, irqISR);  // no need for Basic's ISR anymore
    enableInterrupts();

    word wordToClearWith = color | (color << 4) | ((word) color << 8) | ((word) color << 12);
    // Clear the graphics screen before showing it.
    memset16(COCO3_SCREEN, wordToClearWith, 320u * 192 / 4);  // u suffix forces unsigned arithmetic, avoids negative overflow

    // Set CoCo 3 graphics mode.
    * (byte *) 0xFF90 = 0x4C;  // reset CoCo 2 compatible bit
    * (byte *) 0xFF98 = 0x80;  // graphics mode
    * (byte *) 0xFF99 = 0x1E;  // 320x192, 16 colors
    * (byte *) 0xFF9A = 0;  // border color (0..63)

    * (word *) 0xFF9D = 0xF000;  // 0x78000 >> 3 (must be consistent with COCO3_SCREEN)

    resetGraphicsPalette();
}


void
enableSound(void)
{
    // Based on Color Basic code at $9A2B.
    byte *pia0 = (byte *) 0xFF00;
    pia0[1] &= 0xF7;
    pia0[3] &= 0xF7;
    byte *pia1 = (byte *) 0xFF20;
    pia1[3] |= 0x08;
}


void
disableSound(void)
{
    // Based on ECB code at $A974.
    * (byte *) 0xFF23 &= 0xF7;
}


void playCoCo3Bell(void)
{
    enableSound();

    // Play a short 200 Hz square wave.
    asm
    {
        ldx     #200/12             ; number of complete periods to play of the square wave
@playLoop
        lda     #(32+16)*4          ; load high value of square wave (2 cycles)
        bsr     @playA              ; (6 cycles)
        lda     #(32-16)*4          ; load low value (2 cycles)
        bsr     @playA              ; (6)
        leax    -1,x                ; (5)
        bhi     @playLoop           ; (3) 
        bra     @done
@playA
;
; // A half-period span a BSR followed by this @playA routine,
; // i.e., 6 + (4 + 3 + k * (3 + 3) + 4),
; // where k is the number of iteations of @delayLoop.
;
; // We want this number of cycles to last 1 / F / 2 second,
; // where F is the frequency in Hz of the square wave (e.g., 200 Hz).
; // 1 second is 894886 cycles on a CoCo without any high speed poke.
;
; // So we have 6 + (4 + 3 + k * (3 + 3) + 4) == 894886 / F / 2,
; // i.e., 17 + 6 * k == 447443 / 200
; // i.e., k = (447443 / 200 - 17) / 6.
; // i.e., k = 370.0358333.
;
        sta     $FF20               ; (4)
        ldd     #370                ; (3) half-period delay for chosen frequency
@delayLoop
        subd    #1                  ; (3)
        bhi     @delayLoop          ; (3)
        rts                         ; (4)
@done
    }

    disableSound();
}


#endif  /* CoCo 3 support. */


static void restoreTextScreen(void)
{
    #if defined(_COCO_BASIC_) || defined(DRAGON)
    pmode(0, 0);
    cls(255);
    screen(0, 0);
    #elif defined(OS9)
    quitOS9Graphics();
    #endif
}


// use4x8Font: TRUE to use the 4x8 font (gives 51 columns in PMODE 4, 64 in 320x192 mode),
//             FALSE to use the 5x8 font (gives 42 columns in PMODE 4).
// Returns TRUE if the user wants to continue to the next demo.
// Returns FALSE if the user pressed the Break key or if
// the request demo is not supported.
//
static BOOL demo(BOOL use4x8Font, BOOL useCoCo3Screen)
{
    #ifdef OS9
    Keyboard_init(&theKeyboard);

    byte *screenBuffer = showOS9PMode4Screen();
    if (screenBuffer == NULL)
    {
        printf("Failed to set up graphics screen.\n");
        return FALSE;
    }
    #else
    if (useCoCo3Screen && !use4x8Font)
        return FALSE;  // 5x8 not supported in 4 bpp mode
    #endif

    // initHiResTextScreen() must be called first.
    // Assumes 4 graphics pages reserved at the current start of graphics RAM.
    // TRUE requests that printf() be redirected to the VT52 interpreter,
    // which writes to the 51x24 screen.
    //
    struct HiResTextScreenInit2 init =
        {
            {   // member 'init', of type struct HiResTextScreenInit:

                use4x8Font ? 51 : 42,
                #if defined(USE_51x24) && defined(USE_42x24)
                !use4x8Font ? writeCharAt_42cols
                        : (useCoCo3Screen ? writeCharAt_320x16
                                            : writeCharAt_51cols),
                #elif defined(USE_51x24)
                useCoCo3Screen ? writeCharAt_320x16
                            : writeCharAt_51cols,
                #else
                writeCharAt_42cols,
                #endif

                #if defined(_COCO_BASIC_) || defined(DRAGON)
                useCoCo3Screen ? (byte *) COCO3_SCREEN
                            : (byte *) * (byte *) 0x00BC << 8,  // Get start of graphics RAM from Color Basic variable.
                #elif defined(OS9)
                screenBuffer,
                #else
                #error
                #endif

                TRUE,
                timerAddress,
                0,  // cursorAnimationLowTimerByteMask

                #if defined(_COCO_BASIC_)
                useCoCo3Screen ? pollCoCo3Keyboard : (byte (*)(...)) NULL,  // keyboard polling function (defaut is inkey())
                #elif defined(DRAGON)
                NULL,
                #elif defined(OS9)
                pollBControlKeyboard,
                #endif

                #if defined(_COCO_BASIC_)
                useCoCo3Screen ? playCoCo3Bell : (void (*)(void)) NULL,  // bell function (default is Basic's SOUND 1,1)
                #elif defined(DRAGON)
                NULL,
                #elif defined(OS9)
                playOS9Bell,
                #endif
            },

            useCoCo3Screen ? 320 : 0,  // numPixelsPerRow
            useCoCo3Screen ? 192 : 0,  // numPixelsRowPerScreen
            useCoCo3Screen ? 4 : 0,  // numBitsPerPixel
            useCoCo3Screen ? C_YELLOW : 0,  // fgColor
            useCoCo3Screen ? C_RED : 0,  // fgBoldColor
            useCoCo3Screen ? C_BLUE : 0,  // bgColor
        };

    #if defined(_COCO_BASIC_)
    if (useCoCo3Screen)
        showCoCo3GraphicsMode(init.bgColor);
    else
        showPMode4ScreenAtBuffer(init.init.textScreenBuffer);
    #elif defined(DRAGON)
    showPMode4ScreenAtBuffer(init.init.textScreenBuffer);
    #elif defined(OS9)
    memset16(screenBuffer, 0xFFFF, 6144 / 2);  // clear PMODE 4 screen to green
    #endif

    initHiResTextScreen2(&init);

    BOOL cont = demoContents(use4x8Font, useCoCo3Screen);

    if (useCoCo3Screen)
    {
        // Cannot go back to Basic from the CoCo 3 demo,
        // so print a message instead.
        clrscr();
        writeCenteredLine(11,
                "End of the demo. Reboot the CoCo to go back to Basic.");
    }

    closeHiResTextScreen();  // this unhooks printf() from hirestxt

    if (!useCoCo3Screen)  // CoCo 3 demo does not go back to text mode
        restoreTextScreen();

    #ifdef OS9
    Keyboard_shutdown(&theKeyboard);
    #endif

    return cont;
}


int main()
{
    #if 0  /* Useful when developing. */
    printf("main() build time:\n%s %s\nPress Enter: ", __DATE__, __TIME__);
    readline();
    #endif

    BOOL useCoCo3Screen = FALSE;

    #ifdef OS9
    OS9Timer_init(1);
    timerAddress = OS9Timer_getTimerAddress();
    #else
    isCoCo3 = (* (word *) 0xFFF8 == 0xFEF7);
    if (isCoCo3)
    {
        rgb();  // so that the cursor flashes the right colors
        printf("USE 320X192X16 MODE? (Y/N) ");
        const char *line = readline();
        if (toupper(line[0]) != 'N')
            useCoCo3Screen = TRUE;
    }

    #ifdef _COCO_BASIC_
    if (useCoCo3Screen)
        timerAddress = &timer;
    else
    #endif
        timerAddress = (word *) 0x0112;  // Color Basic TIMER variable

    #endif

    #if defined(USE_51x24) && defined(USE_42x24)
    if (demo(TRUE, useCoCo3Screen))
    {
        if (!useCoCo3Screen)
            demo(FALSE, FALSE);  // 42-column mode
    }
    #elif defined(USE_51x24)
    demo(TRUE, useCoCo3Screen);
    #else
    if (!useCoCo3Screen)
        demo(FALSE, FALSE);  // 42-column mode
    #endif

    #ifdef OS9
    OS9Timer_shutdown();
    #endif

    if (useCoCo3Screen)
    {
        // Cannot go back to Basic, which got overwritten.
        for (;;)
            ;
    }

    printf("END OF THE DEMO.\n");  // printf() now writes to normal console
    return 0;
}
