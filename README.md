hirestxt
========

## Modifications to the Original Library

This is a modified version of the original hirestxt library by Pierre Sarrazin.
The original library can be found at:
http://gvlsywt.cluster051.hosting.ovh.net/dev/hirestxt-0.5.1.tar.gz

The following changes have been made relative to the original:

**Font: line-drawing characters in both fonts**
Characters 160–185 are replaced with line-drawing glyphs (box corners,
T-junctions, horizontal/vertical lines, cross, and block graphics) in both the
42-column (5x8) and the 51-column (4x8) font. These are the default. Call
`setOriginalFont5x8(TRUE)` or `setOriginalFont4x8(TRUE)` for the original
ISO-8859-1 glyphs, and `FALSE` to go back. The 4x8 font is also the one used by
the CoCo 3's 64-column 320x192x16 mode.

Either set can be selected at any time, as often as you like, independently of
init. A single 208-byte table holds whichever set is not currently live and the
two are exchanged in place, so switching costs no more memory than a one-way
replacement would.

Because the font is consulted only as each character is written, text already
on the screen keeps the glyphs it was drawn with. Writing, switching, and
writing again therefore puts both sets on screen simultaneously — which is what
the demo's second page does.

Neither font is linked into a program that does not draw with it, and the
replacement tables are linked only into a program that calls for them, so an
application pays only for the font and the option it actually uses.

The line-drawing glyphs are designed to connect seamlessly: horizontal lines
span the full cell width (no gap at the right edge), and T-junction and cross
characters align their horizontal bar at the same pixel row as the standalone
horizontal line character so that lines are continuous across character
boundaries. The 4x8 cell is 5 pixels wide — an odd number — so its vertical
strokes and crosses sit on an exact center column, which the 6-pixel 5x8 cell
has no room for. The 4x8 half-blocks likewise split evenly, so the thick left
and right verticals tile into a solid with no seam down the middle.

**New function: `clearRowsN()`**
`clearRowsN(byte byteToClearWith, byte textRow, byte rowsToClear)` clears a
specified number of text rows starting at a given row. Significantly faster
than clearing the full screen when only a portion needs to be erased. It
derives its row stride from the current mode, so it works in the PMODE 4
modes and in the CoCo 3's 320x192x16 mode alike.
See `clearRowsN.c` and the declaration in `hirestxt.h`.

Two convenience wrappers come with it. `clearn(byte n)` fills the first `n`
rows with spaces without moving the cursor, the row-limited counterpart of
`clear()`; `clrscrn(byte n)` homes the cursor first, as `clrscr()` does. Both
pick the right fill byte for the current mode, and honour `setScreenInverted()`
the same way `clear()` does.

**New function: `setScreenInverted()`**
`setScreenInverted(BOOL invert)` globally inverts the text screen, and works
in every mode. In the PMODE 4 modes (51x24 and 42x24) the pixel sense is
flipped: `clear()` fills with black instead of the foreground color, and
characters render in the foreground color on a black background. In the CoCo
3's 320x192x16 mode there is no pixel sense to flip, so the foreground and
background colors trade places instead: `clear()` fills with the foreground
color, glyphs render background-on-foreground, and a scrolled-in row matches.

`setInverseVideoMode()` XOR-combines with this flag in both cases, so it
restores the original (un-inverted) look within an otherwise inverted screen.
The cursor blink (XOR-based) continues to work normally over inverted text.
The demo's screen-inversion page exercises both flags together, in all modes.

**New function: `hirestxt_version()`**
`hirestxt_version()` returns this library's version as a string, e.g.
`"0.5.0.4"`. The value is compiled into the library from `VERSION` in the
Makefile, so it describes the `libhirestxt.a` that was actually linked rather
than whatever `hirestxt.h` happens to sit beside it. A calling program cannot
see the Makefile's `-DVERSION` define, so this call is the only way to ask.
Costs nothing unless called.

**New function: `setTrueBold()`**
`setTrueBold(BOOL trueBold)` chooses how `setBoldMode(TRUE)` renders in the
CoCo 3's 320x192x16 mode.

`TRUE`, the default, is *true bold*: the glyph is thickened by smearing each
ink pixel one place to the right, the same technique the PMODE 4 writers use,
and the caller's foreground color is kept. `FALSE` restores the original
library's behavior, which drew bold at normal weight in the separate color set
by `setForegroundBoldColor()`.

The flag has no effect in the PMODE 4 modes, which always thicken because they
have no second color available. It is reset to `TRUE` by
`initHiResTextScreen()` and `initHiResTextScreen2()`.

Inverted bold under `FALSE` keeps the bold color on the glyph and takes the
same field color as inverted normal text, so bold reads as bold either way
round. The original library built its inversion mask from the bold color
instead, which recolored the field and left no cue that the text was bold —
the limitation its `writeCharAt_320x16.c` warned about.

Colour being the only cue on that path, the bold color must differ from
**both** the foreground and the background. Matching the foreground makes bold
identical to normal text un-inverted and invisible inverted; matching the
background does the same the other way round. Nothing enforces this, and
`setForegroundColor()` changes only the foreground, so a program that switches
its foreground to the color already used for bold silently loses the
distinction. True bold has none of this, marking bold by weight.

Thickening has one consequence: the 4x8 cell's last pixel doubles as the
inter-character gap, so a bold stroke reaching the fourth pixel touches the
next character. The PMODE 4 modes behave the same way. The demo's bold page
renders the same sentence both ways for comparison.

**Fixed: the font swap wrote to the wrong glyphs**
The font array runs 32–127 then 160–255, with no entries for 128–159, so
character 160 sits at entry 96. The original swap code addressed it as entry
128 (`(160 - 32) * 8`) and so overwrote characters 192–217 — the accented
capitals À–Ù — while leaving 160–185 untouched. Selecting the original glyphs
therefore never worked and quietly corrupted other characters instead. Present
in releases 0.5.0.1 through 0.5.0.4; harmless to any program that left the
option alone, which was the default.

**New function: `setHiResTextBuffer()`**
`setHiResTextBuffer(byte *newTextScreenBuffer)` redirects all subsequent text
output to the given graphics buffer without changing which buffer is displayed.
This enables double-buffering: draw a frame on a hidden buffer, then make it
visible (e.g. with CMOC's `setPmodeGraphicsAddress()`), and alternate. The new
buffer must satisfy the same constraints as the `textScreenBuffer` field of
`HiResTextScreenInit`. The cursor position is global and is not affected by
switching buffers. See `setHiResTextBuffer.c` and the declaration in
`hirestxt.h`.

It is a plain pointer change and so works in any mode, but it only redirects
*writes*. On the CoCo 3 you must also point the GIME at the other buffer by
writing its address to `$FF9D`, and at 30,720 bytes per 320x192x16 buffer two
of them nearly fill the 64K logical address space, so in practice the second
buffer lives in another physical block and the MMU does the switching — in
which case remapping alone redirects the writes and this call is not needed.

---

This library is in the public domain.

It implements a software 51x24 or 42x24 black-on-green PMODE 4 text
screen (256x192x2), or a 64x24 16-color text screen in the CoCo 3's
320x192x16 graphics mode. It redirects printf() to that screen.

Useful to get true lowercase, including Latin-1 accented characters,
on all CoCos, on the Dragon, and on NitrOS-9.

Optionally supports several VT52 terminal sequences.

A sample program is included. See the *Demo* section in this file.

This library is intended to be compiled by CMOC.

See the [CMOC home page](http://sarrazip.com/dev/cmoc.html).


## Setup

The instructions apply when targetting the Color Basic environment.

Do the following to initialize the 51x24 text screen at $0E00 on a CoCo
or Dragon.

Put this directive somewhere at the beginning of the C file in question:

    #include "hirestxt.h"

Then set up PMODE 4 green/black graphics, like this:

    width(32);  /* PMODE graphics will only appear from 32x16 (does nothing on CoCo 1&2) */
    pmode(4, (byte *) 0x0E00);
    pcls(255);
    screen(1, 0);  /* green/black */

Then define a `HiResTextScreenInit` object:

    struct HiResTextScreenInit init =
        {
            51,  /* characters per row */
            writeCharAt_51cols,  /* must be consistent with previous field */
            0x0E00,
            TRUE  /* redirects printf() to the 51x24 text screen */
            (word *) 0x112,  /* pointer to a 60 Hz async counter (Color Basic's TIMER) */
            0,  /* default cursor blinking rate */
            NULL,  /* use inkey(), i.e., Color Basic's INKEY$ */
            NULL,  /* no sound on '\a' */
        };

Then call `initHiResTextScreen()` with the address of this object:

    initHiResTextScreen(&init);

To use the 42x24 mode, replace 51 and `writeCharAt_51cols` with 42 and
`writeCharAt_42cols`.

Optionally, to get the screen address from Basic instead of assuming
$0E00, use this expression in the `HiResTextScreenInit` struct initializer:

    (byte *) * (byte *) 0x00BC << 8

Then, see hirestxt.h for the available functions.

To compile with this library, pass `-I <dir>` to cmoc, where `<dir>` is
the directory where hirestxt.h is.

To link with this library, specify `-L <dir> -lhirestxt` to the cmoc
invocation that does the linking. This specifies where the libhirestxt.a
file can be found.


## Demo

The demo is compiled as `hirestxt.bin` for the CoCo and Dragon Basic
environments, and `hirestxt` for OS-9.

Under OS-9, the demo must be executed from a 32x16 terminal.


## Dragon support

To compile for the Dragon, pass `TARGET=dragon` to make.


## OS-9 support

To compile for OS-9, pass `TARGET=os9` to make.

This generates a demo in an OS-9 executable named `hirestxt`. Use the
[ToolShed](https://sourceforge.net/projects/toolshed/) `os9` command
to transfer this executable to an OS-9 virtual hard-disk (for example)
and to give it the execution attributes:

    os9 copy -r hirestxt the-disk.vhd,/CMDS/
    os9 attr -epe the-disk.vhd,/CMDS/hirestxt

Under OS-9, the demo must be executed from a 32x16 terminal.

This has been tested with [NitrOS-9](http://www.nitros9.org/battle.html)
EOU 1.0.1 and [CMOC](http://sarrazip.com/dev/cmoc.html) 0.1.89.
The /nil driver must be present.


## VT52 Support

A subset of [VT52](https://en.wikipedia.org/wiki/VT52) terminal sequences
are supported by the code in function processConsoleOutChar() of
processConsoleOutChar.c.

If this support is not needed, the code for it can be omitted by compiling
the library with HIRESTEXT_NO_VT52.


## License

This library is in the public domain.


## Version history

    0.1.0 - 2016-05-01 - First public release.
    0.1.1 - 2016-09-12 - Adapted to CMOC 0.1.31 re: inline asm.
    0.1.2 - 2016-12-26 - HIRESTEXT_NO_VT52 to avoid VT52 code.
    0.2.0 - 2017-12-01 - Adapted to modular compilation under CMOC 0.1.43.
    0.2.1 - 2018-04-03 - Adapted to modular compilation under CMOC 0.1.51.
    0.3.0 - 2018-09-20 - Added 42x24 text screen and 5x8 font.
                         Adapted to CMOC 0.1.53.
    0.3.1 - 2018-11-25 - Fixed linking error re: putBitmaskInScreenWord() in 42x24 mode.
                         Now compilable for the Dragon (make TARGET=dragon).
    0.4.0 - 2023-08-19 - initHiResTextScreen() and closeHiResTextScreen() do not change the
                         screen mode themselves anymore -- see the next section.
                         The HiResTextScreenInit struct must now receive a pointer
                         to a timer word.
                         It may also receive a pointer to an inkey-like function.
                         Replaced textScreenPageNum in struct HiResTextScreenInit with
                         the textScreenBuffer field, which removes a dependency on the SAM chip --
                         see the next section.
                         Stopped exporting the global textScreenBuffer variable.
                         Removed the setTextScreenAddress() function.
                         Fixed typo in the name of the internal oldCHROUT global variable.
                         Merged all library code into a single .a file.
                         Most functions are now in their own file, to optimize the final executable size.
                         Most global variables are now fields of a single global struct instance.
                         Now usable under OS-9 (assumes that the /nil driver is installed).
                         Slightly optimized character writing routine.
    0.4.1 - 2023-09-02 - Added setInverseVideoMode() and setBoldMode() which allow writing
                         characters in inverted colors and/or in bold.
                         Added support for VT52 sequences <ESC> p and <ESC> q to turn inverse
                         video on of off, and <ESC> E to home the cursor and clear the screen.
    0.4.2 - 2024-11-28 - Added showOS9PMode4Screen(), quitOS9Graphics(), OS9Timer_init(),
                         OS9Timer_shutdown() and OS9Timer_getTimerAddress() under OS-9.
                         Fixed a bug in the demo where the column of letters was displayed
                         too slowly.
    0.5.0 - 2025-03-14 - Added support for writing to a 64x24 text screen on a CoCo 3 320x192x16
                         graphics screen. See initHiResTextScreen2(), setForegroundColor() et al.
                         hirestxt-demo.c gives an example.
                         The screen is now cleared from top to bottom instead of bottom up.
                         The demo now plays an audible bell character.


## Using version 0.5.x

Version 0.5.0 introduced support for a 64x24 text mode in the CoCo 3 320x192x16 graphics mode.
This text mode is slower than using the CoCo 3's hardware 40- or 80-column text screens, but it
allows:

* using the same hirestxt-based code to display text on a CoCo 1, 2 or 3, on a Dragon, or
on NitrOS-9;
* mixing text and graphics on the same screen, using a narrower font than the byte-aligned
  font used by Basic's HPRINT command.

The hirestxt-demo.c program shows examples of how to use both the CoCo 3 64x24 mode and the
original 51x24 and 42x24 modes on a 256x192x2 (PMODE 4) screen.


## Adapting to version 0.4.x

The instructions apply when targetting the Color Basic environment.

-   Where the `HiResTextScreenInit` is initialized, add these values:

    -   A value for the `timer` field, which must be the address of a 16-bit word that
        gets incremented 60 times per second.

    -   A zero for the `cursorAnimationLowTimerByteMask` field.

    -   A NULL value for the `inkeyFuncPtr` field, to have the library use `inkey()`,
        as before, to poll the keyboard.

    -   A NULL value for the `bellFuncPtr` field, to have a call to Color Basic's
        SOUND 1,1, as before, when character 7 is sent to the screen.

-   Execute this code before calling `initHiResTextScreen()`:

        initCoCoSupport();
        width(32);
        pmode(4, A);
        pcls(255);
        screen(1, 0);

    Replace `A` with the address put in the `textScreenBuffer` field of struct
    `HiResTextScreenInit`.

    To use the current address where ECB would draw PMODE 4 graphics, use this
    expression: `(byte *) * (byte *) 0x00BC << 8`

    (This takes $BC, sees it as a byte pointer, gets that byte, sees it as a
    16-bit address, then shifts this address 8 bits left, to form the actual
    address of the screen buffer.)

    Under other platforms, execute platform-specific code to set up a 256x192
    green/black 2-color graphics mode, then put its buffer address in
    `textScreenBuffer`.

-   Execute this code after calling `closeHiResTextScreen()`:

        pmode(0, 0);
        cls(255);
        screen(0, 0);

-   If the program was using the `textScreenBuffer` global variable, it
    should now stop using it and use the address that it puts in the
    `textScreenBuffer` field of `HiResTextScreenInit` instead.
    
-   Similarly, the `hiResWidth` global variable has been replaced with
    the `numColumns` field of `HiResTextScreenInit`.

-   The `textPosX` and `textPosY` global variables have been removed from
    the public interface. Functions `getCursorColumn()` and `getCursorRow()`
    have been added. The cursor can be moved with `moveCursor()`.

