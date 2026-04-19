#include "compiler/frontend/parser/parser.h"
#include "compiler/project/zdoc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#define ZT_MKDIR(path) _mkdir(path)
#else
#include <sys/wait.h>
#define ZT_MKDIR(path) mkdir((path), 0755)
#endif

static int tests_run = 0;
static int tests_passed = 0;

#define ASSERT_TRUE(condition, msg) do { \
    tests_run++; \
    if (condition) { tests_passed++; } \
    else { fprintf(stderr, "FAIL: %s\n", msg); } \
} while (0)

static int path_is_dir(const char *path) {
    struct stat info;
    if (path == NULL || stat(path, &info) != 0) return 0;
#ifdef _WIN32
    return (info.st_mode & S_IFDIR) != 0;
#else
    return S_ISDIR(info.st_mode);
#endif
}

static int ensure_dir(const char *path) {
    char buffer[512];
    char *cursor;
    size_t length = strlen(path);

    if (length + 1 > sizeof(buffer)) return 0;
    memcpy(buffer, path, length + 1);

    for (cursor = buffer; *cursor != '\0'; cursor += 1) {
        if (*cursor == '/' || *cursor == '\\') {
            char saved = *cursor;
            if (cursor == buffer) continue;
            *cursor = '\0';
            if (buffer[0] != '\0' && !path_is_dir(buffer)) {
                if (ZT_MKDIR(buffer) != 0 && !path_is_dir(buffer)) {
                    *cursor = saved;
                    return 0;
                }
            }
            *cursor = saved;
        }
    }

    if (!path_is_dir(buffer)) {
        if (ZT_MKDIR(buffer) != 0 && !path_is_dir(buffer)) return 0;
    }

    return 1;
}

static int write_text_file(const char *path, const char *text) {
    FILE *file = fopen(path, "wb");
    size_t length;

    if (file == NULL) return 0;

    length = text != NULL ? strlen(text) : 0;
    if (length > 0 && fwrite(text, 1, length, file) != length) {
        fclose(file);
        return 0;
    }

    fclose(file);
    return 1;
}

static int list_has_code(
        const zt_zdoc_diagnostic_list *list,
        zt_zdoc_code code,
        zt_zdoc_severity severity) {
    size_t i;

    if (list == NULL) return 0;
    for (i = 0; i < list->count; i += 1) {
        if (list->items[i].code == code && list->items[i].severity == severity) return 1;
    }
    return 0;
}

static zt_parser_result parse_source_or_fail(const char *path, const char *source_text) {
    zt_parser_result parsed = zt_parse(&test_arena, &test_pool, ath, source_text, strlen(source_text));
    if (parsed.diagnostics.count != 0) {
        size_t i;
        fprintf(stderr, "FAIL: parser errors while preparing zdoc test fixture (%s)\n", path);
        for (i = 0; i < parsed.diagnostics.count; i += 1) {
            fprintf(stderr, "  parse: %s\n", parsed.diagnostics.items[i].message);
        }
        exit(1);
    }
    return parsed;
}

static void test_zdoc_ok(void) {
    const char *base = ".ztc-tmp/tests/zdoc/ok";
    const char *source_path = ".ztc-tmp/tests/zdoc/ok/src/app/main.zt";
    const char *source_text =
        "namespace app\n"
        "\n"
        "public func main() -> int\n"
        "    return 0\n"
        "end\n";
    const char *api_doc =
        "--- @target: namespace\n"
        "Main module docs.\n"
        "---\n"
        "\n"
        "--- @target: main\n"
        "Entrypoint docs with @link main.\n"
        "---\n";
    const char *guide_doc =
        "--- @page: getting-started\n"
        "Guide text linking @link main.\n"
        "---\n";
    zt_parser_result parsed;
    zt_zdoc_source_unit unit;
    zt_zdoc_diagnostic_list diagnostics;

    ASSERT_TRUE(ensure_dir(".ztc-tmp/tests/zdoc/ok/src/app"), "zdoc ok fixture source dir");
    ASSERT_TRUE(ensure_dir(".ztc-tmp/tests/zdoc/ok/zdoc/app"), "zdoc ok fixture api dir");
    ASSERT_TRUE(ensure_dir(".ztc-tmp/tests/zdoc/ok/zdoc/guides"), "zdoc ok fixture guides dir");

    ASSERT_TRUE(write_text_file(source_path, source_text), "zdoc ok source file write");
    ASSERT_TRUE(write_text_file(".ztc-tmp/tests/zdoc/ok/zdoc/app/main.zdoc", api_doc), "zdoc ok api file write");
    ASSERT_TRUE(write_text_file(".ztc-tmp/tests/zdoc/ok/zdoc/guides/getting-started.zdoc", guide_doc), "zdoc ok guide file write");

    parsed = parse_source_or_fail(source_path, source_text);
    unit.source_path = source_path;
    unit.root = parsed.root;

    zt_zdoc_diagnostic_list_init(&diagnostics);
    ASSERT_TRUE(zt_zdoc_check_project(base, "src", "zdoc", &unit, 1, &diagnostics), "zdoc ok check call");
    ASSERT_TRUE(zt_zdoc_diagnostic_error_count(&diagnostics) == 0, "zdoc ok has no errors");
    ASSERT_TRUE(zt_zdoc_diagnostic_warning_count(&diagnostics) == 0, "zdoc ok has no warnings");

    zt_zdoc_diagnostic_list_dispose(&diagnostics);
    zt_parser_result_dispose(&parsed);
}

static void test_zdoc_missing_target(void) {
    const char *base = ".ztc-tmp/tests/zdoc/missing_target";
    const char *source_path = ".ztc-tmp/tests/zdoc/missing_target/src/app/main.zt";
    const char *source_text =
        "namespace app\n"
        "\n"
        "public func main() -> int\n"
        "    return 0\n"
        "end\n";
    const char *api_doc =
        "---\n"
        "Missing target block.\n"
        "---\n";
    zt_parser_result parsed;
    zt_zdoc_source_unit unit;
    zt_zdoc_diagnostic_list diagnostics;

    ASSERT_TRUE(ensure_dir(".ztc-tmp/tests/zdoc/missing_target/src/app"), "zdoc missing target source dir");
    ASSERT_TRUE(ensure_dir(".ztc-tmp/tests/zdoc/missing_target/zdoc/app"), "zdoc missing target api dir");

    ASSERT_TRUE(write_text_file(source_path, source_text), "zdoc missing target source write");
    ASSERT_TRUE(write_text_file(".ztc-tmp/tests/zdoc/missing_target/zdoc/app/main.zdoc", api_doc), "zdoc missing target api write");

    parsed = parse_source_or_fail(source_path, source_text);
    unit.source_path = source_path;
    unit.root = parsed.root;

    zt_zdoc_diagnostic_list_init(&diagnostics);
    ASSERT_TRUE(zt_zdoc_check_project(base, "src", "zdoc", &unit, 1, &diagnostics), "zdoc missing target check call");
    ASSERT_TRUE(list_has_code(&diagnostics, ZT_ZDOC_MISSING_TARGET, ZT_ZDOC_ERROR), "zdoc missing target diagnostic");

    zt_zdoc_diagnostic_list_dispose(&diagnostics);
    zt_parser_result_dispose(&parsed);
}

static void test_zdoc_missing_page_in_guide(void) {
    const char *base = ".ztc-tmp/tests/zdoc/missing_page";
    const char *source_path = ".ztc-tmp/tests/zdoc/missing_page/src/app/main.zt";
    const char *source_text =
        "namespace app\n"
        "\n"
        "public func main() -> int\n"
        "    return 0\n"
        "end\n";
    const char *api_doc =
        "--- @target: main\n"
        "Entrypoint docs.\n"
        "---\n";
    const char *guide_doc =
        "---\n"
        "Guide without @page.\n"
        "---\n";
    zt_parser_result parsed;
    zt_zdoc_source_unit unit;
    zt_zdoc_diagnostic_list diagnostics;

    ASSERT_TRUE(ensure_dir(".ztc-tmp/tests/zdoc/missing_page/src/app"), "zdoc missing page source dir");
    ASSERT_TRUE(ensure_dir(".ztc-tmp/tests/zdoc/missing_page/zdoc/app"), "zdoc missing page api dir");
    ASSERT_TRUE(ensure_dir(".ztc-tmp/tests/zdoc/missing_page/zdoc/guides"), "zdoc missing page guides dir");

    ASSERT_TRUE(write_text_file(source_path, source_text), "zdoc missing page source write");
    ASSERT_TRUE(write_text_file(".ztc-tmp/tests/zdoc/missing_page/zdoc/app/main.zdoc", api_doc), "zdoc missing page api write");
    ASSERT_TRUE(write_text_file(".ztc-tmp/tests/zdoc/missing_page/zdoc/guides/guide.zdoc", guide_doc), "zdoc missing page guide write");

    parsed = parse_source_or_fail(source_path, source_text);
    unit.source_path = source_path;
    unit.root = parsed.root;

    zt_zdoc_diagnostic_list_init(&diagnostics);
    ASSERT_TRUE(zt_zdoc_check_project(base, "src", "zdoc", &unit, 1, &diagnostics), "zdoc missing page check call");
    ASSERT_TRUE(list_has_code(&diagnostics, ZT_ZDOC_MISSING_PAGE, ZT_ZDOC_ERROR), "zdoc missing page diagnostic");

    zt_zdoc_diagnostic_list_dispose(&diagnostics);
    zt_parser_result_dispose(&parsed);
}

static void test_zdoc_unresolved_target_and_link(void) {
    const char *base = ".ztc-tmp/tests/zdoc/unresolved";
    const char *source_path = ".ztc-tmp/tests/zdoc/unresolved/src/app/main.zt";
    const char *source_text =
        "namespace app\n"
        "\n"
        "public func main() -> int\n"
        "    return 0\n"
        "end\n";
    const char *api_doc =
        "--- @target: missing_symbol\n"
        "Unknown target with link @link also_missing.\n"
        "---\n";
    zt_parser_result parsed;
    zt_zdoc_source_unit unit;
    zt_zdoc_diagnostic_list diagnostics;

    ASSERT_TRUE(ensure_dir(".ztc-tmp/tests/zdoc/unresolved/src/app"), "zdoc unresolved source dir");
    ASSERT_TRUE(ensure_dir(".ztc-tmp/tests/zdoc/unresolved/zdoc/app"), "zdoc unresolved api dir");

    ASSERT_TRUE(write_text_file(source_path, source_text), "zdoc unresolved source write");
    ASSERT_TRUE(write_text_file(".ztc-tmp/tests/zdoc/unresolved/zdoc/app/main.zdoc", api_doc), "zdoc unresolved api write");

    parsed = parse_source_or_fail(source_path, source_text);
    unit.source_path = source_path;
    unit.root = parsed.root;

    zt_zdoc_diagnostic_list_init(&diagnostics);
    ASSERT_TRUE(zt_zdoc_check_project(base, "src", "zdoc", &unit, 1, &diagnostics), "zdoc unresolved check call");
    ASSERT_TRUE(list_has_code(&diagnostics, ZT_ZDOC_UNRESOLVED_TARGET, ZT_ZDOC_ERROR), "zdoc unresolved target diagnostic");
    ASSERT_TRUE(list_has_code(&diagnostics, ZT_ZDOC_UNRESOLVED_LINK, ZT_ZDOC_WARNING), "zdoc unresolved link warning");

    zt_zdoc_diagnostic_list_dispose(&diagnostics);
    zt_parser_result_dispose(&parsed);
}


static void test_zdoc_missing_public_doc_warning(void) {
    const char *base = ".ztc-tmp/tests/zdoc/missing_public_doc";
    const char *source_path = ".ztc-tmp/tests/zdoc/missing_public_doc/src/app/main.zt";
    const char *source_text =
        "namespace app\n"
        "\n"
        "public func main() -> int\n"
        "    return 0\n"
        "end\n"
        "\n"
        "public func helper() -> int\n"
        "    return 1\n"
        "end\n";
    const char *api_doc =
        "--- @target: main\n"
        "Main docs only.\n"
        "---\n";
    zt_parser_result parsed;
    zt_zdoc_source_unit unit;
    zt_zdoc_diagnostic_list diagnostics;

    ASSERT_TRUE(ensure_dir(".ztc-tmp/tests/zdoc/missing_public_doc/src/app"), "zdoc missing public source dir");
    ASSERT_TRUE(ensure_dir(".ztc-tmp/tests/zdoc/missing_public_doc/zdoc/app"), "zdoc missing public api dir");

    ASSERT_TRUE(write_text_file(source_path, source_text), "zdoc missing public source write");
    ASSERT_TRUE(write_text_file(".ztc-tmp/tests/zdoc/missing_public_doc/zdoc/app/main.zdoc", api_doc), "zdoc missing public api write");

    parsed = parse_source_or_fail(source_path, source_text);
    unit.source_path = source_path;
    unit.root = parsed.root;

    zt_zdoc_diagnostic_list_init(&diagnostics);
    ASSERT_TRUE(zt_zdoc_check_project(base, "src", "zdoc", &unit, 1, &diagnostics), "zdoc missing public check call");
    ASSERT_TRUE(zt_zdoc_diagnostic_error_count(&diagnostics) == 0, "zdoc missing public has no errors");
    ASSERT_TRUE(list_has_code(&diagnostics, ZT_ZDOC_MISSING_PUBLIC_DOC, ZT_ZDOC_WARNING), "zdoc missing public warning");

    zt_zdoc_diagnostic_list_dispose(&diagnostics);
    zt_parser_result_dispose(&parsed);
}

static void test_zdoc_private_docs_supported(void) {
    const char *base = ".ztc-tmp/tests/zdoc/private_docs";
    const char *source_path = ".ztc-tmp/tests/zdoc/private_docs/src/app/main.zt";
    const char *source_text =
        "namespace app\n"
        "\n"
        "public func main() -> int\n"
        "    return normalize_name(1)\n"
        "end\n"
        "\n"
        "func normalize_name(x: int) -> int\n"
        "    return x\n"
        "end\n";
    const char *api_doc =
        "--- @target: main\n"
        "Main docs.\n"
        "---\n"
        "\n"
        "--- @target: normalize_name\n"
        "Private helper docs.\n"
        "---\n";
    zt_parser_result parsed;
    zt_zdoc_source_unit unit;
    zt_zdoc_diagnostic_list diagnostics;

    ASSERT_TRUE(ensure_dir(".ztc-tmp/tests/zdoc/private_docs/src/app"), "zdoc private source dir");
    ASSERT_TRUE(ensure_dir(".ztc-tmp/tests/zdoc/private_docs/zdoc/app"), "zdoc private api dir");

    ASSERT_TRUE(write_text_file(source_path, source_text), "zdoc private source write");
    ASSERT_TRUE(write_text_file(".ztc-tmp/tests/zdoc/private_docs/zdoc/app/main.zdoc", api_doc), "zdoc private api write");

    parsed = parse_source_or_fail(source_path, source_text);
    unit.source_path = source_path;
    unit.root = parsed.root;

    zt_zdoc_diagnostic_list_init(&diagnostics);
    ASSERT_TRUE(zt_zdoc_check_project(base, "src", "zdoc", &unit, 1, &diagnostics), "zdoc private check call");
    ASSERT_TRUE(zt_zdoc_diagnostic_error_count(&diagnostics) == 0, "zdoc private has no errors");
    ASSERT_TRUE(zt_zdoc_diagnostic_warning_count(&diagnostics) == 0, "zdoc private has no warnings");

    zt_zdoc_diagnostic_list_dispose(&diagnostics);
    zt_parser_result_dispose(&parsed);
}


int main(void) {
    test_zdoc_ok();
    test_zdoc_missing_target();
    test_zdoc_missing_page_in_guide();
    test_zdoc_unresolved_target_and_link();
    test_zdoc_missing_public_doc_warning();
    test_zdoc_private_docs_supported();

    printf("ZDoc tests: %d/%d passed\n", tests_passed, tests_run);
    return tests_passed == tests_run ? 0 : 1;
}
