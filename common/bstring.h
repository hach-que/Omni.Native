#ifndef __OMNI_BSTRING_H
#define __OMNI_BSTRING_H

#include <bstrlib.h>
#include <bstraux.h>

// We use freed_bstring in our argument definitions so that
// it is clear when a method will free one of it's arguments
// and that if the callee wishes to keep it, it needs to copy
// the string before passing it in.
typedef struct
{
    bstring ref;
    short dummy; // Prevents automatic casting.
} freed_bstring;

freed_bstring bautofree(bstring str);
void bautodestroy(freed_bstring f);
freed_bstring bautocpy(const_bstring b1);

int bescape(bstring str);

#endif
