/*  os9timer.c

    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#include "hirestxt_private.h"


static const char *OS9Timer_deviceName = "/Nil";  // driver presumed installed in OS-9
static byte OS9Timer_nilPathNumber;
static word OS9Timer_counter;  // incremented by OS9Timer_signalHandler()


static asm interrupt void
OS9Timer_signalHandler(void)
{
    asm
    {
        leay    ,u      // make Y point to data segment, to access globals
        ldd     :OS9Timer_counter
        addd    #1
        std     :OS9Timer_counter
    }
}


byte
OS9Timer_init(unsigned ticksPerIncrement)
{
    byte errorCode;
    asm
    {
        clr     :errorCode
        ;
        // Tell OS-9 which function must be called when this process receives a signal.
        leax    OS9Timer_signalHandler
        exg     u,y                 // pass data segment address in U
        os9     $09                 // F$Icpt
        exg     u,y                 // restore both registers

        lda     #1                  // read mode
        ldx     :OS9Timer_deviceName
        os9     $84                 // I$Open
        bcs     @ErrExit
        sta     :OS9Timer_nilPathNumber
        bra     @nilOpen
@ErrExit
        stb     :errorCode
        bra     @done
@nilOpen
        pshs    y                   // preserve data segment register
        ldy     :ticksPerIncrement  // number of 60 Hz ticks
        ldb     #$C7                // set FS2+ VIRQ entry
        tfr     y,x
        os9     $8E                 // I$SetStt
        puls    y
        bcs     @ErrExit
@done
    }
    return errorCode;
}


byte
OS9Timer_shutdown(void)
{
    byte errorCode;
    asm
    {
        clr     :errorCode
        pshs    y                   // preserve data segment register
        lda     :OS9Timer_nilPathNumber
        ldb     #$81                // call code
        ldy     #0                  // clears entry
        os9     $8E                 // I$SetStt
        puls    y
        bcc     @entryCleared       // if success
        stb     :errorCode
        bra     @done
@entryCleared
        ; Close the path to /Nil.
        lda     :OS9Timer_nilPathNumber
        os9     $8F                 // I$Close
        bcc     @done               // if success
        stb     :errorCode
@done
    }
    return errorCode;
}


word *
OS9Timer_getTimerAddress(void)
{
    return &OS9Timer_counter;
}
