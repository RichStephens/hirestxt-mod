/*  writeCharAt_320x16.c

    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#include "hirestxt_private.h"

#include "font4x8.h"


#define USE_ASM


static byte nybbleTable[4] = { 0x00, 0x0F, 0xF0, 0xFF };


// tayste: 2 pixels: 00, 01, 10 or 11
// Returns a byte that contains 2 4-bit pixels.
//
#ifdef USE_ASM
asm __gcccall
#endif
byte convertTaysteTo4BitPixels(byte tayste, byte fgColorMask)
{
    #ifndef USE_ASM
    byte octet = nybbleTable[tayste];  // 2 4-bit pixels: $00, $0F, $F0 or $FF
    byte byteToWrite =
              ( octet & hiResTextConfig.bgColorMask)
                    // $F nybble(s) become selected 4-bit background color
            | (~octet & fgColorMask);
                    // $0 nybble(s) become selected 4-bit foreground color
    return byteToWrite;
    #else
    asm
    {
        leax    :nybbleTable
        ldb     b,x             // B contains tayste as per __gcccall, becomes octet
        tfr     b,a
        andb    :hiResTextConfig.bgColorMask
        coma                    // ~octet
        anda    3,s             // fgColorMask: low byte of 1st arg, stacked as 16 bits at 2,s
        pshs    a
        orb     ,s+
    }
    #endif
}


// Uses hiResTextConfig, inverseVideoMode and boldMode.
// Assumes that hiResTextConfig.numPixelsPerRow == 320 and
// hiResTextConfig.numBitsPerPixel == 4.
// Uses font4x8[]. Assumes 5-bit-wide glyphs.
//
void writeCharAt_320x16(byte x, byte y, byte asciiCode)
{
    // Row of bytes in 320x16:
    //
    // Bytes:           0000000011111111222222223333333344444444... (5 bytes shown)
    // 4-bit pixels:    ====----====----====----====----====----... (10 pixels shown)
    // 51-column chars: ********************++++++++++++++++++++... (2 chars shown)
    //
    // When x is an even text column (0, 2, ..., 50), screenByte will
    // point to a 3-byte region whose first 20 bits will get overwritten
    // with the glyph represented by asciiCode.
    // The other 4 bits will remain unmodified.
    //
    // When x is an odd text column (1, 3, ..., 49), screenByte will
    // point to a 3-byte region whose LAST 20 bits will get overwritten.
    // The FIRST 4 bits will remain unmodified.
    //
    byte *screenByte;
    #ifndef USE_ASM
    screenByte = hiResTextConfig.textScreenBuffer
                            + y * (320 * 4 / 8 * PIXEL_ROWS_PER_TEXT_ROW)
                            + (word) x * (5 * 4) / 8;
    #else
    asm
    {
        // (word) x * (5 * 4) / 8
        ldb     :x
        lda     #5*4
        mul
        lsra
        rorb
        lsra
        rorb
        lsra
        rorb

        ldx     :hiResTextConfig.textScreenBuffer
        leax    d,x     // add (word) x * (5 * 4) / 8

        // y * 1280 == y * 5 * 256
        lda     :y
        ldb     #5
        mul             // D = y * 5 <= 120, so fits in B, A is 0, so do D <<= 8
        tfr     b,a
        clrb            // D = y * 5 * 256 = y * 1280
        
        leax    d,x     // add y * 1280
        stx     :screenByte
    }
    #endif

    // In charBitmask, only the high 5 bits of each byte are significant.
    // The others must be 0.
    //
    const byte *charBitmask;
    #ifndef USE_ASM
    charBitmask = font4x8 + (((word) asciiCode - (asciiCode < 128 ? 32 : 64)) << 3);
    #else
    asm
    {
        ldb     :asciiCode
        bpl     @below128
        subb    #32
@below128
        subb    #32
        clra
        lslb
        rola
        lslb
        rola
        lslb
        rola
        leax    :font4x8
        leax    d,x
        stx     :charBitmask
    }
    #endif

    const byte fgColorMask = (boldMode ? hiResTextConfig.fgBoldColorMask : hiResTextConfig.fgColorMask);

    for (byte row = 0; row < PIXEL_ROWS_PER_TEXT_ROW; ++row)
    {
        if (asciiCode != 0)
        {
            byte charBitmaskByte = charBitmask[row];
            byte xorArg = (inverseVideoMode ? fgColorMask ^ hiResTextConfig.bgColorMask : 0);
                                  // N.B.: Does not support inverting bold color.

            #ifndef USE_ASM
            #define SHIFT2 charBitmaskByte >>= 2
            #define SHIFT3 charBitmaskByte >>= 3
            #else
            #define SHIFT2 asm { lsr :charBitmaskByte } asm { lsr :charBitmaskByte }
            #define SHIFT3 SHIFT2 asm { lsr :charBitmaskByte }
            #endif
            if ((x & 1) == 0)  // if x is even
            {
                SHIFT2;
                byte pixelPair = convertTaysteTo4BitPixels(charBitmaskByte & 0b11, fgColorMask);
                SHIFT2;
                screenByte[1]  = convertTaysteTo4BitPixels(charBitmaskByte & 0b11, fgColorMask) ^ xorArg;
                SHIFT2;
                screenByte[0]  = convertTaysteTo4BitPixels(charBitmaskByte       , fgColorMask) ^ xorArg;
                screenByte[2]  = (screenByte[2] & 0x0F)
                                 | ((pixelPair ^ xorArg) & 0xF0);  // change high nybble only
                                    // note parentheses due to & having priority over ^
            }
            else  // x is odd:
            {
                SHIFT3;
                screenByte[2] = convertTaysteTo4BitPixels(charBitmaskByte & 0b11, fgColorMask) ^ xorArg;
                SHIFT2;
                screenByte[1] = convertTaysteTo4BitPixels(charBitmaskByte & 0b11, fgColorMask) ^ xorArg;
                SHIFT2;
                byte lowNybble = (convertTaysteTo4BitPixels(charBitmaskByte, fgColorMask) ^ xorArg) & 0x0F;  // use left-most bit of glyph
                screenByte[0] = (screenByte[0] & 0xF0) | lowNybble;  // only change low nybble of screen byte
            }
        }
        else  // inverting colors instead of writing a character:
        {
            byte maskCombo = fgColorMask ^ hiResTextConfig.bgColorMask;  // N.B.: Does not support inverting bold color.
            if ((x & 1) == 0)  // if x is even
            {
                screenByte[0] ^= maskCombo;
                screenByte[1] ^= maskCombo;
                screenByte[2] = (screenByte[2] & 0x0F)
                                | ((screenByte[2] ^ maskCombo) & 0xF0);  // change high nybble only
            }
            else  // x is odd:
            {
                screenByte[0] = (screenByte[0] & 0xF0)
                                | ((screenByte[0] ^ maskCombo) & 0x0F);  // change low nybble only
                screenByte[1] ^= maskCombo;
                screenByte[2] ^= maskCombo;
            }
        }

        screenByte += 320 * 4 / 8;
    }
}
