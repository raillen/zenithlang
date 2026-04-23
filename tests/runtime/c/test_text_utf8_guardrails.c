#include "runtime/c/zenith_rt.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *runtime_test_argv0 = NULL;

static void assert_true(const char *name, int condition) {
    if (!condition) {
        fprintf(stderr, "falha no teste %s\n", name);
        exit(1);
    }
}

static zt_text *runtime_test_absolute_path(const char *path) {
    zt_outcome_text_core_error cwd_outcome = zt_host_os_current_dir();
    zt_text *base;
    zt_text *relative;
    zt_text *absolute;

    assert_true("absolute_path_cwd_success", cwd_outcome.is_success);
    base = zt_outcome_text_core_error_value(cwd_outcome);
    relative = zt_text_from_utf8_literal(path);
    absolute = zt_path_absolute(relative, base);

    zt_release(relative);
    zt_release(base);
    zt_outcome_text_core_error_dispose(&cwd_outcome);
    return absolute;
}

static zt_text *runtime_test_self_program(void) {
    assert_true("self_program_available", runtime_test_argv0 != NULL && runtime_test_argv0[0] != '\0');
    return runtime_test_absolute_path(runtime_test_argv0);
}

static int runtime_test_subprocess_mode(int argc, char **argv) {
    if (argc >= 2 && strcmp(argv[1], "--invalid-from-utf8") == 0) {
        const char raw[] = { 'A', (char)0x80, 'B' };
        zt_text *value = zt_text_from_utf8(raw, sizeof(raw));
        zt_release(value);
        return 91;
    }
    return -1;
}

static void test_text_from_utf8_accepts_valid_input(void) {
    const char raw[] = { 'A', (char)0xC3, (char)0xA9 };
    zt_text *value = zt_text_from_utf8(raw, sizeof(raw));

    assert_true("valid_text_len", zt_text_len(value) == 2);
    assert_true("valid_text_data", strcmp(zt_text_data(value), "A\xc3\xa9") == 0);
    zt_release(value);
}

static void test_text_from_utf8_rejects_invalid_input(void) {
    zt_text *program;
    zt_text *mode = zt_text_from_utf8_literal("--invalid-from-utf8");
    zt_text *items[] = { mode };
    zt_list_text *args = zt_list_text_from_array(items, 1);
    zt_outcome_i64_core_error outcome;

    zt_host_set_api(NULL);
    program = runtime_test_self_program();
    outcome = zt_host_process_run(program, args, zt_optional_text_empty());

    assert_true("invalid_utf8_subprocess_success", zt_outcome_i64_core_error_is_success(outcome));
    assert_true("invalid_utf8_subprocess_nonzero", zt_outcome_i64_core_error_value(outcome) != 0);

    zt_outcome_i64_core_error_dispose(&outcome);
    zt_release(args);
    zt_release(mode);
    zt_release(program);
}

int main(int argc, char **argv) {
    int subprocess_mode = runtime_test_subprocess_mode(argc, argv);
    if (subprocess_mode >= 0) {
        return subprocess_mode;
    }

    runtime_test_argv0 = argc > 0 ? argv[0] : NULL;
    test_text_from_utf8_accepts_valid_input();
    test_text_from_utf8_rejects_invalid_input();
    printf("Runtime text UTF-8 guardrail tests OK\n");
    return 0;
}
