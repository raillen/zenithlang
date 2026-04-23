#include "runtime/c/zenith_rt.h"
#include <stdio.h>

static zt_int zt_app_main__main(void);

static zt_int zt_app_main__main(void) {
    zt_int a;
    zt_int b;
    goto zt_block_entry;

zt_block_entry:
    a = 40;
    b = 2;
    return zt_add_i64(a, b);
}

int main(int argc, char **argv) {
    zt_runtime_capture_process_args(argc, argv);
    return (int)zt_app_main__main();
}
