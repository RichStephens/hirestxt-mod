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


// Fill levels from empty to solid, for shading by proportion.
//
static const byte shadeRamp[5] = { ' ', 177, 174, 182, 183 };

// The same levels without the empty one: an empty cell shows background and
// so carries no hue, which defeats a color gradient. The monochrome modes
// keep the empty level, shape being their only cue.
//
static const byte solidRamp[4] = { 177, 174, 182, 183 };

// Palette slots from cool to hot, walked in step with solidRamp so the
// block gradates in hue as well as density. Starts at aqua, not blue: the
// background is blue and a ramp through it loses its cool end. Ends on red
// rather than white, which reads as a blowout rather than a peak.
//
static const byte hueRamp[9] = { C_AQUA,   C_CYAN,   C_FOREST,
                                 C_GREEN,  C_TAN,    C_YELLOW,
                                 C_ORANGE, C_SALMON, C_RED };


// Where the text screen lives, and its shape. hiResTextConfig is private to
// the library, so demo() copies it here: the graphics page plots into
// exactly the buffer the text glyphs are written into.
//
byte *gfxBuffer;
word gfxWidth;
byte gfxBpp;


// A classic 11x8 invader, one bit per pixel in bits 10..0.
//
static const word invaderRows[8] =
{
    0x104,  // ..#.....#..
    0x088,  // ...#...#...
    0x1FC,  // ..#######..
    0x376,  // .##.###.##.
    0x7FF,  // ###########
    0x5FD,  // #.#######.#
    0x505,  // #.#.....#.#
    0x0D8,  // ...##.##...
};


// Palette slots for the invader, a row each, so it comes out banded in the
// 16-color mode. Ignored where there are only two colors to play with.
//
static const byte invaderHues[8] =
{
    C_MAGENTA, C_PURPLE, C_BLUE,   C_CYAN,
    C_GREEN,   C_YELLOW, C_ORANGE, C_RED,
};


// Bytes per pixel row of the graphics buffer.
//
static word
gfxBytesPerRow(void)
{
    return (gfxBpp == 4 ? (gfxWidth >> 1) : (gfxWidth >> 3));
}


// The byte value that paints a whole run of pixels in one color: both
// nybbles where a byte holds two 4-bit pixels, and all eight bits where it
// holds eight 1-bit pixels. Ink is a reset bit in the PMODE 4 fonts, so a
// non-zero color clears the byte there rather than setting it.
//
static byte
gfxSolidByte(byte color)
{
    if (gfxBpp == 4)
        return (byte) ((color << 4) | color);
    return (byte) (color ? 0x00 : 0xFF);
}


// Paints a horizontal run of pixels: memset() for the whole bytes in the
// middle, a pixel at a time only for the partial byte at each end, so
// shapes are not forced onto byte boundaries.
//
static void
gfxHLine(word x, byte y, word len, byte color)
{
    if (len == 0)
        return;

    word bytesPerRow = gfxBytesPerRow();
    byte *row = gfxBuffer + (word) y * bytesPerRow;
    word xEnd = x + len;                      // exclusive

    if (gfxBpp == 4)
    {
        if (x & 1)                            // partial byte at the left
        {
            byte *p = row + (x >> 1);
            *p = (byte) ((*p & 0xF0) | (color & 0x0F));
            ++x;
        }
        if (xEnd & 1)                         // partial byte at the right
        {
            byte *p = row + (xEnd >> 1);
            *p = (byte) ((*p & 0x0F) | (byte) (color << 4));
            --xEnd;
        }
        if (xEnd > x)
            memset(row + (x >> 1), gfxSolidByte(color), (xEnd - x) >> 1);
    }
    else
    {
        while ((x & 7) != 0 && x < xEnd)      // leading bits
        {
            byte *p = row + (x >> 3);
            byte mask = (byte) (0x80 >> (x & 7));
            if (color)
                *p &= (byte) ~mask;
            else
                *p |= mask;
            ++x;
        }
        while ((xEnd & 7) != 0 && xEnd > x)   // trailing bits
        {
            --xEnd;
            byte *p = row + (xEnd >> 3);
            byte mask = (byte) (0x80 >> (xEnd & 7));
            if (color)
                *p &= (byte) ~mask;
            else
                *p |= mask;
        }
        if (xEnd > x)
            memset(row + (x >> 3), gfxSolidByte(color), (xEnd - x) >> 3);
    }
}


// Paints a rectangle, a span per pixel row.
//
static void
gfxRect(word x, byte y, word w, byte h, byte color)
{
    for (byte dy = 0; dy < h; ++dy)
        gfxHLine(x, (byte) (y + dy), w, color);
}


// Bit positions filled in as ink density rises, spread out rather than
// consecutive so a partly filled byte reads as texture and not as a bar.
//
static const byte ditherOrder[8] = { 0, 4, 2, 6, 1, 5, 3, 7 };


// One byte of an ordered dither: 'density' of the 8 pixels inked, rotated
// by the pixel row so the texture does not line up into vertical stripes.
//
static byte
ditherByte(byte density, byte y)
{
    byte bits = 0;
    for (byte i = 0; i < density; ++i)
        bits |= (byte) (0x80 >> ditherOrder[i]);

    byte sh = (byte) ((y & 3) * 2);
    bits = (byte) ((bits >> sh) | (bits << (8 - sh)));
    return (byte) ~bits;                      // ink is a reset bit
}


// A Tandy Color Computer and its monitor, in design units on a 64x32 grid,
// painted back to front. The 1-bpp column is ink or paper; the 4-bpp column
// is a palette slot.
//
static const byte artX[8]   = {  8, 12, 28, 18,  0,  5,  7,  7 };
static const byte artY[8]   = {  0,  2, 18, 20, 24, 26, 27, 29 };
static const byte artW[8]   = { 48, 40,  8, 28, 64, 54, 50, 50 };
static const byte artH[8]   = { 18, 13,  2,  2,  8,  4,  1,  1 };
static const byte artInk[8] = {  1,  0,  1,  1,  1,  0,  1,  1 };
static const byte artHue[8] =
{
    C_GREY,   // monitor case
    C_BLACK,  // screen
    C_GREY,   // stand neck
    C_GREY,   // stand base
    C_TAN,    // computer body
    C_BROWN,  // keyboard well
    C_TAN,    // key rows
    C_TAN,
};


// Column at which an item of the given width starts, to center it
// in a pane that begins at paneLeft and is paneWidth columns wide.
//
static byte
centerCol(byte paneLeft, byte paneWidth, byte itemWidth)
{
    return paneLeft + (paneWidth - itemWidth) / 2;
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

    // Both glyph sets, on one screen. The font is read as each character is
    // drawn, so the first row keeps the glyphs it was written with after the
    // set is switched underneath it.
    //
    {
        clrscr();
        writeCenteredLine(1, " Two glyph sets, switchable ");

        writeCenteredLine(4, "Characters 160-185 hold line-drawing");
        writeCenteredLine(5, "and block graphics by default:");

        moveCursor((cols - 26) / 2, 7);
        for (byte ch = 160; ch <= 185; ++ch)
            writeChar(ch);

        writeCenteredLine(10, use4x8Font
                                ? "setOriginalFont4x8(TRUE) swaps in the"
                                : "setOriginalFont5x8(TRUE) swaps in the");
        writeCenteredLine(11, "ISO-8859-1 originals:");

        if (use4x8Font)
            setOriginalFont4x8(TRUE);
        else
            setOriginalFont5x8(TRUE);

        moveCursor((cols - 26) / 2, 13);
        for (byte ch = 160; ch <= 185; ++ch)
            writeChar(ch);

        writeCenteredLine(16, "Both rows are on screen at once: the");
        writeCenteredLine(17, "font is read as each character is drawn.");

        // Put the line-drawing set back for the pages that follow.
        if (use4x8Font)
            setOriginalFont4x8(FALSE);
        else
            setOriginalFont5x8(FALSE);

        writeCenteredLine(20, "Press a key to continue: ");

        if (waitKeyBlinkingCursor() == BREAK)
            return FALSE;
    }

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
    byte col = (cols - 35) / 2;  // 3 in 42-col mode, 8 in 51-col mode
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

    // Demo setScreenInverted. Works in every mode: it flips the pixel
    // sense at 1 bpp and swaps the two colors at 4 bpp.
    //
    {
        clrscr();
        writeCenteredLine(2, " Screen inversion demo ");
        writeCenteredLine(5, "This is the default screen:");
        writeCenteredLine(6, "text in the foreground color.");
        moveCursor(0, 9);
        printf("setInverseVideoMode(TRUE) gives\n");
        setInverseVideoMode(TRUE);
        printf("inverted text");
        setInverseVideoMode(FALSE);
        printf(" within the normal screen.");
        writeCenteredLine(20, "Press a key to invert the screen: ");
        if (waitKeyBlinkingCursor() == BREAK)
            return FALSE;

        setScreenInverted(TRUE);
        clrscr();
        writeCenteredLine(2, " Screen inversion demo (inverted) ");
        writeCenteredLine(5, "After setScreenInverted(TRUE) the");
        writeCenteredLine(6, "screen background is now dark.");
        moveCursor(0, 9);
        printf("setInverseVideoMode(TRUE) now restores\n");
        setInverseVideoMode(TRUE);
        printf("the original look");
        setInverseVideoMode(FALSE);
        printf(" within an otherwise\ninverted screen.");
        moveCursor(0, 13);
        printf("Bold ");
        setBoldMode(TRUE);
        printf("characters");
        setBoldMode(FALSE);
        printf(" still work too.");

        line = 16;
        moveCursor(0, line);
        writeString("The cursor still blinks over text.");
        moveCursor(0, line + 1);
        writeString("Press a key to continue.");
        moveCursor(24, line);  // 'o' of "over"

        BOOL inversionContinue = (waitKeyBlinkingCursor() != BREAK);
        setScreenInverted(FALSE);
        if (!inversionContinue)
            return FALSE;
    }

    // Demo the line-drawing and block characters. Both fonts carry them,
    // so this page runs in any mode. Every dimension below is derived from
    // 'cols', so a wider screen fills with more content rather than more
    // margin.
    //
    {
        byte dividerCol = cols / 2;
        byte lpL = 1, lpW = dividerCol - 1;                      // left pane
        byte rpL = dividerCol + 1, rpW = cols - 2 - dividerCol;  // right pane
        byte c0, w, n, r;

        clrscr();
        writeCenteredLine(1, " Line-drawing patterns and block graphics ");
        line = 3;

        // Outer frame, split into two panes by a vertical divider that meets
        // the border with top and bottom T-junctions.
        moveCursor(0, line);
        writeChar(160);
        for (byte i = 1; i < cols - 1; ++i)
            writeChar((i == dividerCol) ? 166 : 169);
        writeChar(161);

        for (byte y = line + 1; y < line + 18; ++y)
        {
            moveCursor(0, y);
            writeChar(168);
            moveCursor(dividerCol, y);
            writeChar(168);
            moveCursor(cols - 1, y);
            writeChar(168);
        }

        moveCursor(0, line + 18);
        writeChar(162);
        for (byte i = 1; i < cols - 1; ++i)
            writeChar((i == dividerCol) ? 167 : 169);
        writeChar(163);

        // Left pane: a table of 3-column-wide cells, as many as fit, so the
        // junction characters get denser rather than the margins wider.
        moveCursor(centerCol(lpL, lpW, 16), line + 1);
        printf("T-junction grid:");

        byte tCells = (lpW - 1) / 4;        // each cell costs 3 + 1 divider
        if (tCells < 2)
            tCells = 2;
        byte tW = 1 + tCells * 4;
        c0 = centerCol(lpL, lpW, tW);

        moveCursor(c0, line + 2);
        writeChar(160);
        for (n = 0; n < tCells; ++n)
        {
            for (w = 0; w < 3; ++w)
                writeChar(169);
            writeChar((n < tCells - 1) ? 166 : 161);
        }

        for (r = 0; r < 3; ++r)
        {
            moveCursor(c0, line + 3 + r * 2);
            writeChar(168);
            for (n = 0; n < tCells; ++n)
            {
                for (w = 0; w < 3; ++w)
                    writeChar(' ');
                writeChar(168);
            }

            if (r < 2)
            {
                moveCursor(c0, line + 4 + r * 2);
                writeChar(165);
                for (n = 0; n < tCells; ++n)
                {
                    for (w = 0; w < 3; ++w)
                        writeChar(169);
                    writeChar((n < tCells - 1) ? 170 : 164);
                }
            }
        }

        moveCursor(c0, line + 8);
        writeChar(162);
        for (n = 0; n < tCells; ++n)
        {
            for (w = 0; w < 3; ++w)
                writeChar(169);
            writeChar((n < tCells - 1) ? 167 : 163);
        }

        // A thick box whose corners are rounded by the inverted quarter
        // blocks, stretched to the pane.
        moveCursor(centerCol(lpL, lpW, 12), line + 9);
        printf("Rounded box:");

        byte bW = (lpW > 8) ? (lpW - 4) : 5;
        c0 = centerCol(lpL, lpW, bW);

        moveCursor(c0, line + 10);
        writeChar(181);
        for (w = 0; w < bW - 2; ++w)
            writeChar(174);
        writeChar(182);

        for (byte y = line + 11; y < line + 14; ++y)
        {
            moveCursor(c0, y);
            writeChar(171);
            for (w = 0; w < bW - 2; ++w)
                writeChar(' ');
            writeChar(172);
        }

        moveCursor(c0, line + 14);
        writeChar(179);
        for (w = 0; w < bW - 2; ++w)
            writeChar(173);
        writeChar(180);

        // The plain quarter blocks used as what they are: corner dots,
        // repeated across the pane.
        moveCursor(centerCol(lpL, lpW, 12), line + 15);
        printf("Corner dots:");

        byte dPairs = (lpW > 8) ? ((lpW - 6) / 2) : 2;
        c0 = centerCol(lpL, lpW, dPairs * 2);

        moveCursor(c0, line + 16);
        for (n = 0; n < dPairs; ++n)
        {
            writeChar(177); writeChar(178);
        }
        moveCursor(c0, line + 17);
        for (n = 0; n < dPairs; ++n)
        {
            writeChar(175); writeChar(176);
        }

        // Right pane: a diagonal sweep from empty to solid. The ramp is
        // scaled to the block so it still reaches solid at the far corner
        // whatever the width.
        moveCursor(centerCol(rpL, rpW, 9), line + 1);
        printf("Gradient:");

        byte gW = (rpW > 4) ? (rpW - 2) : 4;
        byte gH = 7;
        byte maxLevel = (gH - 1) + (gW - 1);
        c0 = centerCol(rpL, rpW, gW);

        for (byte gy = 0; gy < gH; ++gy)
        {
            moveCursor(c0, line + 2 + gy);
            for (byte gx = 0; gx < gW; ++gx)
            {
                // Scale position along the diagonal onto the fill levels,
                // and onto the palette too where there is one to use.
                byte level = gy + gx;
                if (useCoCo3Screen)
                {
                    setForegroundColor(hueRamp[(word) level * 9 / (maxLevel + 1)]);
                    writeChar(solidRamp[(word) level * 4 / (maxLevel + 1)]);
                }
                else
                    writeChar(shadeRamp[(word) level * 5 / (maxLevel + 1)]);
            }
        }
        if (useCoCo3Screen)
            setForegroundColor(C_YELLOW);  // restore for the rest of the page

        // The two diagonals alternated so their edges line up cell to cell.
        moveCursor(centerCol(rpL, rpW, 15), line + 9);
        printf("Diagonal weave:");

        for (r = 0; r < 4; ++r)
        {
            moveCursor(c0, line + 10 + r);
            for (w = 0; w < gW; ++w)
                writeChar(((r + w) % 2 == 0) ? 184 : 185);
        }

        moveCursor(centerCol(rpL, rpW, 13), line + 14);
        printf("Checkerboard:");

        for (r = 0; r < 3; ++r)
        {
            moveCursor(c0, line + 15 + r);
            for (w = 0; w < gW; ++w)
                writeChar(((r + w) % 2 == 0) ? 183 : ' ');
        }

        writeCenteredLine(22, "Press a key to continue: ");

        if (waitKeyBlinkingCursor() == BREAK)
            return FALSE;
    }

    // Bold styles. Only the 320x192x16 mode has a choice to make: the
    // PMODE 4 writers always thicken, having no second color available.
    // Each style is shown normally and inverted, since inverting bold is
    // where the two differ most.
    //
    if (useCoCo3Screen)
    {
        clrscr();
        writeCenteredLine(1, " Two ways to render bold ");

        setTrueBold(TRUE);
        moveCursor(4, 4);
        printf("setTrueBold(TRUE), the default:");
        moveCursor(6, 6);
        printf("normal  ");
        setBoldMode(TRUE);
        printf("bold");
        setBoldMode(FALSE);
        printf("   inverted: ");
        setInverseVideoMode(TRUE);
        printf("normal  ");
        setBoldMode(TRUE);
        printf("bold");
        setBoldMode(FALSE);
        setInverseVideoMode(FALSE);
        moveCursor(6, 8);
        printf("thickened glyph, foreground color kept");

        setTrueBold(FALSE);
        moveCursor(4, 12);
        printf("setTrueBold(FALSE), as the original library:");
        moveCursor(6, 14);
        printf("normal  ");
        setBoldMode(TRUE);
        printf("bold");
        setBoldMode(FALSE);
        printf("   inverted: ");
        setInverseVideoMode(TRUE);
        printf("normal  ");
        setBoldMode(TRUE);
        printf("bold");
        setBoldMode(FALSE);
        setInverseVideoMode(FALSE);
        moveCursor(6, 16);
        printf("same weight, setForegroundBoldColor() color");

        setTrueBold(TRUE);
        writeCenteredLine(21, "Press a key to continue: ");

        if (waitKeyBlinkingCursor() == BREAK)
            return FALSE;
    }

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

    // Graphics page. Everything on this screen is in one graphics buffer:
    // the words go through the ordinary text calls, the picture is painted
    // into the very same memory a moment earlier.
    //
    {
        word bytesPerRow = gfxBytesPerRow();
        byte scale = 3;
        word artLeft = (gfxWidth - 64 * scale) / 2;
        byte artTop = 56;

        if (gfxBpp == 4)
        {
            // The color grid before this page leaves its own colors behind,
            // and a black background would swallow the black bar.
            setForegroundColor(C_YELLOW);
            setBackgroundColor(C_BLUE);
        }

        clrscr();
        writeCenteredLine(1, " Text and graphics share one buffer ");

        // A band across the top: every color at 4 bpp, and where there are
        // only two, an ordered dither ramping from paper to solid ink.
        if (gfxBpp == 4)
        {
            word barBytes = bytesPerRow / 16;
            for (byte b = 0; b < 16; ++b)
                gfxRect(b * barBytes * 2, 24, barBytes * 2, 24, b);
        }
        else
        {
            for (byte dy = 0; dy < 24; ++dy)
            {
                byte y = (byte) (24 + dy);
                byte *row = gfxBuffer + (word) y * bytesPerRow;
                for (word bx = 0; bx < bytesPerRow; ++bx)
                    row[bx] = ditherByte((byte) ((bx * 9) / bytesPerRow), y);
            }
        }

        // The machine itself.
        for (byte i = 0; i < 8; ++i)
            gfxRect(artLeft + (word) artX[i] * scale,
                    (byte) (artTop + artY[i] * scale),
                    (word) artW[i] * scale,
                    (byte) (artH[i] * scale),
                    (byte) (gfxBpp == 4 ? artHue[i] : artInk[i]));

        // ...showing an invader on its screen, because of course it is.
        for (byte r = 0; r < 8; ++r)
        {
            word bits = invaderRows[r];
            for (byte c = 0; c < 11; ++c)
                if (bits & ((word) 1 << (10 - c)))
                    gfxRect(artLeft + (word) (26 + c) * scale,
                            (byte) (artTop + (4 + r) * scale),
                            scale, scale,
                            (byte) (gfxBpp == 4 ? C_GREEN : 1));
        }

        writeCenteredLine(19, "These graphics are pixels, painted into");
        writeCenteredLine(20, "the buffer these words are written to.");

        writeCenteredLine(22, "Press a key to continue: ");

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

    gfxBuffer = init.init.textScreenBuffer;
    gfxWidth  = (useCoCo3Screen ? 320 : 256);
    gfxBpp    = (useCoCo3Screen ? 4 : 1);

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
