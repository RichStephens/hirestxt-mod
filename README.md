hirestxt
========

NOTE: This library is a modification of the original hirestxt
library by by Pierre Sarrazin <http://sarrazip.com/>

The only real difference is that the 42x24 font has been modified
so that the chracters 160-185 have been changed to line-drawing
characters, such as lines and box corners.

This library is in the public domain.

It implements a software 51x24 or 42x24 black-on-green PMODE 4 text
screen (256x192x2), or a 64x24 16-color text screen in the CoCo 3's
320x200x16 graphics mode. It redirects printf() to that screen.

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

