#include "runtime/c/zenith_rt.h"
#include <stdio.h>

static zt_int zt_main__main(void);

static zt_int zt_main__main(void) {
    goto zt_block_entry;

zt_block_entry:
    return 0;
}

int main(void) {
    return (int)zt_main__main();
}
