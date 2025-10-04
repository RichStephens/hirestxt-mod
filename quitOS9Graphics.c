/*  quitOS9Graphics.c

    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#include "hirestxt_private.h"


void
quitOS9Graphics(void)
{
    putchar(14);  // Display Alpha (text mode); does not free graphics buffer
    putchar(18);  // Quit Graphics (frees the 256x192 graphics buffer)
}
