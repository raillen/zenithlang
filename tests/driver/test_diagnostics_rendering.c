/*
 * R3.M1 - Diagnostics rendering unit tests.
 *
 * Exercises the action-first and CI renderers to verify the ACTION/WHY/NEXT
 * contract, effort-hint calibration, and profile error limits.
 */

#include "compiler/semantic/diagnostics/diagnostics.h"
#include "compiler/frontend/lexer/token.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int tests_run = 0;
static int tests_passed = 0;

#define ASSERT_TRUE(cond, msg) do { \
    tests_run += 1; \
    if (cond) { tests_passed += 1; } \
    else { fprintf(stderr, "FAIL: %s\n", (msg)); } \
} while (0)

#define ASSERT_CONTAINS(buffer, needle, msg) do { \
    tests_run += 1; \
    if ((buffer) != NULL && strstr((buffer), (needle)) != NULL) { \
        tests_passed += 1; \
    } else { \
        fprintf(stderr, "FAIL: %s: expected to find \"%s\"\n", (msg), (needle)); \
        if ((buffer) != NULL) { \
            fprintf(stderr, "----- actual output -----\n%s\n-------------------------\n", (buffer)); \
        } \
    } \
} while (0)

#define ASSERT_ENUM_EQ(actual, expected, msg) do { \
    tests_run += 1; \
    if ((int)(actual) == (int)(expected)) { tests_passed += 1; } \
    else { fprintf(stderr, "FAIL: %s: expected %d, got %d\n", (msg), (int)(expected), (int)(actual)); } \
} while (0)

#define ASSERT_SIZE_EQ(actual, expected, msg) do { \
    tests_run += 1; \
    if ((size_t)(actual) == (size_t)(expected)) { tests_passed += 1; } \
    else { fprintf(stderr, "FAIL: %s: expected %zu, got %zu\n", (msg), (size_t)(expected), (size_t)(actual)); } \
} while (0)

static char *capture_render_action_first(const zt_diag *diag, const char *stage) {
    FILE *tmp;
    long size;
    char *buf;
    size_t read;

    tmp = tmpfile();
    if (tmp == NULL) return NULL;

    zt_diag_render_action_first(tmp, stage, diag);
    fflush(tmp);

    fseek(tmp, 0, SEEK_END);
    size = ftell(tmp);
    fseek(tmp, 0, SEEK_SET);

    if (size <= 0) {
        fclose(tmp);
        return NULL;
    }

    buf = (char *)malloc((size_t)size + 1);
    if (buf == NULL) {
        fclose(tmp);
        return NULL;
    }

    read = fread(buf, 1, (size_t)size, tmp);
    buf[read] = '\0';
    fclose(tmp);
    return buf;
}

static char *capture_render_ci(const zt_diag *diag, const char *stage) {
    FILE *tmp;
    long size;
    char *buf;
    size_t read;

    tmp = tmpfile();
    if (tmp == NULL) return NULL;

    zt_diag_render_ci(tmp, stage, diag);
    fflush(tmp);

    fseek(tmp, 0, SEEK_END);
    size = ftell(tmp);
    fseek(tmp, 0, SEEK_SET);

    if (size <= 0) {
        fclose(tmp);
        return NULL;
    }

    buf = (char *)malloc((size_t)size + 1);
    if (buf == NULL) {
        fclose(tmp);
        return NULL;
    }

    read = fread(buf, 1, (size_t)size, tmp);
    buf[read] = '\0';
    fclose(tmp);
    return buf;
}

static zt_diag make_diag(zt_diag_code code, const char *message) {
    zt_diag diag;
    memset(&diag, 0, sizeof(diag));
    diag.code = code;
    diag.severity = ZT_DIAG_SEVERITY_ERROR;
    diag.effort = zt_diag_code_effort(code);
    diag.span = zt_source_span_make("tests/fake.zt", 8, 21, 25);
    snprintf(diag.message, sizeof(diag.message), "%s", message != NULL ? message : "");
    diag.suggestion[0] = '\0';
    return diag;
}

static void test_action_first_has_action_why_next(void) {
    zt_diag diag = make_diag(ZT_DIAG_TYPE_MISMATCH, "expected int but found text");
    char *out = capture_render_action_first(&diag, "semantic");

    ASSERT_TRUE(out != NULL, "action-first: rendered output available");
    ASSERT_CONTAINS(out, "ACTION:", "action-first: ACTION block present");
    ASSERT_CONTAINS(out, "WHY:", "action-first: WHY block present");
    ASSERT_CONTAINS(out, "NEXT:", "action-first: NEXT block present (R3.M1 requirement)");
    ASSERT_CONTAINS(out, "type.mismatch", "action-first: stable code surfaced");
    ASSERT_CONTAINS(out, "tests/fake.zt:8:21", "action-first: WHERE span surfaced");
    ASSERT_CONTAINS(out, "expected int but found text", "action-first: original message surfaced as WHY");

    free(out);
}

static void test_action_first_next_for_unresolved_name(void) {
    zt_diag diag = make_diag(ZT_DIAG_UNRESOLVED_NAME, "cannot find `foo` in this scope");
    char *out = capture_render_action_first(&diag, "semantic");

    ASSERT_TRUE(out != NULL, "action-first unresolved: rendered output available");
    ASSERT_CONTAINS(out, "ACTION:", "action-first unresolved: ACTION block present");
    ASSERT_CONTAINS(out, "NEXT:", "action-first unresolved: NEXT block present");
    ASSERT_CONTAINS(out, "zt check", "action-first unresolved: NEXT refers to `zt check`");
    ASSERT_CONTAINS(out, "name.unresolved", "action-first unresolved: stable code surfaced");

    free(out);
}

static void test_ci_emits_effort_and_next_fields(void) {
    zt_diag diag = make_diag(ZT_DIAG_TYPE_MISMATCH, "expected int but found text");
    char *out = capture_render_ci(&diag, "semantic");

    ASSERT_TRUE(out != NULL, "ci: rendered output available");
    ASSERT_CONTAINS(out, "type.mismatch", "ci: stable code surfaced");
    ASSERT_CONTAINS(out, "stage=semantic", "ci: stage field present");
    ASSERT_CONTAINS(out, "effort=", "ci: effort field present");
    ASSERT_CONTAINS(out, "action=\"", "ci: action field present");
    ASSERT_CONTAINS(out, "next=\"", "ci: next field present (R3.M1 requirement)");

    free(out);
}

static void test_effort_hint_calibration(void) {
    /* Quick fix class */
    ASSERT_ENUM_EQ(zt_diag_code_effort(ZT_DIAG_SYNTAX_ERROR), ZT_DIAG_EFFORT_QUICK_FIX, "effort: syntax_error is quick_fix");
    ASSERT_ENUM_EQ(zt_diag_code_effort(ZT_DIAG_UNEXPECTED_TOKEN), ZT_DIAG_EFFORT_QUICK_FIX, "effort: unexpected_token is quick_fix");
    ASSERT_ENUM_EQ(zt_diag_code_effort(ZT_DIAG_UNRESOLVED_NAME), ZT_DIAG_EFFORT_QUICK_FIX, "effort: unresolved_name is quick_fix");
    ASSERT_ENUM_EQ(zt_diag_code_effort(ZT_DIAG_CONST_REASSIGNMENT), ZT_DIAG_EFFORT_QUICK_FIX, "effort: const_reassignment is quick_fix");

    /* Moderate class */
    ASSERT_ENUM_EQ(zt_diag_code_effort(ZT_DIAG_TYPE_MISMATCH), ZT_DIAG_EFFORT_MODERATE, "effort: type_mismatch is moderate");
    ASSERT_ENUM_EQ(zt_diag_code_effort(ZT_DIAG_INVALID_CALL), ZT_DIAG_EFFORT_MODERATE, "effort: invalid_call is moderate");
    ASSERT_ENUM_EQ(zt_diag_code_effort(ZT_DIAG_INVALID_ARGUMENT), ZT_DIAG_EFFORT_MODERATE, "effort: invalid_argument is moderate");
    ASSERT_ENUM_EQ(zt_diag_code_effort(ZT_DIAG_PROJECT_UNRESOLVED_IMPORT), ZT_DIAG_EFFORT_MODERATE, "effort: project_unresolved_import is moderate");

    /* Requires thinking class */
    ASSERT_ENUM_EQ(zt_diag_code_effort(ZT_DIAG_NON_EXHAUSTIVE_MATCH), ZT_DIAG_EFFORT_REQUIRES_THINKING, "effort: non_exhaustive_match is requires_thinking");
    ASSERT_ENUM_EQ(zt_diag_code_effort(ZT_DIAG_PROJECT_IMPORT_CYCLE), ZT_DIAG_EFFORT_REQUIRES_THINKING, "effort: project_import_cycle is requires_thinking");
    ASSERT_ENUM_EQ(zt_diag_code_effort(ZT_DIAG_INTEGER_OVERFLOW), ZT_DIAG_EFFORT_REQUIRES_THINKING, "effort: integer_overflow is requires_thinking");
    ASSERT_ENUM_EQ(zt_diag_code_effort(ZT_DIAG_BACKEND_C_EMIT_ERROR), ZT_DIAG_EFFORT_REQUIRES_THINKING, "effort: backend_c_emit_error is requires_thinking");
}

static void test_profile_error_limits(void) {
    ASSERT_SIZE_EQ(zt_cog_profile_error_limit(ZT_COG_PROFILE_BEGINNER), 3, "profile: beginner caps at 3");
    ASSERT_SIZE_EQ(zt_cog_profile_error_limit(ZT_COG_PROFILE_BALANCED), 5, "profile: balanced caps at 5");
    ASSERT_SIZE_EQ(zt_cog_profile_error_limit(ZT_COG_PROFILE_FULL), (size_t)-1, "profile: full is unbounded");

    ASSERT_ENUM_EQ(zt_cog_profile_from_text("beginner"), ZT_COG_PROFILE_BEGINNER, "profile: parses beginner");
    ASSERT_ENUM_EQ(zt_cog_profile_from_text("balanced"), ZT_COG_PROFILE_BALANCED, "profile: parses balanced");
    ASSERT_ENUM_EQ(zt_cog_profile_from_text("full"), ZT_COG_PROFILE_FULL, "profile: parses full");
    ASSERT_ENUM_EQ(zt_cog_profile_from_text("unknown"), ZT_COG_PROFILE_BALANCED, "profile: unknown falls back to balanced");
    ASSERT_ENUM_EQ(zt_cog_profile_from_text(NULL), ZT_COG_PROFILE_BALANCED, "profile: null falls back to balanced");
}

static void test_action_first_list_profile_cap(void) {
    zt_diag_list list = zt_diag_list_make();
    zt_source_span span = zt_source_span_make("tests/fake.zt", 1, 1, 1);
    FILE *tmp;
    char buf[4096];
    size_t read;
    size_t i;

    for (i = 0; i < 6; i += 1) {
        zt_diag_list_add(&list, ZT_DIAG_TYPE_MISMATCH, span, "error %zu", i);
    }

    tmp = tmpfile();
    ASSERT_TRUE(tmp != NULL, "list-cap: tmpfile opened");
    zt_diag_render_action_first_list(tmp, "semantic", &list, zt_cog_profile_error_limit(ZT_COG_PROFILE_BEGINNER));
    fflush(tmp);
    fseek(tmp, 0, SEEK_SET);
    read = fread(buf, 1, sizeof(buf) - 1, tmp);
    buf[read] = '\0';
    fclose(tmp);

    ASSERT_CONTAINS(buf, "and 3 more error(s)", "list-cap: beginner profile truncates after 3 and annotates suppression");
    ASSERT_CONTAINS(buf, "--all", "list-cap: suppression footer suggests --all");

    zt_diag_list_dispose(&list);
}

int main(void) {
    test_action_first_has_action_why_next();
    test_action_first_next_for_unresolved_name();
    test_ci_emits_effort_and_next_fields();
    test_effort_hint_calibration();
    test_profile_error_limits();
    test_action_first_list_profile_cap();

    printf("Diagnostics rendering tests: %d/%d passed\n", tests_passed, tests_run);
    return tests_passed == tests_run ? 0 : 1;
}
