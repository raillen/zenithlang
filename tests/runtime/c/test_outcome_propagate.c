#include "runtime/c/zenith_rt.h"

#include <stdio.h>
#include <stdlib.h>

static void assert_true(const char *name, int condition) {
    if (!condition) {
        fprintf(stderr, "assert failed: %s\n", name);
        exit(1);
    }
}

static void test_outcome_text_text_propagate_success(void) {
    zt_text *source = zt_text_from_utf8_literal("ok");
    zt_outcome_text_text original = zt_outcome_text_text_success(source);
    zt_outcome_text_text copy = zt_outcome_text_text_propagate(original);
    zt_text *original_value = zt_outcome_text_text_value(original);
    zt_text *copy_value = zt_outcome_text_text_value(copy);

    assert_true("outcome_text_success_original", zt_outcome_text_text_is_success(original));
    assert_true("outcome_text_success_copy", zt_outcome_text_text_is_success(copy));
    assert_true("outcome_text_success_value_eq", zt_text_eq(original_value, copy_value));

    zt_release(original_value);
    zt_release(copy_value);
    zt_outcome_text_text_dispose(&original);
    zt_outcome_text_text_dispose(&copy);
    zt_release(source);
}

static void test_outcome_text_text_propagate_failure(void) {
    zt_text *error = zt_text_from_utf8_literal("boom");
    zt_outcome_text_text original = zt_outcome_text_text_failure(error);
    zt_outcome_text_text copy = zt_outcome_text_text_propagate(original);

    assert_true("outcome_text_failure_original", !zt_outcome_text_text_is_success(original));
    assert_true("outcome_text_failure_copy", !zt_outcome_text_text_is_success(copy));
    assert_true("outcome_text_failure_eq", zt_outcome_text_text_eq(original, copy));

    zt_outcome_text_text_dispose(&original);
    zt_outcome_text_text_dispose(&copy);
    zt_release(error);
}

int main(void) {
    test_outcome_text_text_propagate_success();
    test_outcome_text_text_propagate_failure();
    puts("Runtime outcome propagate tests OK");
    return 0;
}
