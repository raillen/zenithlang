#include "runtime/c/zenith_rt.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#define ZT_MKDIR(path) _mkdir(path)
#define ZT_SETENV(name, value) _putenv_s((name), (value))
#define ZT_UNSETENV(name) _putenv_s((name), "")
#else
#include <unistd.h>
#define ZT_MKDIR(path) mkdir((path), 0755)
#define ZT_SETENV(name, value) setenv((name), (value), 1)
#define ZT_UNSETENV(name) unsetenv((name))
#endif

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

static void ensure_dir(const char *path) {
    char buffer[512];
    char *cursor;
    size_t length = strlen(path);

    assert_true("ensure_dir_length", length + 1 <= sizeof(buffer));
    memcpy(buffer, path, length + 1);

    for (cursor = buffer; *cursor != '\0'; cursor += 1) {
        if (*cursor == '/' || *cursor == '\\') {
            char saved = *cursor;
            if (cursor == buffer) {
                continue;
            }
            *cursor = '\0';
            if (buffer[0] != '\0' && !path_is_dir(buffer)) {
                assert_true("ensure_dir_mkdir_partial", ZT_MKDIR(buffer) == 0 || path_is_dir(buffer));
            }
            *cursor = saved;
        }
    }

    if (!path_is_dir(buffer)) {
        assert_true("ensure_dir_mkdir_final", ZT_MKDIR(buffer) == 0 || path_is_dir(buffer));
    }
}

static void write_bytes(const char *path, const unsigned char *bytes, size_t length) {
    FILE *file = fopen(path, "wb");
    assert_true("write_bytes_open", file != NULL);
    if (length > 0) {
        assert_true("write_bytes_write", fwrite(bytes, 1, length, file) == length);
    }
    assert_true("write_bytes_close", fclose(file) == 0);
}

static void test_invalid_utf8_is_rejected(void) {
    const char *path = ".ztc-tmp/tests/runtime/c/fs_guardrails/invalid_utf8.txt";
    const unsigned char bytes[] = { 'A', 0x80u, 'B' };
    zt_text *path_text;
    zt_outcome_text_core_error outcome;

    ensure_dir(".ztc-tmp/tests/runtime/c/fs_guardrails");
    write_bytes(path, bytes, sizeof(bytes));

    path_text = zt_text_from_utf8_literal(path);
    outcome = zt_host_read_file(path_text);

    assert_true("invalid_utf8_failure", !outcome.is_success);
    assert_true("invalid_utf8_code", strcmp(zt_text_data(outcome.error.code), "fs.io") == 0);
    assert_true(
        "invalid_utf8_message",
        strstr(zt_text_data(outcome.error.message), "valid UTF-8") != NULL);

    zt_core_error_dispose(&outcome.error);
    zt_release(path_text);
}

static void test_root_policy_blocks_escape(void) {
    const char *root = ".ztc-tmp/tests/runtime/c/fs_guardrails/root";
    const char *inside_path = ".ztc-tmp/tests/runtime/c/fs_guardrails/root/inside.txt";
    const char *escaped_path = ".ztc-tmp/tests/runtime/c/fs_guardrails/root/../outside.txt";
    zt_text *inside_text;
    zt_text *escaped_text;
    zt_text *content;
    zt_outcome_void_core_error inside_outcome;
    zt_outcome_void_core_error escaped_outcome;

    ensure_dir(root);
    assert_true("set_root_env", ZT_SETENV("ZENITH_HOST_FS_ROOT", root) == 0);

    inside_text = zt_text_from_utf8_literal(inside_path);
    escaped_text = zt_text_from_utf8_literal(escaped_path);
    content = zt_text_from_utf8_literal("ok");

    inside_outcome = zt_host_write_file(inside_text, content);
    assert_true("root_inside_success", inside_outcome.is_success);

    escaped_outcome = zt_host_write_file(escaped_text, content);
    assert_true("root_escape_failure", !escaped_outcome.is_success);
    assert_true(
        "root_escape_code",
        strcmp(zt_text_data(escaped_outcome.error.code), "fs.permission_denied") == 0);

    zt_core_error_dispose(&escaped_outcome.error);
    zt_release(content);
    zt_release(escaped_text);
    zt_release(inside_text);
    assert_true("unset_root_env", ZT_UNSETENV("ZENITH_HOST_FS_ROOT") == 0);
}

static void test_nul_path_is_rejected(void) {
    const char raw_path[] = { '.', 't', 'm', 'p', '\0', 'x' };
    zt_text *bad_path = zt_text_from_utf8(raw_path, sizeof(raw_path));
    zt_text *content = zt_text_from_utf8_literal("x");
    zt_outcome_void_core_error outcome = zt_host_write_file(bad_path, content);

    assert_true("nul_path_failure", !outcome.is_success);
    assert_true("nul_path_code", strcmp(zt_text_data(outcome.error.code), "fs.invalid_path") == 0);

    zt_core_error_dispose(&outcome.error);
    zt_release(content);
    zt_release(bad_path);
}

int main(void) {
    test_invalid_utf8_is_rejected();
    test_root_policy_blocks_escape();
    test_nul_path_is_rejected();
    printf("Runtime host fs guardrail tests OK\n");
    return 0;
}
