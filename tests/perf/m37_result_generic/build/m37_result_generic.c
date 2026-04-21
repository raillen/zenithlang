#include "runtime/c/zenith_rt.h"
#include <stdio.h>
#include <string.h>


typedef struct zt_app_main__io_Input {
    zt_int handle;
} zt_app_main__io_Input;

typedef struct zt_app_main__io_Output {
    zt_int handle;
} zt_app_main__io_Output;

typedef enum zt_app_main__io_Error_tag {
    zt_app_main__io_Error__ReadFailed,
    zt_app_main__io_Error__WriteFailed,
    zt_app_main__io_Error__Unknown
} zt_app_main__io_Error_tag;

typedef struct zt_app_main__io_Error {
    zt_app_main__io_Error_tag tag;
} zt_app_main__io_Error;

typedef struct zt_generated_outcome_bool_core_error_ {
    zt_bool is_success;
    zt_bool value;
    zt_core_error error;
} zt_generated_outcome_bool_core_error_;

static zt_generated_outcome_bool_core_error_ zt_generated_outcome_bool_core_error__success(zt_bool value) {
    zt_generated_outcome_bool_core_error_ outcome;
    memset(&outcome, 0, sizeof(outcome));
    outcome.is_success = true;
    outcome.value = value;
    return outcome;
}

static zt_generated_outcome_bool_core_error_ zt_generated_outcome_bool_core_error__failure(zt_core_error error) {
    zt_generated_outcome_bool_core_error_ outcome;
    memset(&outcome, 0, sizeof(outcome));
    outcome.is_success = false;
    outcome.error = error;
    outcome.error = zt_core_error_clone(outcome.error);
    return outcome;
}

static zt_generated_outcome_bool_core_error_ zt_generated_outcome_bool_core_error__failure_message(const char *message) {
    zt_core_error error = zt_core_error_from_message("error", message != NULL ? message : "error");
    zt_generated_outcome_bool_core_error_ outcome = zt_generated_outcome_bool_core_error__failure(error);
    zt_core_error_dispose(&error);
    return outcome;
}

static zt_generated_outcome_bool_core_error_ zt_generated_outcome_bool_core_error__failure_text(zt_text *message) {
    zt_core_error error = zt_core_error_from_text("error", message);
    zt_generated_outcome_bool_core_error_ outcome = zt_generated_outcome_bool_core_error__failure(error);
    zt_core_error_dispose(&error);
    return outcome;
}

static zt_bool zt_generated_outcome_bool_core_error__is_success(zt_generated_outcome_bool_core_error_ outcome) {
    return outcome.is_success;
}

static zt_bool zt_generated_outcome_bool_core_error__value(zt_generated_outcome_bool_core_error_ outcome) {
    if (!outcome.is_success) zt_runtime_error(ZT_ERR_UNWRAP, "outcome_value on failure");
    {
        zt_bool value = outcome.value;
        return value;
    }
}

static zt_generated_outcome_bool_core_error_ zt_generated_outcome_bool_core_error__propagate(zt_generated_outcome_bool_core_error_ outcome) {
    if (outcome.is_success) return zt_generated_outcome_bool_core_error__success(outcome.value);
    return zt_generated_outcome_bool_core_error__failure(outcome.error);
}

static void zt_generated_outcome_bool_core_error__dispose(zt_generated_outcome_bool_core_error_ *outcome) {
    if (outcome == NULL) return;
    if (outcome->is_success) {
    } else {
        zt_core_error_dispose(&outcome->error);
    }
    memset(outcome, 0, sizeof(*outcome));
}

typedef struct zt_generated_outcome_void_bool_ {
    zt_bool is_success;
    zt_bool error;
} zt_generated_outcome_void_bool_;

static zt_generated_outcome_void_bool_ zt_generated_outcome_void_bool__success(void) {
    zt_generated_outcome_void_bool_ outcome;
    memset(&outcome, 0, sizeof(outcome));
    outcome.is_success = true;
    return outcome;
}

static zt_generated_outcome_void_bool_ zt_generated_outcome_void_bool__failure(zt_bool error) {
    zt_generated_outcome_void_bool_ outcome;
    memset(&outcome, 0, sizeof(outcome));
    outcome.is_success = false;
    outcome.error = error;
    return outcome;
}

static zt_bool zt_generated_outcome_void_bool__is_success(zt_generated_outcome_void_bool_ outcome) {
    return outcome.is_success;
}

static zt_generated_outcome_void_bool_ zt_generated_outcome_void_bool__propagate(zt_generated_outcome_void_bool_ outcome) {
    if (outcome.is_success) return zt_generated_outcome_void_bool__success();
    return zt_generated_outcome_void_bool__failure(outcome.error);
}

static void zt_generated_outcome_void_bool__dispose(zt_generated_outcome_void_bool_ *outcome) {
    if (outcome == NULL) return;
    if (outcome->is_success) {
    } else {
    }
    memset(outcome, 0, sizeof(*outcome));
}

typedef struct zt_generated_outcome_text_bool_ {
    zt_bool is_success;
    zt_text *value;
    zt_bool error;
} zt_generated_outcome_text_bool_;

static zt_generated_outcome_text_bool_ zt_generated_outcome_text_bool__success(zt_text * value) {
    zt_generated_outcome_text_bool_ outcome;
    memset(&outcome, 0, sizeof(outcome));
    outcome.is_success = true;
    outcome.value = value;
    if (outcome.value != NULL) { zt_retain(outcome.value); }
    return outcome;
}

static zt_generated_outcome_text_bool_ zt_generated_outcome_text_bool__failure(zt_bool error) {
    zt_generated_outcome_text_bool_ outcome;
    memset(&outcome, 0, sizeof(outcome));
    outcome.is_success = false;
    outcome.error = error;
    return outcome;
}

static zt_bool zt_generated_outcome_text_bool__is_success(zt_generated_outcome_text_bool_ outcome) {
    return outcome.is_success;
}

static zt_text * zt_generated_outcome_text_bool__value(zt_generated_outcome_text_bool_ outcome) {
    if (!outcome.is_success) zt_runtime_error(ZT_ERR_UNWRAP, "outcome_value on failure");
    {
        zt_text *value = outcome.value;
        if (value != NULL) { zt_retain(value); }
        return value;
    }
}

static zt_generated_outcome_text_bool_ zt_generated_outcome_text_bool__propagate(zt_generated_outcome_text_bool_ outcome) {
    if (outcome.is_success) return zt_generated_outcome_text_bool__success(outcome.value);
    return zt_generated_outcome_text_bool__failure(outcome.error);
}

static void zt_generated_outcome_text_bool__dispose(zt_generated_outcome_text_bool_ *outcome) {
    if (outcome == NULL) return;
    if (outcome->is_success) {
        if (outcome->value != NULL) { zt_release(outcome->value); outcome->value = NULL; }
    } else {
    }
    memset(outcome, 0, sizeof(*outcome));
}

typedef struct zt_generated_outcome_int_bool_ {
    zt_bool is_success;
    zt_int value;
    zt_bool error;
} zt_generated_outcome_int_bool_;

static zt_generated_outcome_int_bool_ zt_generated_outcome_int_bool__success(zt_int value) {
    zt_generated_outcome_int_bool_ outcome;
    memset(&outcome, 0, sizeof(outcome));
    outcome.is_success = true;
    outcome.value = value;
    return outcome;
}

static zt_generated_outcome_int_bool_ zt_generated_outcome_int_bool__failure(zt_bool error) {
    zt_generated_outcome_int_bool_ outcome;
    memset(&outcome, 0, sizeof(outcome));
    outcome.is_success = false;
    outcome.error = error;
    return outcome;
}

static zt_bool zt_generated_outcome_int_bool__is_success(zt_generated_outcome_int_bool_ outcome) {
    return outcome.is_success;
}

static zt_int zt_generated_outcome_int_bool__value(zt_generated_outcome_int_bool_ outcome) {
    if (!outcome.is_success) zt_runtime_error(ZT_ERR_UNWRAP, "outcome_value on failure");
    {
        zt_int value = outcome.value;
        return value;
    }
}

static zt_generated_outcome_int_bool_ zt_generated_outcome_int_bool__propagate(zt_generated_outcome_int_bool_ outcome) {
    if (outcome.is_success) return zt_generated_outcome_int_bool__success(outcome.value);
    return zt_generated_outcome_int_bool__failure(outcome.error);
}

static void zt_generated_outcome_int_bool__dispose(zt_generated_outcome_int_bool_ *outcome) {
    if (outcome == NULL) return;
    if (outcome->is_success) {
    } else {
    }
    memset(outcome, 0, sizeof(*outcome));
}
static zt_outcome_text_core_error zt_app_main__managed_text(zt_int seed);
static zt_generated_outcome_bool_core_error_ zt_app_main__managed_flag(zt_int value);
static zt_outcome_i64_core_error zt_app_main__managed_batch(zt_int iterations);
static zt_generated_outcome_void_bool_ zt_app_main__bool_guard(zt_int seed);
static zt_generated_outcome_text_bool_ zt_app_main__bool_text(zt_int seed);
static zt_generated_outcome_int_bool_ zt_app_main__bool_batch(zt_int iterations);
static zt_outcome_void_core_error zt_app_main__main(void);
static zt_outcome_optional_text_core_error zt_app_main__io_read_line(zt_app_main__io_Input from);
static zt_outcome_text_core_error zt_app_main__io_read_all(zt_app_main__io_Input from);
static zt_outcome_void_core_error zt_app_main__io_write(zt_text *value, zt_app_main__io_Output to);
static zt_outcome_void_core_error zt_app_main__io_print(zt_text *value, zt_app_main__io_Output to);

static zt_outcome_text_core_error zt_app_main__managed_text(zt_int seed) {
    zt_text *even_label = NULL;
    zt_text *odd_label = NULL;
    zt_outcome_text_core_error zt_return_value = {0};
    goto zt_block_entry;

zt_block_entry:
    if ((seed < 0)) goto zt_block_if_then_0;
    goto zt_block_if_else_1;

zt_block_if_then_0:
    zt_return_value = zt_outcome_text_core_error_failure_message("m37.managed_text negative seed");
    goto zt_cleanup;

zt_block_if_else_1:
    goto zt_block_if_join_2;

zt_block_if_join_2:
    if ((zt_rem_i64(seed, 2) == 0)) goto zt_block_if_then_3;
    goto zt_block_if_else_4;

zt_block_if_then_3:
    if (even_label != NULL) { zt_release(even_label); even_label = NULL; }
    even_label = zt_text_from_utf8_literal("zenith");
    zt_return_value = zt_outcome_text_core_error_success(even_label);
    goto zt_cleanup;

zt_block_if_else_4:
    goto zt_block_if_join_5;

zt_block_if_join_5:
    if (odd_label != NULL) { zt_release(odd_label); odd_label = NULL; }
    odd_label = zt_text_from_utf8_literal("generic");
    zt_return_value = zt_outcome_text_core_error_success(odd_label);
    goto zt_cleanup;

zt_cleanup:
    if (even_label != NULL) { zt_release(even_label); even_label = NULL; }
    if (odd_label != NULL) { zt_release(odd_label); odd_label = NULL; }
    return zt_return_value;
}

static zt_generated_outcome_bool_core_error_ zt_app_main__managed_flag(zt_int value) {
    goto zt_block_entry;

zt_block_entry:
    if ((value < 0)) goto zt_block_if_then_0;
    goto zt_block_if_else_1;

zt_block_if_then_0:
    return zt_generated_outcome_bool_core_error__failure_message("m37.managed_flag negative value");

zt_block_if_else_1:
    goto zt_block_if_join_2;

zt_block_if_join_2:
    return zt_generated_outcome_bool_core_error__success((value > 0));
}

static zt_outcome_i64_core_error zt_app_main__managed_batch(zt_int iterations) {
    zt_int i;
    zt_int total;
    zt_outcome_text_core_error __zt_try_result_0 = {0};
    zt_text *label = NULL;
    zt_generated_outcome_bool_core_error_ __zt_try_result_1 = {0};
    zt_bool ok;
    zt_outcome_i64_core_error zt_return_value = {0};
    goto zt_block_entry;

zt_block_entry:
    i = 0;
    total = 0;
    goto zt_block_while_cond_0;

zt_block_while_cond_0:
    if ((i < iterations)) goto zt_block_while_body_1;
    goto zt_block_while_exit_2;

zt_block_while_body_1:
    zt_outcome_text_core_error_dispose(&__zt_try_result_0);
    __zt_try_result_0 = zt_app_main__managed_text(i);
    if (zt_outcome_text_core_error_is_success(__zt_try_result_0)) goto zt_block_try_success_0;
    goto zt_block_try_failure_1;

zt_block_try_failure_1:
    zt_return_value = zt_outcome_i64_core_error_failure((__zt_try_result_0).error);
    goto zt_cleanup;

zt_block_try_success_0:
    if (label != NULL) { zt_release(label); label = NULL; }
    label = zt_outcome_text_core_error_value(__zt_try_result_0);
    goto zt_block_try_after_2;

zt_block_try_after_2:
    zt_generated_outcome_bool_core_error__dispose(&__zt_try_result_1);
    __zt_try_result_1 = zt_app_main__managed_flag(zt_text_len(label));
    if (zt_generated_outcome_bool_core_error__is_success(__zt_try_result_1)) goto zt_block_try_success_3;
    goto zt_block_try_failure_4;

zt_block_try_failure_4:
    zt_return_value = zt_outcome_i64_core_error_failure((__zt_try_result_1).error);
    goto zt_cleanup;

zt_block_try_success_3:
    ok = zt_generated_outcome_bool_core_error__value(__zt_try_result_1);
    goto zt_block_try_after_5;

zt_block_try_after_5:
    if (ok) goto zt_block_if_then_0;
    goto zt_block_if_else_1;

zt_block_if_then_0:
    total = zt_add_i64(total, zt_text_len(label));
    goto zt_block_if_join_2;

zt_block_if_else_1:
    goto zt_block_if_join_2;

zt_block_if_join_2:
    i = zt_add_i64(i, 1);
    goto zt_block_while_cond_0;

zt_block_while_exit_2:
    zt_return_value = zt_outcome_i64_core_error_success(total);
    goto zt_cleanup;

zt_cleanup:
    zt_outcome_text_core_error_dispose(&__zt_try_result_0);
    if (label != NULL) { zt_release(label); label = NULL; }
    zt_generated_outcome_bool_core_error__dispose(&__zt_try_result_1);
    return zt_return_value;
}

static zt_generated_outcome_void_bool_ zt_app_main__bool_guard(zt_int seed) {
    goto zt_block_entry;

zt_block_entry:
    if ((seed < 0)) goto zt_block_if_then_0;
    goto zt_block_if_else_1;

zt_block_if_then_0:
    return zt_generated_outcome_void_bool__failure(false);

zt_block_if_else_1:
    goto zt_block_if_join_2;

zt_block_if_join_2:
    return zt_generated_outcome_void_bool__success();
}

static zt_generated_outcome_text_bool_ zt_app_main__bool_text(zt_int seed) {
    zt_generated_outcome_void_bool_ __zt_try_result_0 = {0};
    zt_text *even_label = NULL;
    zt_text *odd_label = NULL;
    zt_generated_outcome_text_bool_ zt_return_value = {0};
    goto zt_block_entry;

zt_block_entry:
    zt_generated_outcome_void_bool__dispose(&__zt_try_result_0);
    __zt_try_result_0 = zt_app_main__bool_guard(seed);
    if (zt_generated_outcome_void_bool__is_success(__zt_try_result_0)) goto zt_block_try_success_0;
    goto zt_block_try_failure_1;

zt_block_try_failure_1:
    zt_return_value = zt_generated_outcome_text_bool__failure((__zt_try_result_0).error);
    goto zt_cleanup;

zt_block_try_success_0:
    goto zt_block_try_after_2;

zt_block_try_after_2:
    if ((zt_rem_i64(seed, 2) == 0)) goto zt_block_if_then_0;
    goto zt_block_if_else_1;

zt_block_if_then_0:
    if (even_label != NULL) { zt_release(even_label); even_label = NULL; }
    even_label = zt_text_from_utf8_literal("bool-even");
    zt_return_value = zt_generated_outcome_text_bool__success(even_label);
    goto zt_cleanup;

zt_block_if_else_1:
    goto zt_block_if_join_2;

zt_block_if_join_2:
    if (odd_label != NULL) { zt_release(odd_label); odd_label = NULL; }
    odd_label = zt_text_from_utf8_literal("bool-odd");
    zt_return_value = zt_generated_outcome_text_bool__success(odd_label);
    goto zt_cleanup;

zt_cleanup:
    zt_generated_outcome_void_bool__dispose(&__zt_try_result_0);
    if (even_label != NULL) { zt_release(even_label); even_label = NULL; }
    if (odd_label != NULL) { zt_release(odd_label); odd_label = NULL; }
    return zt_return_value;
}

static zt_generated_outcome_int_bool_ zt_app_main__bool_batch(zt_int iterations) {
    zt_int i;
    zt_int total;
    zt_generated_outcome_text_bool_ __zt_try_result_0 = {0};
    zt_text *label = NULL;
    zt_generated_outcome_int_bool_ zt_return_value = {0};
    goto zt_block_entry;

zt_block_entry:
    i = 0;
    total = 0;
    goto zt_block_while_cond_0;

zt_block_while_cond_0:
    if ((i < iterations)) goto zt_block_while_body_1;
    goto zt_block_while_exit_2;

zt_block_while_body_1:
    zt_generated_outcome_text_bool__dispose(&__zt_try_result_0);
    __zt_try_result_0 = zt_app_main__bool_text(i);
    if (zt_generated_outcome_text_bool__is_success(__zt_try_result_0)) goto zt_block_try_success_0;
    goto zt_block_try_failure_1;

zt_block_try_failure_1:
    zt_return_value = zt_generated_outcome_int_bool__failure((__zt_try_result_0).error);
    goto zt_cleanup;

zt_block_try_success_0:
    if (label != NULL) { zt_release(label); label = NULL; }
    label = zt_generated_outcome_text_bool__value(__zt_try_result_0);
    goto zt_block_try_after_2;

zt_block_try_after_2:
    total = zt_add_i64(total, zt_text_len(label));
    i = zt_add_i64(i, 1);
    goto zt_block_while_cond_0;

zt_block_while_exit_2:
    zt_return_value = zt_generated_outcome_int_bool__success(total);
    goto zt_cleanup;

zt_cleanup:
    zt_generated_outcome_text_bool__dispose(&__zt_try_result_0);
    if (label != NULL) { zt_release(label); label = NULL; }
    return zt_return_value;
}

static zt_outcome_void_core_error zt_app_main__main(void) {
    zt_outcome_i64_core_error __zt_try_result_0 = {0};
    zt_int managed_a;
    zt_outcome_i64_core_error __zt_try_result_1 = {0};
    zt_int managed_b;
    zt_generated_outcome_int_bool_ bool_total = {0};
    zt_generated_outcome_text_bool_ bool_once = {0};
    zt_generated_outcome_void_bool_ bool_void = {0};
    zt_outcome_void_core_error __zt_try_result_2 = {0};
    zt_outcome_void_core_error __zt_try_result_3 = {0};
    zt_outcome_void_core_error __zt_try_result_4 = {0};
    zt_outcome_void_core_error __zt_try_result_5 = {0};
    zt_outcome_void_core_error __zt_try_result_6 = {0};
    zt_outcome_void_core_error zt_return_value = {0};
    goto zt_block_entry;

zt_block_entry:
    zt_outcome_i64_core_error_dispose(&__zt_try_result_0);
    __zt_try_result_0 = zt_app_main__managed_batch(60000);
    if (zt_outcome_i64_core_error_is_success(__zt_try_result_0)) goto zt_block_try_success_0;
    goto zt_block_try_failure_1;

zt_block_try_failure_1:
    zt_return_value = zt_outcome_void_core_error_failure((__zt_try_result_0).error);
    goto zt_cleanup;

zt_block_try_success_0:
    managed_a = zt_outcome_i64_core_error_value(__zt_try_result_0);
    goto zt_block_try_after_2;

zt_block_try_after_2:
    zt_outcome_i64_core_error_dispose(&__zt_try_result_1);
    __zt_try_result_1 = zt_app_main__managed_batch(60000);
    if (zt_outcome_i64_core_error_is_success(__zt_try_result_1)) goto zt_block_try_success_3;
    goto zt_block_try_failure_4;

zt_block_try_failure_4:
    zt_return_value = zt_outcome_void_core_error_failure((__zt_try_result_1).error);
    goto zt_cleanup;

zt_block_try_success_3:
    managed_b = zt_outcome_i64_core_error_value(__zt_try_result_1);
    goto zt_block_try_after_5;

zt_block_try_after_5:
    zt_generated_outcome_int_bool__dispose(&bool_total);
    bool_total = zt_app_main__bool_batch(30000);
    zt_generated_outcome_text_bool__dispose(&bool_once);
    bool_once = zt_app_main__bool_text(1);
    zt_generated_outcome_void_bool__dispose(&bool_void);
    bool_void = zt_app_main__bool_guard(1);
    if (((managed_a <= 0) || (managed_b <= 0))) goto zt_block_if_then_0;
    goto zt_block_if_else_1;

zt_block_if_then_0:
    zt_return_value = zt_outcome_void_core_error_failure_message("m37.managed batch should stay positive");
    goto zt_cleanup;

zt_block_if_else_1:
    goto zt_block_if_join_2;

zt_block_if_join_2:
    if ((managed_a == managed_b)) goto zt_block_if_then_3;
    goto zt_block_if_else_4;

zt_block_if_then_3:
    zt_outcome_void_core_error_dispose(&__zt_try_result_2);
    __zt_try_result_2 = zt_app_main__io_print(zt_text_from_utf8_literal(""), ((zt_app_main__io_Output){.handle = 1}));
    if (zt_outcome_void_core_error_is_success(__zt_try_result_2)) goto zt_block_try_success_6;
    goto zt_block_try_failure_7;

zt_block_try_failure_7:
    zt_return_value = zt_outcome_void_core_error_propagate(__zt_try_result_2);
    goto zt_cleanup;

zt_block_try_success_6:
    goto zt_block_try_after_8;

zt_block_try_after_8:
    goto zt_block_if_join_5;

zt_block_if_else_4:
    goto zt_block_if_join_5;

zt_block_if_join_5:
    zt_outcome_void_core_error_dispose(&__zt_try_result_3);
    __zt_try_result_3 = zt_app_main__io_print(zt_text_from_utf8_literal(""), ((zt_app_main__io_Output){.handle = 1}));
    if (zt_outcome_void_core_error_is_success(__zt_try_result_3)) goto zt_block_try_success_9;
    goto zt_block_try_failure_10;

zt_block_try_failure_10:
    zt_return_value = zt_outcome_void_core_error_propagate(__zt_try_result_3);
    goto zt_cleanup;

zt_block_try_success_9:
    goto zt_block_try_after_11;

zt_block_try_after_11:
    zt_outcome_void_core_error_dispose(&__zt_try_result_4);
    __zt_try_result_4 = zt_app_main__io_print(zt_text_from_utf8_literal(""), ((zt_app_main__io_Output){.handle = 1}));
    if (zt_outcome_void_core_error_is_success(__zt_try_result_4)) goto zt_block_try_success_12;
    goto zt_block_try_failure_13;

zt_block_try_failure_13:
    zt_return_value = zt_outcome_void_core_error_propagate(__zt_try_result_4);
    goto zt_cleanup;

zt_block_try_success_12:
    goto zt_block_try_after_14;

zt_block_try_after_14:
    zt_outcome_void_core_error_dispose(&__zt_try_result_5);
    __zt_try_result_5 = zt_app_main__io_write(zt_text_from_utf8_literal("m37-result-generic-ok"), ((zt_app_main__io_Output){.handle = 1}));
    if (zt_outcome_void_core_error_is_success(__zt_try_result_5)) goto zt_block_try_success_15;
    goto zt_block_try_failure_16;

zt_block_try_failure_16:
    zt_return_value = zt_outcome_void_core_error_propagate(__zt_try_result_5);
    goto zt_cleanup;

zt_block_try_success_15:
    goto zt_block_try_after_17;

zt_block_try_after_17:
    zt_outcome_void_core_error_dispose(&__zt_try_result_6);
    __zt_try_result_6 = zt_app_main__io_write(zt_text_from_utf8_literal("\n"), ((zt_app_main__io_Output){.handle = 1}));
    if (zt_outcome_void_core_error_is_success(__zt_try_result_6)) goto zt_block_try_success_18;
    goto zt_block_try_failure_19;

zt_block_try_failure_19:
    zt_return_value = zt_outcome_void_core_error_propagate(__zt_try_result_6);
    goto zt_cleanup;

zt_block_try_success_18:
    goto zt_block_try_after_20;

zt_block_try_after_20:
    zt_return_value = zt_outcome_void_core_error_success();
    goto zt_cleanup;

zt_cleanup:
    zt_outcome_i64_core_error_dispose(&__zt_try_result_0);
    zt_outcome_i64_core_error_dispose(&__zt_try_result_1);
    zt_generated_outcome_int_bool__dispose(&bool_total);
    zt_generated_outcome_text_bool__dispose(&bool_once);
    zt_generated_outcome_void_bool__dispose(&bool_void);
    zt_outcome_void_core_error_dispose(&__zt_try_result_2);
    zt_outcome_void_core_error_dispose(&__zt_try_result_3);
    zt_outcome_void_core_error_dispose(&__zt_try_result_4);
    zt_outcome_void_core_error_dispose(&__zt_try_result_5);
    zt_outcome_void_core_error_dispose(&__zt_try_result_6);
    return zt_return_value;
}

static zt_outcome_optional_text_core_error zt_app_main__io_read_line(zt_app_main__io_Input from) {
    goto zt_block_entry;

zt_block_entry:
    return zt_host_read_line_stdin();
}

static zt_outcome_text_core_error zt_app_main__io_read_all(zt_app_main__io_Input from) {
    goto zt_block_entry;

zt_block_entry:
    return zt_host_read_all_stdin();
}

static zt_outcome_void_core_error zt_app_main__io_write(zt_text *value, zt_app_main__io_Output to) {
    goto zt_block_entry;

zt_block_entry:
    if (((to.handle) == 2)) goto zt_block_if_then_0;
    goto zt_block_if_else_1;

zt_block_if_then_0:
    return zt_host_write_stderr(value);

zt_block_if_else_1:
    goto zt_block_if_join_2;

zt_block_if_join_2:
    return zt_host_write_stdout(value);
}

static zt_outcome_void_core_error zt_app_main__io_print(zt_text *value, zt_app_main__io_Output to) {
    goto zt_block_entry;

zt_block_entry:
    return zt_app_main__io_write(value, to);
}

int main(void) {
    zt_outcome_void_core_error __zt_main_result = zt_app_main__main();
    if (!__zt_main_result.is_success) {
        if (__zt_main_result.error.message != NULL) {
            (void)zt_host_write_stderr(__zt_main_result.error.message);
        }
        zt_outcome_void_core_error_dispose(&__zt_main_result);
        return 1;
    }
    zt_outcome_void_core_error_dispose(&__zt_main_result);
    return 0;
}