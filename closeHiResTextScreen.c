/*  closeHiResTextScreen.c

    By Pierre Sarrazin <http://sarrazip.com/>
    This file is in the public domain.
*/

#include "hirestxt_private.h"


void closeHiResTextScreen(void)
{
    if (hiResTextConfig.oldCHROUT)
        setConsoleOutHook(hiResTextConfig.oldCHROUT);
}
