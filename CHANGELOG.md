# Changelog

Notes for each release. The section whose heading matches a tag becomes that
release's description on GitHub, with the generated commit list appended below
it, so writing a release note is just editing this file before tagging.

## Unreleased

### Line-drawing and block glyphs in both fonts

Characters 160-185 carry line-drawing and block graphics — box corners,
T-junctions, horizontal and vertical lines, a cross, half blocks, quarter
blocks and diagonals. Previously these existed only in the 5x8 font, which
serves the 42-column mode. They are now in the 4x8 font as well, so they are
available in the 51-column PMODE 4 mode and in the CoCo 3's 64-column
320x192x16 mode.

The 4x8 cell is 5 pixels wide. Being an odd number, it has a true center
column, so vertical strokes and crosses sit dead center — the 6-pixel 5x8 cell
has no exact middle and has always been a pixel off. The 4x8 half blocks also
split evenly, so the thick left and right verticals tile into a solid with no
seam, which the 5x8 pair cannot manage.

### Either glyph set, at any time

Selecting a glyph set is now a setter that works in both directions:

    setOriginalFont4x8(TRUE);    /* ISO-8859-1 accented characters */
    setOriginalFont4x8(FALSE);   /* line-drawing and block glyphs  */

`setOriginalFont5x8()` does the same for the 42-column font. This replaces the
`useOriginal5x8Font` flag, which was read once during init and could not be
undone.

The font is consulted as each character is written, so text already on the
screen keeps the glyphs it was drawn with. Writing, switching, and writing
again puts both sets on screen at once — the demo's second page does exactly
that.

Internally one 208-byte table holds whichever set is not currently live and
the two are exchanged in place, so switching in both directions costs no more
memory than the old one-way replacement did.

### Fixed: selecting the original glyphs never worked

The font array runs 32-127 and then 160-255, with no entries for 128-159, so
character 160 sits at entry 96. The swap addressed it as entry 128. It
therefore overwrote characters 192-217 — the accented capitals À through Ù —
with the glyphs meant for 160-185, and never touched 160-185 at all.

Setting `useOriginal5x8Font` thus produced no font change and quietly
corrupted other characters instead. Present in 0.5.0.1 through 0.5.0.4.
Programs that left the flag alone, which was the default, were unaffected.

### Programs no longer carry fonts they never draw with

The font swap used to live in `init.c`, which named `font5x8` unconditionally.
Because every program links `init.o`, every program also linked the entire
42-column font whether or not it used it — a 51-column program was carrying
about 1.7K of glyphs it never drew a character with.

Each font now follows only its own renderer. Compared with 0.5.0.4, a
51-column program is roughly 1.7K smaller and a 42-column one about 260 bytes
smaller, with no source changes. The glyph tables are likewise linked only
into programs that call for them.

### Screen inversion in the CoCo 3 mode

`setScreenInverted()` previously affected only the PMODE 4 modes, where it
flips the pixel sense. There is no pixel sense to flip at 4 bits per pixel, so
in the 320x192x16 mode it now trades the foreground and background colors:
`clear()` fills with the foreground color, glyphs render background-on-
foreground, and a row scrolled in at the bottom matches. `setInverseVideoMode()`
combines with it as before, so it restores the un-inverted look within an
otherwise inverted screen.

### Bold that actually looks bold

    setTrueBold(TRUE);    /* thicker glyph, foreground color kept — default */
    setTrueBold(FALSE);   /* normal weight in setForegroundBoldColor()'s color */

Bold in the 320x192x16 mode used to mean nothing but a second color, so bold
text was the same weight as normal text. It now thickens the glyph by smearing
each ink pixel one place to the right — the same technique the PMODE 4 modes
have always used — and keeps whatever foreground color you set.

Inverted bold works properly either way. Previously the inversion mask was
built from the bold color, which merely recolored the field behind the text
and left no cue that it was bold; the bold color is now applied to the glyph,
so inverted bold sits on the same field as inverted normal text and differs by
lettering color.

On the `FALSE` path, color is the only cue, so the bold color must differ from
both the foreground and the background. Matching either one makes bold
indistinguishable from normal text in one direction and invisible in the
other. Nothing enforces this; all three colors are yours to choose. True bold
has no such constraint.

### Asking which library you linked against

    printf("hirestxt %s\n", hirestxt_version());

The version is compiled into the library from the Makefile, so this reports
the `libhirestxt.a` actually linked rather than whatever `hirestxt.h` happens
to sit beside it. There was previously no way for a program to ask: the
version existed only as a `-D` on the library's own build. Costs nothing
unless called.

### Demo

New pages showing the two glyph sets side by side, line and block patterns,
the two bold styles both normally and inverted, and text sharing a buffer with
graphics drawn straight into it. Page layouts are derived from the column
count rather than written per mode, so all three modes lay themselves out and
a new width would too.

## 0.5.0.4

### Double buffering

    setHiResTextBuffer(hiddenBuffer);   /* draw out of sight from here on */

Redirects all subsequent text output to another graphics buffer without
changing which buffer is displayed. Draw a frame into the hidden buffer, make
it visible — with CMOC's `setPmodeGraphicsAddress()` in PMODE 4 — and
alternate, so the screen never shows a partly drawn frame.

The new buffer must meet the same requirements as the `textScreenBuffer` field
passed to `initHiResTextScreen()`. The cursor position is global and is not
affected by switching buffers.

## 0.5.0.3

### Whole-screen inversion

    setScreenInverted(TRUE);

Inverts the entire PMODE 4 text screen: `clear()` fills dark instead of light
and characters render light on dark, rather than inverting a run of text at a
time.

`setInverseVideoMode()` combines with this flag rather than overriding it, so
within an otherwise inverted screen it restores the original look — useful for
highlighting a selection on an inverted screen. The blinking cursor, which
works by inverting pixels, continues to behave correctly over inverted text.

## 0.5.0.2

### Line-drawing and block glyphs in the 42-column font

Characters 160-185 of the 5x8 font now hold box corners, T-junctions,
horizontal and vertical lines, a cross, and block graphics, in place of the
ISO-8859-1 characters that occupied that range. Frames, tables and menus can
be drawn with ordinary text calls, with no graphics code and no cost beyond
the characters themselves.

The glyphs are drawn to connect: horizontal lines span the full character cell
rather than stopping short of the inter-character gap, and the junction and
cross characters put their horizontal bar on the same pixel row as the plain
horizontal line. Runs of line characters therefore join across cell boundaries
instead of showing gaps at every character.

A `useOriginal5x8Font` flag was meant to select the original ISO-8859-1 glyphs
instead. It did not work — see the release that fixed it.

### Release workflow

Releases now build lwtools and CMOC from source rather than depending on
prebuilt toolchains, and the artifact is named with a hyphen before the
version.

## 0.5.0.1

### Clearing part of the screen

    clearRowsN(byteToClearWith, textRow, rowsToClear);

Clears a given number of text rows starting at a given row. Clearing the whole
screen means writing every byte of a 6K or 30K buffer, which is slow enough to
see on a 0.89 MHz 6809; this touches only the rows that need erasing, which
matters for anything that repaints a status line, a menu or a message area on
every pass.

## 0.5.0

First release of this modified version of Pierre Sarrazin's hirestxt.

The library implements a software text screen on a graphics screen: 51x24 or
42x24 on a 256x192 PMODE 4 screen, or 64x24 in the CoCo 3's 320x192x16 mode,
and redirects `printf()` to it. It gives true lowercase and Latin-1 accented
characters on any CoCo, and optionally understands a number of VT52 terminal
sequences.

A binary release contains `libhirestxt.a` and `hirestxt.h`; build from source
with CMOC and lwtools if you would rather.
