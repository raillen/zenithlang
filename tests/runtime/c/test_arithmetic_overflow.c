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
    if (argc < 2) {
        return -1;
    }

    if (strcmp(argv[1], "--overflow-add") == 0) {
        (void)zt_add_i64(INT64_MAX, 1);
        return 91;
    }

    if (strcmp(argv[1], "--overflow-sub") == 0) {
        (void)zt_sub_i64(INT64_MIN, 1);
        return 92;
    }

    if (strcmp(argv[1], "--overflow-mul") == 0) {
        (void)zt_mul_i64(INT64_MAX, 2);
        return 93;
    }

    return -1;
}

static void test_add_sub_mul_success(void) {
    assert_true("add_i64_success", zt_add_i64(40, 2) == 42);
    assert_true("sub_i64_success", zt_sub_i64(44, 2) == 42);
    assert_true("mul_i64_success", zt_mul_i64(6, 7) == 42);
}

static void assert_overflow_subprocess(const char *name, const char *mode) {
    zt_text *program;
    zt_text *mode_text = zt_text_from_utf8_literal(mode);
    zt_text *items[] = { mode_text };
    zt_list_text *args = zt_list_text_from_array(items, 1);
    zt_outcome_i64_core_error outcome;
    zt_int exit_code;

    zt_host_set_api(NULL);
    program = runtime_test_self_program();
    outcome = zt_host_process_run(program, args, zt_optional_text_empty());

    assert_true(name, zt_outcome_i64_core_error_is_success(outcome));
    exit_code = zt_outcome_i64_core_error_value(outcome);
    assert_true("overflow_subprocess_nonzero", exit_code != 0);
    assert_true("overflow_subprocess_not_sentinel", exit_code != 91 && exit_code != 92 && exit_code != 93);

    zt_outcome_i64_core_error_dispose(&outcome);
    zt_release(args);
    zt_release(mode_text);
    zt_release(program);
}

int main(int argc, char **argv) {
    int subprocess_mode;

    runtime_test_argv0 = (argc > 0) ? argv[0] : NULL;
    subprocess_mode = runtime_test_subprocess_mode(argc, argv);
    if (subprocess_mode >= 0) {
        return subprocess_mode;
    }

    test_add_sub_mul_success();
    assert_overflow_subprocess("overflow_add_subprocess", "--overflow-add");
    assert_overflow_subprocess("overflow_sub_subprocess", "--overflow-sub");
    assert_overflow_subprocess("overflow_mul_subprocess", "--overflow-mul");
    printf("Runtime arithmetic overflow tests OK\n");
    return 0;
}
