/*  font4x8.h - ISO-8859-1 font for a 51x24 software text screen.

    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#ifndef _font4x8_h_
#define _font4x8_h_


// Characters 32 to 127 and 160 to 255.
// Only the 5 high bits of each byte are part of the glyph.
// A reset bit is ink; a set bit is paper.
// Bit 3 is the inter-character gap, and is set except in the
// line-drawing glyphs, whose horizontal strokes span it so that
// lines join across cell boundaries.
// The 3 low bits of each byte are zero.
//
// Characters 160-185 hold line-drawing and block glyphs by default.
// Not const: useOriginalFont4x8() overwrites them with the original
// ISO-8859-1 glyphs.
//
extern unsigned char font4x8[1536];


#endif  /* _font4x8_h_ */
