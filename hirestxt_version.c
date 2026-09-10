/*  hirestxt_version.c

    VERSION comes from this library's Makefile via -D, which a calling
    program cannot see, so this call is the only way to ask. It therefore
    reports the library that was actually linked, not what an accompanying
    hirestxt.h happens to claim.

    In its own file so a program that never asks does not link the string.

    This file is in the public domain.
*/

#include "hirestxt.h"


const char *hirestxt_version(void)
{
    return VERSION;
}
