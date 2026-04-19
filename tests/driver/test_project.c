#include "compiler/frontend/parser/parser.h"
#include "compiler/hir/lowering/from_ast.h"
#include "compiler/project/ztproj.h"
#include "compiler/semantic/binder/binder.h"
#include "compiler/semantic/types/checker.h"
#include "compiler/targets/c/emitter.h"
#include "compiler/zir/lowering/from_hir.h"
#include "compiler/zir/verifier.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int tests_run = 0;
static int tests_passed = 0;

#define ASSERT_TRUE(condition, msg) do { \
    tests_run++; \
    if (condition) { tests_passed++; } \
    else { fprintf(stderr, "FAIL: %s\n", msg); } \
} while (0)

#define ASSERT_STR_EQ(actual, expected, msg) do { \
    tests_run++; \
    if ((actual) != NULL && strcmp((actual), (expected)) == 0) { tests_passed++; } \
    else { fprintf(stderr, "FAIL: %s: expected %s, got %s\n", msg, (expected), (actual) != NULL ? (actual) : "<null>"); } \
} while (0)

static char *read_file_or_fail(const char *path) {
    FILE *file;
    long file_size;
    char *buffer;

    file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "FAIL: cannot open %s\n", path);
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    buffer = (char *)malloc((size_t)file_size + 1);
    if (buffer == NULL) {
        fclose(file);
        fprintf(stderr, "FAIL: out of memory reading %s\n", path);
        exit(1);
    }

    if (file_size > 0 && fread(buffer, 1, (size_t)file_size, file) != (size_t)file_size) {
        fclose(file);
        free(buffer);
        fprintf(stderr, "FAIL: cannot read %s\n", path);
        exit(1);
    }

    buffer[file_size] = '\0';
    fclose(file);
    return buffer;
}

static int assert_no_parser_errors(const char *name, const zt_parser_result *result) {
    size_t i;
    tests_run++;
    if (result->diagnostics.count == 0) {
        tests_passed++;
        return 1;
    }

    fprintf(stderr, "FAIL: %s parser errors=%zu\n", name, result->diagnostics.count);
    for (i = 0; i < result->diagnostics.count; i += 1) {
        fprintf(stderr, "  parse: %s\n", result->diagnostics.items[i].message);
    }
    return 0;
}

static int assert_no_diagnostics(const char *name, const zt_diag_list *diagnostics) {
    size_t i;
    tests_run++;
    if (diagnostics->count == 0) {
        tests_passed++;
        return 1;
    }

    fprintf(stderr, "FAIL: %s diagnostics=%zu\n", name, diagnostics->count);
    for (i = 0; i < diagnostics->count; i += 1) {
        fprintf(stderr, "  diag: %s\n", diagnostics->items[i].message);
    }
    return 0;
}

static void test_manifest_parse(void) {
    const char *manifest =
        "[project]\n"
        "name = \"demo-app\"\n"
        "kind = \"app\"\n"
        "version = \"0.1.0\"\n"
        "\n"
        "[source]\n"
        "root = \"src\"\n"
        "\n"
        "[app]\n"
        "entry = \"app.main\"\n"
        "\n"
        "[build]\n"
        "target = \"native\"\n"
        "output = \"build\"\n"
        "profile = \"release\"\n"
        "monomorphization_limit = 2048\n"
        "\n"
        "[test]\n"
        "root = \"spec\"\n"
        "\n"
        "[zdoc]\n"
        "root = \"docs\"\n"
        "\n"
        "[dependencies]\n"
        "json = { git = \"https://github.com/zenith-lang/json.git\", tag = \"v1.2.0\" }\n"
        "core = \"1.0.0\"\n"
        "\n"
        "[dev_dependencies]\n"
        "test_helpers = { path = \"../local_libs/test_helpers\" }\n";
    zt_project_parse_result result;

    ASSERT_TRUE(zt_project_parse_text(manifest, strlen(manifest), &result), "manifest parse succeeds");
    ASSERT_STR_EQ(result.manifest.project_name, "demo-app", "manifest project name");
    ASSERT_STR_EQ(result.manifest.project_kind, "app", "manifest project kind");
    ASSERT_STR_EQ(result.manifest.version, "0.1.0", "manifest version");
    ASSERT_STR_EQ(result.manifest.source_root, "src", "manifest source root");
    ASSERT_STR_EQ(result.manifest.app_entry, "app.main", "manifest app entry");
    ASSERT_STR_EQ(result.manifest.entry, "app.main", "manifest internal entry");
    ASSERT_STR_EQ(result.manifest.build_target, "native", "manifest build target");
    ASSERT_STR_EQ(result.manifest.build_output, "build", "manifest build output");
    ASSERT_STR_EQ(result.manifest.build_profile, "release", "manifest build profile");
    ASSERT_TRUE(result.manifest.build_monomorphization_limit == 2048, "manifest build monomorphization limit");
    ASSERT_STR_EQ(result.manifest.test_root, "spec", "manifest test root");
    ASSERT_STR_EQ(result.manifest.zdoc_root, "docs", "manifest zdoc root");
    ASSERT_STR_EQ(result.manifest.output_name, "demo-app", "manifest output_name default");
    ASSERT_STR_EQ(result.manifest.output_dir, "build", "manifest output_dir default");
    ASSERT_TRUE(result.manifest.dependency_count == 2, "manifest dependency count");
    ASSERT_STR_EQ(result.manifest.dependencies[0].name, "json", "manifest dependency json name");
    ASSERT_STR_EQ(result.manifest.dependencies[1].name, "core", "manifest dependency core name");
    ASSERT_STR_EQ(result.manifest.dependencies[1].spec, "1.0.0", "manifest dependency core spec");
    ASSERT_TRUE(result.manifest.dev_dependency_count == 1, "manifest dev dependency count");
    ASSERT_STR_EQ(result.manifest.dev_dependencies[0].name, "test_helpers", "manifest dev dependency name");
}

static void test_manifest_rejects_unknown_legacy_key(void) {
    const char *manifest =
        "[project]\n"
        "name = \"demo-app\"\n"
        "kind = \"app\"\n"
        "version = \"0.1.0\"\n"
        "edition = \"next\"\n"
        "\n"
        "[source]\n"
        "root = \"src\"\n"
        "\n"
        "[app]\n"
        "entry = \"app.main\"\n";
    zt_project_parse_result result;

    ASSERT_TRUE(!zt_project_parse_text(manifest, strlen(manifest), &result), "manifest rejects legacy edition key");
    ASSERT_TRUE(result.code == ZT_PROJECT_UNKNOWN_KEY, "manifest reports unknown key");
}

static void test_manifest_rejects_invalid_target(void) {
    const char *manifest =
        "[project]\n"
        "name = \"demo-app\"\n"
        "kind = \"app\"\n"
        "version = \"0.1.0\"\n"
        "\n"
        "[source]\n"
        "root = \"src\"\n"
        "\n"
        "[app]\n"
        "entry = \"app.main\"\n"
        "\n"
        "[build]\n"
        "target = \"llvm\"\n";
    zt_project_parse_result result;

    ASSERT_TRUE(!zt_project_parse_text(manifest, strlen(manifest), &result), "manifest rejects invalid target");
    ASSERT_TRUE(result.code == ZT_PROJECT_INVALID_TARGET, "manifest reports invalid target");
}

static void test_manifest_rejects_invalid_monomorphization_limit(void) {
    const char *manifest =
        "[project]\n"
        "name = \"demo-app\"\n"
        "kind = \"app\"\n"
        "version = \"0.1.0\"\n"
        "\n"
        "[source]\n"
        "root = \"src\"\n"
        "\n"
        "[app]\n"
        "entry = \"app.main\"\n"
        "\n"
        "[build]\n"
        "monomorphization_limit = 0\n";
    zt_project_parse_result result;

    ASSERT_TRUE(!zt_project_parse_text(manifest, strlen(manifest), &result), "manifest rejects invalid monomorphization limit");
    ASSERT_TRUE(result.code == ZT_PROJECT_INVALID_MONOMORPHIZATION_LIMIT, "manifest reports invalid monomorphization limit");
}
static void test_manifest_rejects_invalid_dependency_spec(void) {
    const char *manifest =
        "[project]\n"
        "name = \"demo-app\"\n"
        "kind = \"app\"\n"
        "version = \"0.1.0\"\n"
        "\n"
        "[source]\n"
        "root = \"src\"\n"
        "\n"
        "[app]\n"
        "entry = \"app.main\"\n"
        "\n"
        "[dependencies]\n"
        "json = 123\n";
    zt_project_parse_result result;

    ASSERT_TRUE(!zt_project_parse_text(manifest, strlen(manifest), &result), "manifest rejects invalid dependency spec");
    ASSERT_TRUE(result.code == ZT_PROJECT_INVALID_ASSIGNMENT, "manifest reports invalid dependency assignment");
}

static void test_manifest_lib_requires_root_namespace(void) {
    const char *manifest =
        "[project]\n"
        "name = \"demo-lib\"\n"
        "kind = \"lib\"\n"
        "version = \"0.1.0\"\n"
        "\n"
        "[source]\n"
        "root = \"src\"\n";
    zt_project_parse_result result;

    ASSERT_TRUE(!zt_project_parse_text(manifest, strlen(manifest), &result), "lib manifest requires root namespace");
    ASSERT_TRUE(result.code == ZT_PROJECT_MISSING_FIELD, "manifest reports missing lib root namespace");
}

static void test_manifest_lib_parse(void) {
    const char *manifest =
        "[project]\n"
        "name = \"demo-lib\"\n"
        "kind = \"lib\"\n"
        "version = \"0.1.0\"\n"
        "\n"
        "[source]\n"
        "root = \"src\"\n"
        "\n"
        "[lib]\n"
        "root_namespace = \"demo_lib\"\n";
    zt_project_parse_result result;

    ASSERT_TRUE(zt_project_parse_text(manifest, strlen(manifest), &result), "lib manifest parses");
    ASSERT_TRUE(zt_project_manifest_kind(&result.manifest) == ZT_PROJECT_KIND_LIB, "lib manifest kind");
    ASSERT_STR_EQ(result.manifest.lib_root_namespace, "demo_lib", "manifest lib root namespace");
    ASSERT_STR_EQ(result.manifest.entry, "demo_lib", "manifest lib internal entry");
    ASSERT_TRUE(result.manifest.build_monomorphization_limit == ZT_PROJECT_DEFAULT_MONOMORPHIZATION_LIMIT, "manifest default monomorphization limit");
}

static void test_entry_resolution(void) {
    const char *manifest_text =
        "[project]\n"
        "name = \"demo-app\"\n"
        "kind = \"app\"\n"
        "version = \"0.1.0\"\n"
        "\n"
        "[source]\n"
        "root = \"src\"\n"
        "\n"
        "[app]\n"
        "entry = \"app.main\"\n";
    zt_project_parse_result result;
    char path[256];

    ASSERT_TRUE(zt_project_parse_text(manifest_text, strlen(manifest_text), &result), "entry resolution manifest parse");
    ASSERT_TRUE(zt_project_resolve_entry_source_path(&result.manifest, "project", path, sizeof(path)), "entry path resolves");
    ASSERT_STR_EQ(path, "project/src/app/main.zt", "entry path uses namespace path");
}

static void test_behavior_fixture_pipeline(void) {
    zt_project_parse_result project;
    char entry_path[512];
    char *source;
    zt_parser_result parsed;
    zt_bind_result bound;
    zt_check_result checked;
    zt_hir_lower_result hir;
    zir_lower_result zir;
    zir_verifier_result verifier;
    c_emitter emitter;
    c_emit_result emit_result;
    const char *c_text;

    if (!zt_project_load_file("tests/behavior/simple_app/zenith.ztproj", &project)) {
        ASSERT_TRUE(0, "behavior fixture project loads");
        return;
    }
    ASSERT_TRUE(1, "behavior fixture project loads");

    if (!zt_project_resolve_entry_source_path(
            &project.manifest,
            "tests/behavior/simple_app",
            entry_path,
            sizeof(entry_path))) {
        ASSERT_TRUE(0, "behavior fixture entry resolves");
        return;
    }
    ASSERT_TRUE(1, "behavior fixture entry resolves");

    source = read_file_or_fail(entry_path);
    parsed = zt_parse(&test_arena, &test_pool, ntry_path, source, strlen(source));
    if (!assert_no_parser_errors("behavior fixture", &parsed)) {
        zt_parser_result_dispose(&parsed);
        free(source);
        return;
    }

    bound = zt_bind_file(parsed.root);
    if (!assert_no_diagnostics("behavior fixture binder", &bound.diagnostics)) {
        zt_bind_result_dispose(&bound);
        zt_parser_result_dispose(&parsed);
        free(source);
        return;
    }

    checked = zt_check_file(parsed.root);
    if (!assert_no_diagnostics("behavior fixture types", &checked.diagnostics)) {
        zt_check_result_dispose(&checked);
        zt_bind_result_dispose(&bound);
        zt_parser_result_dispose(&parsed);
        free(source);
        return;
    }

    hir = zt_lower_ast_to_hir(parsed.root);
    if (!assert_no_diagnostics("behavior fixture HIR lowering", &hir.diagnostics)) {
        zt_hir_lower_result_dispose(&hir);
        zt_check_result_dispose(&checked);
        zt_bind_result_dispose(&bound);
        zt_parser_result_dispose(&parsed);
        free(source);
        return;
    }

    zir = zir_lower_hir_to_zir(hir.module);
    if (!assert_no_diagnostics("behavior fixture ZIR lowering", &zir.diagnostics)) {
        zir_lower_result_dispose(&zir);
        zt_hir_lower_result_dispose(&hir);
        zt_check_result_dispose(&checked);
        zt_bind_result_dispose(&bound);
        zt_parser_result_dispose(&parsed);
        free(source);
        return;
    }

    ASSERT_TRUE(zir_verify_module(&zir.module, &verifier), "behavior fixture ZIR verifies");

    c_emitter_init(&emitter);
    c_emit_result_init(&emit_result);
    ASSERT_TRUE(c_emitter_emit_module(&emitter, &zir.module, &emit_result), "behavior fixture emits C");
    c_text = c_emitter_text(&emitter);
    ASSERT_TRUE(strstr(c_text, "int main(void)") != NULL, "behavior fixture C has main wrapper");
    ASSERT_TRUE(strstr(c_text, "return (a + b);") != NULL, "behavior fixture C has lowered return");

    c_emitter_dispose(&emitter);
    zir_lower_result_dispose(&zir);
    zt_hir_lower_result_dispose(&hir);
    zt_check_result_dispose(&checked);
    zt_bind_result_dispose(&bound);
    zt_parser_result_dispose(&parsed);
    free(source);
}

int main(void) {
    test_manifest_parse();
    test_manifest_rejects_unknown_legacy_key();
    test_manifest_rejects_invalid_target();
    test_manifest_rejects_invalid_monomorphization_limit();
    test_manifest_rejects_invalid_dependency_spec();
    test_manifest_lib_requires_root_namespace();
    test_manifest_lib_parse();
    test_entry_resolution();
    test_behavior_fixture_pipeline();

    printf("Project tests: %d/%d passed\n", tests_passed, tests_run);
    return tests_passed == tests_run ? 0 : 1;
}

