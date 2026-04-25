#include "compiler/driver/driver_internal.h"

zt_arena global_arena;
zt_string_pool global_pool;

int main(int argc, char *argv[]) {
    zt_driver_context ctx;
    int i;
    int status;
    char **filtered_argv;
    int filtered_argc = 0;

    zt_arena_init(&global_arena, 1048576);
    zt_string_pool_init(&global_pool, &global_arena);

    zt_driver_context_init(&ctx);

    filtered_argv = (char **)malloc(argc * sizeof(char *));
    if (filtered_argv == NULL) return 1;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--verbose") == 0 || strcmp(argv[i], "-v") == 0) {
            ctx.verbose = 1;
        } else if (strcmp(argv[i], "--quiet") == 0 || strcmp(argv[i], "-q") == 0) {
            ctx.quiet = 1;
        } else if ((strcmp(argv[i], "--project") == 0 || strcmp(argv[i], "-p") == 0) && i + 1 < argc) {
            ctx.project_path = argv[i + 1];
            i++;
        } else {
            filtered_argv[filtered_argc++] = argv[i];
        }
    }

    if (filtered_argc < 1) {
        char *help_argv[] = { "help" };
        status = zt_handle_zpm(&ctx, 1, help_argv);
    } else {
        status = zt_handle_zpm(&ctx, filtered_argc, filtered_argv);
    }

    free(filtered_argv);
    zt_driver_context_dispose(&ctx);
    zt_string_pool_dispose(&global_pool);
    zt_arena_dispose(&global_arena);

    return status;
}
