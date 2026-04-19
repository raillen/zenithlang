#include "runtime/c/zenith_rt.h"
#include <stdio.h>

static zt_int zt_main__main(void);

static zt_int zt_main__main(void) {
    zt_text *t0 = NULL;
    zt_int t1;
    zt_int zt_return_value;
    goto zt_block_entry;

zt_block_entry:
    if (t0 != NULL) { zt_release(t0); t0 = NULL; }
    t0 = zt_text_from_utf8_literal("hello");
    t1 = puts(zt_text_data(t0));
    zt_return_value = t1;
    goto zt_cleanup;

zt_cleanup:
    if (t0 != NULL) { zt_release(t0); t0 = NULL; }
    return zt_return_value;
}

int main(void) {
    return (int)zt_main__main();
}
