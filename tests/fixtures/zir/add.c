#include "runtime/c/zenith_rt.h"
#include <stdio.h>

static zt_int zt_main__add(zt_int a, zt_int b);

static zt_int zt_main__add(zt_int a, zt_int b) {
    zt_int t0;
    goto zt_block_entry;

zt_block_entry:
    t0 = (a + b);
    return t0;
}
