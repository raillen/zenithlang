#include "runtime/c/zenith_rt.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void assert_true(const char *name, int condition) {
    if (!condition) {
        fprintf(stderr, "falha no teste %s\n", name);
        exit(1);
    }
}

static void assert_text_equals(const char *name, const zt_text *value, const char *expected) {
    const char *actual = zt_text_data(value);

    if (strcmp(actual, expected) != 0) {
        fprintf(stderr, "falha no teste %s\n", name);
        fprintf(stderr, "esperado: %s\n", expected);
        fprintf(stderr, "recebido: %s\n", actual);
        exit(1);
    }
}

static void test_shared_text_runtime(void) {
    zt_text *value = zt_text_from_utf8_literal("shared");
    zt_shared_text *shared = zt_shared_text_new(value);
    zt_shared_text *alias;
    const zt_text *borrowed;
    zt_text *snapshot;

    assert_true("shared_text_new_ref_count", zt_shared_text_ref_count(shared) == 1);
    assert_true("shared_text_new_retains_value", value->header.rc == 2);

    alias = zt_shared_text_retain(shared);
    assert_true("shared_text_retain_ref_count", zt_shared_text_ref_count(shared) == 2);

    borrowed = zt_shared_text_borrow(shared);
    assert_text_equals("shared_text_borrow_value", borrowed, "shared");

    snapshot = zt_shared_text_snapshot(shared);
    assert_text_equals("shared_text_snapshot_value", snapshot, "shared");
    assert_true("shared_text_snapshot_distinct", snapshot != borrowed);
    assert_true("shared_text_snapshot_rc", snapshot->header.rc == 1);

    zt_release(value);
    assert_text_equals("shared_text_borrow_after_release", zt_shared_text_borrow(shared), "shared");

    zt_release(snapshot);
    zt_shared_text_release(alias);
    assert_true("shared_text_release_ref_count", zt_shared_text_ref_count(shared) == 1);
    zt_shared_text_release(shared);
}

int main(void) {
    test_shared_text_runtime();
    puts("Runtime shared text tests OK");
    return 0;
}
