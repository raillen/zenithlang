#include "runtime/c/zenith_rt.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#define ZT_MKDIR(path) _mkdir(path)
#else
#include <unistd.h>
#define ZT_MKDIR(path) mkdir((path), 0755)
#endif

static const char *runtime_test_argv0 = NULL;

static void assert_true(const char *name, int condition) {
    if (!condition) {
        fprintf(stderr, "falha no teste %s\n", name);
        exit(1);
    }
}

static int path_is_dir(const char *path) {
    struct stat info;

    if (path == NULL || stat(path, &info) != 0) {
        return 0;
    }

#ifdef _WIN32
    return (info.st_mode & S_IFDIR) != 0;
#else
    return S_ISDIR(info.st_mode);
#endif
}

static int path_exists(const char *path) {
    struct stat info;
    return path != NULL && stat(path, &info) == 0;
}

static void ensure_dir(const char *path) {
    char buffer[512];
    char *cursor;
    size_t length = strlen(path);

    if (length + 1 > sizeof(buffer)) {
        fprintf(stderr, "fixture grande demais: %s\n", path);
        exit(1);
    }

    memcpy(buffer, path, length + 1);
    for (cursor = buffer; *cursor != '\0'; cursor += 1) {
        if (*cursor == '/' || *cursor == '\\') {
            char saved = *cursor;
            if (cursor == buffer) continue;
            *cursor = '\0';
            if (buffer[0] != '\0' && !path_is_dir(buffer)) {
                if (ZT_MKDIR(buffer) != 0 && !path_is_dir(buffer)) {
                    fprintf(stderr, "falha ao criar diretório %s\n", buffer);
                    exit(1);
                }
            }
            *cursor = saved;
        }
    }

    if (!path_is_dir(buffer) && ZT_MKDIR(buffer) != 0 && !path_is_dir(buffer)) {
        fprintf(stderr, "falha ao criar diretório %s\n", buffer);
        exit(1);
    }
}

static char *read_text_file(const char *path) {
    FILE *file = fopen(path, "rb");
    char *buffer;
    long size;

    if (file == NULL) {
        fprintf(stderr, "falha ao abrir %s\n", path);
        exit(1);
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        fprintf(stderr, "falha ao medir %s\n", path);
        exit(1);
    }

    size = ftell(file);
    if (size < 0) {
        fclose(file);
        fprintf(stderr, "falha ao obter tamanho de %s\n", path);
        exit(1);
    }

    if (fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        fprintf(stderr, "falha ao reiniciar leitura de %s\n", path);
        exit(1);
    }

    buffer = (char *)malloc((size_t)size + 1);
    if (buffer == NULL) {
        fclose(file);
        fprintf(stderr, "falha ao alocar leitura de %s\n", path);
        exit(1);
    }

    if (size > 0 && fread(buffer, 1, (size_t)size, file) != (size_t)size) {
        free(buffer);
        fclose(file);
        fprintf(stderr, "falha ao ler %s\n", path);
        exit(1);
    }

    buffer[size] = '\0';
    fclose(file);
    return buffer;
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

    if (strcmp(argv[1], "--noop") == 0) {
        return 0;
    }

    if (strcmp(argv[1], "--exit-code") == 0) {
        if (argc < 3) {
            return 97;
        }
        return atoi(argv[2]);
    }

    if (strcmp(argv[1], "--write-cwd") == 0) {
        char cwd_buffer[4096];
        FILE *file;

        if (argc < 3) {
            return 96;
        }

#ifdef _WIN32
        if (_getcwd(cwd_buffer, (int)sizeof(cwd_buffer)) == NULL) {
            return 95;
        }
#else
        if (getcwd(cwd_buffer, sizeof(cwd_buffer)) == NULL) {
            return 95;
        }
#endif

        file = fopen(argv[2], "wb");
        if (file == NULL) {
            return 94;
        }

        fputs(cwd_buffer, file);
        fclose(file);
        return 0;
    }

    if (strcmp(argv[1], "--capture-echo") == 0) {
        fputs("alpha", stdout);
        fputs("beta", stderr);
        fflush(stdout);
        fflush(stderr);
        return 7;
    }

    if (strcmp(argv[1], "--overflow-text") == 0) {
        zt_text left = {0};
        zt_text right = {0};

        left.data = "";
        left.len = SIZE_MAX;
        right.data = "x";
        right.len = 1;
        zt_text_concat(&left, &right);
        return 93;
    }

    if (strcmp(argv[1], "--overflow-bytes") == 0) {
        zt_bytes left = {0};
        zt_bytes right = {0};
        static const uint8_t byte = 0x2A;

        left.data = (uint8_t *)&byte;
        left.len = SIZE_MAX;
        right.data = (uint8_t *)&byte;
        right.len = 1;
        zt_bytes_join(&left, &right);
        return 92;
    }

    return -1;
}

static void test_host_process_run_exit_code(void) {
    zt_text *program;
    zt_text *mode = zt_text_from_utf8_literal("--exit-code");
    zt_text *value = zt_text_from_utf8_literal("7");
    zt_text *items[] = { mode, value };
    zt_list_text *args = zt_list_text_from_array(items, 2);
    zt_outcome_i64_core_error outcome;

    zt_host_set_api(NULL);
    program = runtime_test_self_program();
    outcome = zt_host_process_run(program, args, zt_optional_text_empty());

    assert_true("process_run_exit_success", zt_outcome_i64_core_error_is_success(outcome));
    assert_true("process_run_exit_value", zt_outcome_i64_core_error_value(outcome) == 7);

    zt_outcome_i64_core_error_dispose(&outcome);
    zt_release(args);
    zt_release(value);
    zt_release(mode);
    zt_release(program);
}

static void test_host_process_run_blocks_shell_injection(void) {
    const char *marker_path = ".ztc-tmp/tests/runtime/process_injected.txt";
    zt_text *program;
    zt_text *mode = zt_text_from_utf8_literal("--noop");
#ifdef _WIN32
    zt_text *payload = zt_text_from_utf8_literal("& echo injected>.ztc-tmp/tests/runtime/process_injected.txt");
#else
    zt_text *payload = zt_text_from_utf8_literal("; touch .ztc-tmp/tests/runtime/process_injected.txt");
#endif
    zt_text *items[] = { mode, payload };
    zt_list_text *args = zt_list_text_from_array(items, 2);
    zt_outcome_i64_core_error outcome;

    ensure_dir(".ztc-tmp/tests/runtime");
    remove(marker_path);

    zt_host_set_api(NULL);
    program = runtime_test_self_program();
    outcome = zt_host_process_run(program, args, zt_optional_text_empty());

    assert_true("process_run_injection_success", zt_outcome_i64_core_error_is_success(outcome));
    assert_true("process_run_injection_exit", zt_outcome_i64_core_error_value(outcome) == 0);
    assert_true("process_run_injection_marker_absent", !path_exists(marker_path));

    zt_outcome_i64_core_error_dispose(&outcome);
    zt_release(args);
    zt_release(payload);
    zt_release(mode);
    zt_release(program);
}

static void test_host_process_run_honors_cwd(void) {
    char *actual_raw;
    zt_text *program;
    zt_text *expected_cwd;
    zt_text *output_path;
    zt_text *mode = zt_text_from_utf8_literal("--write-cwd");
    zt_text *args_items[2];
    zt_list_text *args;
    zt_outcome_i64_core_error outcome;
    zt_text *actual_text;
    zt_text *actual_normalized;
    zt_text *expected_normalized;

    ensure_dir(".ztc-tmp/tests/runtime/cwd-fixture");
    expected_cwd = runtime_test_absolute_path(".ztc-tmp/tests/runtime/cwd-fixture");
    output_path = runtime_test_absolute_path(".ztc-tmp/tests/runtime/process-cwd.txt");
    remove(zt_text_data(output_path));

    zt_host_set_api(NULL);
    program = runtime_test_self_program();
    args_items[0] = mode;
    args_items[1] = output_path;
    args = zt_list_text_from_array(args_items, 2);
    outcome = zt_host_process_run(program, args, zt_optional_text_present(expected_cwd));

    assert_true("process_run_cwd_success", zt_outcome_i64_core_error_is_success(outcome));
    assert_true("process_run_cwd_exit", zt_outcome_i64_core_error_value(outcome) == 0);

    actual_raw = read_text_file(zt_text_data(output_path));
    actual_text = zt_text_from_utf8_literal(actual_raw);
    actual_normalized = zt_path_normalize(actual_text);
    expected_normalized = zt_path_normalize(expected_cwd);

    assert_true("process_run_cwd_value", zt_text_eq(actual_normalized, expected_normalized));

    free(actual_raw);
    zt_release(expected_normalized);
    zt_release(actual_normalized);
    zt_release(actual_text);
    zt_outcome_i64_core_error_dispose(&outcome);
    zt_release(args);
    zt_release(output_path);
    zt_release(expected_cwd);
    zt_release(mode);
    zt_release(program);
}

static void test_host_process_run_capture_outputs(void) {
    zt_text *program;
    zt_text *mode = zt_text_from_utf8_literal("--capture-echo");
    zt_text *items[] = { mode };
    zt_list_text *args = zt_list_text_from_array(items, 1);
    zt_text *expected_stdout = zt_text_from_utf8_literal("alpha");
    zt_text *expected_stderr = zt_text_from_utf8_literal("beta");
    zt_outcome_process_captured_run_core_error outcome;

    zt_host_set_api(NULL);
    program = runtime_test_self_program();
    outcome = zt_host_process_run_capture(program, args, zt_optional_text_empty());

    assert_true("process_run_capture_success", outcome.is_success);
    assert_true("process_run_capture_exit", outcome.value.status.code == 7);
    assert_true(
        "process_run_capture_stdout",
        outcome.value.stdout_text != NULL && zt_text_eq(outcome.value.stdout_text, expected_stdout));
    assert_true(
        "process_run_capture_stderr",
        outcome.value.stderr_text != NULL && zt_text_eq(outcome.value.stderr_text, expected_stderr));

    zt_outcome_process_captured_run_core_error_dispose(&outcome);
    zt_release(expected_stderr);
    zt_release(expected_stdout);
    zt_release(args);
    zt_release(mode);
    zt_release(program);
}

static void test_text_concat_overflow_guard(void) {
    zt_text *program;
    zt_text *mode = zt_text_from_utf8_literal("--overflow-text");
    zt_text *items[] = { mode };
    zt_list_text *args = zt_list_text_from_array(items, 1);
    zt_outcome_i64_core_error outcome;

    zt_host_set_api(NULL);
    program = runtime_test_self_program();
    outcome = zt_host_process_run(program, args, zt_optional_text_empty());

    assert_true("text_concat_overflow_success", zt_outcome_i64_core_error_is_success(outcome));
    assert_true("text_concat_overflow_exit", zt_outcome_i64_core_error_value(outcome) == ZT_EXIT_CODE_RUNTIME_ERROR);

    zt_outcome_i64_core_error_dispose(&outcome);
    zt_release(args);
    zt_release(mode);
    zt_release(program);
}

static void test_bytes_join_overflow_guard(void) {
    zt_text *program;
    zt_text *mode = zt_text_from_utf8_literal("--overflow-bytes");
    zt_text *items[] = { mode };
    zt_list_text *args = zt_list_text_from_array(items, 1);
    zt_outcome_i64_core_error outcome;

    zt_host_set_api(NULL);
    program = runtime_test_self_program();
    outcome = zt_host_process_run(program, args, zt_optional_text_empty());

    assert_true("bytes_join_overflow_success", zt_outcome_i64_core_error_is_success(outcome));
    assert_true("bytes_join_overflow_exit", zt_outcome_i64_core_error_value(outcome) == ZT_EXIT_CODE_RUNTIME_ERROR);

    zt_outcome_i64_core_error_dispose(&outcome);
    zt_release(args);
    zt_release(mode);
    zt_release(program);
}

int main(int argc, char **argv) {
    int subprocess_mode;

    runtime_test_argv0 = (argc > 0) ? argv[0] : NULL;
    subprocess_mode = runtime_test_subprocess_mode(argc, argv);
    if (subprocess_mode >= 0) {
        return subprocess_mode;
    }

    test_host_process_run_exit_code();
    test_host_process_run_blocks_shell_injection();
    test_host_process_run_honors_cwd();
    test_host_process_run_capture_outputs();
    test_text_concat_overflow_guard();
    test_bytes_join_overflow_guard();

    puts("Runtime process tests OK");
    return 0;
}
