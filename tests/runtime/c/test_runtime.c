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

static int custom_host_read_calls = 0;
static int custom_host_stdout_calls = 0;
static int custom_host_stderr_calls = 0;

static zt_outcome_text_text *custom_host_read(const zt_text *path) {
    zt_text *value;
    zt_outcome_text_text *outcome;

    assert_text_equals("custom_host_read_path", path, "ignored.txt");
    custom_host_read_calls += 1;
    value = zt_text_from_utf8_literal("custom");
    outcome = zt_outcome_text_text_success(value);
    zt_release(value);
    return outcome;
}

static zt_outcome_void_text *custom_host_stdout(const zt_text *value) {
    zt_outcome_void_text *outcome;

    assert_text_equals("custom_host_stdout_value", value, "hello host");
    custom_host_stdout_calls += 1;
    outcome = zt_outcome_void_text_success();
    return outcome;
}

static zt_outcome_void_text *custom_host_stderr(const zt_text *value) {
    zt_outcome_void_text *outcome;

    assert_text_equals("custom_host_stderr_value", value, "hello host");
    custom_host_stderr_calls += 1;
    outcome = zt_outcome_void_text_success();
    return outcome;
}

static void test_text_from_utf8_and_len(void) {
    zt_text *hello = zt_text_from_utf8_literal("hello");

    assert_true("text_len", zt_text_len(hello) == 5);
    assert_text_equals("text_from_utf8_literal", hello, "hello");

    zt_release(hello);
}

static void test_bytes_basics(void) {
    const uint8_t raw[] = {0xDE, 0xAD, 0xBE, 0xEF};
    zt_bytes *value = zt_bytes_from_array(raw, 4);
    zt_bytes *slice = zt_bytes_slice(value, 1, 2);

    assert_true("bytes_len", zt_bytes_len(value) == 4);
    assert_true("bytes_get", zt_bytes_get(value, 2) == 0xBE);
    assert_true("bytes_slice_len", zt_bytes_len(slice) == 2);
    assert_true("bytes_slice_first", zt_bytes_get(slice, 0) == 0xAD);

    zt_release(slice);
    zt_release(value);
}

static void test_bytes_empty(void) {
    zt_bytes *value = zt_bytes_empty();

    assert_true("bytes_empty_len", zt_bytes_len(value) == 0);

    zt_release(value);
}

static void test_bytes_std_helpers(void) {
    const zt_int numbers[] = { 1, 2, 3 };
    const uint8_t suffix_raw[] = { 4, 5 };
    zt_list_i64 *list = zt_list_i64_from_array(numbers, sizeof(numbers) / sizeof(numbers[0]));
    zt_bytes *from_list = zt_bytes_from_list_i64(list);
    zt_list_i64 *back_to_list = zt_bytes_to_list_i64(from_list);
    zt_bytes *suffix = zt_bytes_from_array(suffix_raw, sizeof(suffix_raw));
    zt_bytes *joined = zt_bytes_join(from_list, suffix);
    zt_bytes *prefix = zt_bytes_from_array((const uint8_t[]){ 1, 2 }, 2);
    zt_bytes *good_suffix = zt_bytes_from_array((const uint8_t[]){ 4, 5 }, 2);
    zt_bytes *part = zt_bytes_from_array((const uint8_t[]){ 2, 3, 4 }, 3);
    zt_bytes *missing = zt_bytes_from_array((const uint8_t[]){ 9 }, 1);

    assert_true("bytes_from_list_len", zt_bytes_len(from_list) == 3);
    assert_true("bytes_from_list_second", zt_bytes_get(from_list, 1) == 2);

    assert_true("bytes_to_list_len", zt_list_i64_len(back_to_list) == 3);
    assert_true("bytes_to_list_third", zt_list_i64_get(back_to_list, 2) == 3);

    assert_true("bytes_join_len", zt_bytes_len(joined) == 5);
    assert_true("bytes_join_last", zt_bytes_get(joined, 4) == 5);

    assert_true("bytes_starts_with_true", zt_bytes_starts_with(joined, prefix));
    assert_true("bytes_ends_with_true", zt_bytes_ends_with(joined, good_suffix));
    assert_true("bytes_contains_true", zt_bytes_contains(joined, part));
    assert_true("bytes_contains_false", !zt_bytes_contains(joined, missing));

    zt_release(missing);
    zt_release(part);
    zt_release(good_suffix);
    zt_release(prefix);
    zt_release(joined);
    zt_release(suffix);
    zt_release(back_to_list);
    zt_release(from_list);
    zt_release(list);
}
static void test_text_utf8_bytes_roundtrip(void) {
    const char utf8_raw[] = { 'o', 'l', (char)0xC3, (char)0xA1 };
    zt_text *original = zt_text_from_utf8(utf8_raw, sizeof(utf8_raw));
    zt_bytes *encoded = zt_text_to_utf8_bytes(original);
    zt_outcome_text_text *decoded_outcome = zt_text_from_utf8_bytes(encoded);
    zt_text *decoded;

    assert_true("text_utf8_bytes_roundtrip_success", zt_outcome_text_text_is_success(decoded_outcome));
    decoded = zt_outcome_text_text_value(decoded_outcome);

    assert_true("text_utf8_bytes_roundtrip_len", zt_bytes_len(encoded) == zt_text_len(original));
    assert_true("text_utf8_bytes_roundtrip_eq", zt_text_eq(decoded, original));

    zt_release(decoded);
    zt_release(decoded_outcome);
    zt_release(encoded);
    zt_release(original);
}

static void test_text_from_utf8_bytes_invalid(void) {
    const uint8_t invalid[] = { 0xC3, 0x28 };
    zt_bytes *value = zt_bytes_from_array(invalid, sizeof(invalid));
    zt_outcome_text_text *decoded_outcome = zt_text_from_utf8_bytes(value);

    assert_true("text_from_utf8_bytes_invalid_failure", !zt_outcome_text_text_is_success(decoded_outcome));
    assert_true("text_from_utf8_bytes_invalid_error_message", strstr(zt_text_data(decoded_outcome->error), "invalid UTF-8") != NULL);

    zt_release(decoded_outcome);
    zt_release(value);
}

static void test_retain_and_release(void) {
    zt_text *value = zt_text_from_utf8_literal("rc");

    assert_true("retain_initial_rc", value->header.rc == 1);
    zt_retain(value);
    assert_true("retain_increment", value->header.rc == 2);
    zt_release(value);
    assert_true("release_decrement", value->header.rc == 1);
    zt_release(value);
}

static void test_concat_and_eq(void) {
    zt_text *left = zt_text_from_utf8_literal("hello");
    zt_text *space = zt_text_from_utf8_literal(" ");
    zt_text *right = zt_text_from_utf8_literal("world");
    zt_text *hello_space = zt_text_concat(left, space);
    zt_text *joined = zt_text_concat(hello_space, right);
    zt_text *expected = zt_text_from_utf8_literal("hello world");

    assert_text_equals("concat_text", joined, "hello world");
    assert_true("text_eq", zt_text_eq(joined, expected));
    assert_true("text_eq_false", !zt_text_eq(left, right));

    zt_release(expected);
    zt_release(joined);
    zt_release(hello_space);
    zt_release(right);
    zt_release(space);
    zt_release(left);
}

static void test_text_index(void) {
    zt_text *value = zt_text_from_utf8_literal("hello");
    zt_text *second = zt_text_index(value, 1);

    assert_text_equals("text_index", second, "e");

    zt_release(second);
    zt_release(value);
}

static void test_slice_semantics(void) {
    zt_text *value = zt_text_from_utf8_literal("hello world");
    zt_text *prefix = zt_text_slice(value, 0, 4);
    zt_text *suffix = zt_text_slice(value, 6, -1);
    zt_text *empty = zt_text_slice(value, 20, -1);

    assert_text_equals("slice_prefix", prefix, "hello");
    assert_text_equals("slice_suffix", suffix, "world");
    assert_text_equals("slice_empty", empty, "");
    assert_true("slice_empty_len", zt_text_len(empty) == 0);

    zt_release(empty);
    zt_release(suffix);
    zt_release(prefix);
    zt_release(value);
}

static void test_list_i64_basics(void) {
    zt_list_i64 *list = zt_list_i64_new();

    assert_true("list_new_rc", list->header.rc == 1);
    assert_true("list_new_len", zt_list_i64_len(list) == 0);

    zt_list_i64_push(list, 4);
    zt_list_i64_push(list, 7);
    zt_list_i64_push(list, 9);
    zt_list_i64_set(list, 1, 8);

    assert_true("list_len_after_push", zt_list_i64_len(list) == 3);
    assert_true("list_get_first", zt_list_i64_get(list, 0) == 4);
    assert_true("list_get_second", zt_list_i64_get(list, 1) == 8);
    assert_true("list_get_third", zt_list_i64_get(list, 2) == 9);

    zt_release(list);
}

static void test_list_i64_from_array_and_slice(void) {
    const zt_int items[] = { 4, 7, 9, 11 };
    zt_list_i64 *list = zt_list_i64_from_array(items, sizeof(items) / sizeof(items[0]));
    zt_list_i64 *slice = zt_list_i64_slice(list, 1, 2);
    zt_list_i64 *tail = zt_list_i64_slice(list, 2, -1);
    zt_list_i64 *empty = zt_list_i64_slice(list, 20, -1);

    assert_true("list_from_array_len", zt_list_i64_len(list) == 4);
    assert_true("list_slice_len", zt_list_i64_len(slice) == 2);
    assert_true("list_slice_first", zt_list_i64_get(slice, 0) == 7);
    assert_true("list_slice_second", zt_list_i64_get(slice, 1) == 9);
    assert_true("list_tail_len", zt_list_i64_len(tail) == 2);
    assert_true("list_tail_first", zt_list_i64_get(tail, 0) == 9);
    assert_true("list_tail_second", zt_list_i64_get(tail, 1) == 11);
    assert_true("list_empty_len", zt_list_i64_len(empty) == 0);

    zt_release(empty);
    zt_release(tail);
    zt_release(slice);
    zt_release(list);
}

static void test_list_i64_get_optional(void) {
    const zt_int items[] = { 4, 7, 9 };
    zt_list_i64 *list = zt_list_i64_from_array(items, sizeof(items) / sizeof(items[0]));
    zt_optional_i64 present = zt_list_i64_get_optional(list, 1);
    zt_optional_i64 missing = zt_list_i64_get_optional(list, 99);

    assert_true("list_i64_get_optional_present", zt_optional_i64_is_present(present));
    assert_true("list_i64_get_optional_present_value", zt_optional_i64_coalesce(present, -1) == 7);
    assert_true("list_i64_get_optional_missing", !zt_optional_i64_is_present(missing));
    assert_true("list_i64_get_optional_missing_fallback", zt_optional_i64_coalesce(missing, 123) == 123);

    zt_release(list);
}

static void test_list_i64_set_owned_copy_on_write(void) {
    const zt_int items[] = { 1, 2, 3 };
    zt_list_i64 *base = zt_list_i64_from_array(items, sizeof(items) / sizeof(items[0]));
    zt_list_i64 *alias = base;
    zt_list_i64 *mutated;

    zt_retain(alias);
    mutated = zt_list_i64_set_owned(alias, 1, 99);

    assert_true("list_i64_set_owned_clone_ptr", mutated != base);
    assert_true("list_i64_set_owned_base_unchanged", zt_list_i64_get(base, 1) == 2);
    assert_true("list_i64_set_owned_mutated_value", zt_list_i64_get(mutated, 1) == 99);

    zt_release(mutated);
    zt_release(base);
}

static void test_list_text_set_owned_copy_on_write(void) {
    zt_text *alpha = zt_text_from_utf8_literal("alpha");
    zt_text *beta = zt_text_from_utf8_literal("beta");
    zt_text *omega = zt_text_from_utf8_literal("omega");
    zt_text *items[] = { alpha, beta };
    zt_list_text *base = zt_list_text_from_array(items, 2);
    zt_list_text *alias = base;
    zt_list_text *mutated;
    zt_text *base_first;
    zt_text *mutated_first;

    zt_retain(alias);
    mutated = zt_list_text_set_owned(alias, 0, omega);

    assert_true("list_text_set_owned_clone_ptr", mutated != base);

    base_first = zt_list_text_get(base, 0);
    mutated_first = zt_list_text_get(mutated, 0);
    assert_text_equals("list_text_set_owned_base_unchanged", base_first, "alpha");
    assert_text_equals("list_text_set_owned_mutated_value", mutated_first, "omega");

    zt_release(mutated_first);
    zt_release(base_first);
    zt_release(mutated);
    zt_release(base);
    zt_release(omega);
    zt_release(beta);
    zt_release(alpha);
}
static void test_list_text_basics(void) {
    zt_text *alpha = zt_text_from_utf8_literal("alpha");
    zt_text *beta = zt_text_from_utf8_literal("beta");
    zt_text *gamma = zt_text_from_utf8_literal("gamma");
    zt_text *items[] = { alpha, beta };
    zt_list_text *list = zt_list_text_from_array(items, 2);
    zt_text *picked = zt_list_text_get(list, 1);
    zt_text *first_after_set;
    zt_list_text *slice;
    zt_text *slice_first;
    zt_text *slice_second;

    assert_true("list_text_len", zt_list_text_len(list) == 2);
    assert_text_equals("list_text_get", picked, "beta");

    zt_list_text_set(list, 0, gamma);
    zt_list_text_push(list, gamma);
    slice = zt_list_text_slice(list, 0, 1);
    first_after_set = zt_list_text_get(list, 0);
    slice_first = zt_list_text_get(slice, 0);
    slice_second = zt_list_text_get(slice, 1);

    assert_true("list_text_set_len", zt_list_text_len(list) == 3);
    assert_text_equals("list_text_first_after_set", first_after_set, "gamma");
    assert_true("list_text_slice_len", zt_list_text_len(slice) == 2);
    assert_text_equals("list_text_slice_first", slice_first, "gamma");
    assert_text_equals("list_text_slice_second", slice_second, "beta");

    zt_release(slice_second);
    zt_release(slice_first);
    zt_release(first_after_set);
    zt_release(slice);
    zt_release(picked);
    zt_release(list);
    zt_release(gamma);
    zt_release(beta);
    zt_release(alpha);
}

static void test_list_text_get_optional(void) {
    zt_text *alpha = zt_text_from_utf8_literal("alpha");
    zt_text *beta = zt_text_from_utf8_literal("beta");
    zt_text *fallback = zt_text_from_utf8_literal("fallback");
    zt_text *items[] = { alpha, beta };
    zt_list_text *list = zt_list_text_from_array(items, 2);
    zt_optional_text *present = zt_list_text_get_optional(list, 0);
    zt_optional_text *missing = zt_list_text_get_optional(list, 99);
    zt_text *present_value = zt_optional_text_coalesce(present, fallback);
    zt_text *missing_value = zt_optional_text_coalesce(missing, fallback);

    assert_true("list_text_get_optional_present", zt_optional_text_is_present(present));
    assert_true("list_text_get_optional_missing", !zt_optional_text_is_present(missing));
    assert_text_equals("list_text_get_optional_present_value", present_value, "alpha");
    assert_text_equals("list_text_get_optional_missing_fallback", missing_value, "fallback");

    zt_release(missing_value);
    zt_release(present_value);
    zt_release(missing);
    zt_release(present);
    zt_release(list);
    zt_release(fallback);
    zt_release(beta);
    zt_release(alpha);
}

static void test_map_text_text_basics(void) {
    zt_text *key_mode = zt_text_from_utf8_literal("mode");
    zt_text *key_status = zt_text_from_utf8_literal("status");
    zt_text *key_profile = zt_text_from_utf8_literal("profile");
    zt_text *value_debug = zt_text_from_utf8_literal("debug");
    zt_text *value_ok = zt_text_from_utf8_literal("ok");
    zt_text *value_release = zt_text_from_utf8_literal("release");
    zt_text *value_dev = zt_text_from_utf8_literal("dev");
    zt_text *keys[] = { key_mode, key_status };
    zt_text *values[] = { value_debug, value_ok };
    zt_map_text_text *map = zt_map_text_text_from_arrays(keys, values, 2);
    zt_text *picked_mode = zt_map_text_text_get(map, key_mode);
    zt_text *picked_profile;

    assert_true("map_new_len", zt_map_text_text_len(map) == 2);
    assert_text_equals("map_get_mode", picked_mode, "debug");
    zt_release(picked_mode);

    zt_map_text_text_set(map, key_mode, value_release);
    zt_map_text_text_set(map, key_profile, value_dev);
    picked_mode = zt_map_text_text_get(map, key_mode);
    picked_profile = zt_map_text_text_get(map, key_profile);

    assert_true("map_len_after_updates", zt_map_text_text_len(map) == 3);
    assert_text_equals("map_get_updated_mode", picked_mode, "release");
    assert_text_equals("map_get_profile", picked_profile, "dev");
    assert_true("map_key_profile_rc", key_profile->header.rc == 2);
    assert_true("map_value_dev_rc", value_dev->header.rc == 3);

    zt_release(picked_profile);
    zt_release(picked_mode);
    zt_release(map);
    zt_release(value_dev);
    zt_release(value_release);
    zt_release(value_ok);
    zt_release(value_debug);
    zt_release(key_profile);
    zt_release(key_status);
    zt_release(key_mode);
}

static void test_map_text_text_set_owned_copy_on_write(void) {
    zt_text *key_mode = zt_text_from_utf8_literal("mode");
    zt_text *value_debug = zt_text_from_utf8_literal("debug");
    zt_text *value_release = zt_text_from_utf8_literal("release");
    zt_text *keys[] = { key_mode };
    zt_text *values[] = { value_debug };
    zt_map_text_text *base = zt_map_text_text_from_arrays(keys, values, 1);
    zt_map_text_text *alias = base;
    zt_map_text_text *mutated;
    zt_text *base_mode;
    zt_text *mutated_mode;

    zt_retain(alias);
    mutated = zt_map_text_text_set_owned(alias, key_mode, value_release);

    assert_true("map_set_owned_clone_ptr", mutated != base);

    base_mode = zt_map_text_text_get(base, key_mode);
    mutated_mode = zt_map_text_text_get(mutated, key_mode);
    assert_text_equals("map_set_owned_base_unchanged", base_mode, "debug");
    assert_text_equals("map_set_owned_mutated_value", mutated_mode, "release");

    zt_release(mutated_mode);
    zt_release(base_mode);
    zt_release(mutated);
    zt_release(base);
    zt_release(value_release);
    zt_release(value_debug);
    zt_release(key_mode);
}
static void test_optional_i64(void) {
    zt_optional_i64 present = zt_optional_i64_present(7);
    zt_optional_i64 empty = zt_optional_i64_empty();

    assert_true("optional_present_flag", zt_optional_i64_is_present(present));
    assert_true("optional_empty_flag", !zt_optional_i64_is_present(empty));
    assert_true("optional_present_value", zt_optional_i64_coalesce(present, 99) == 7);
    assert_true("optional_empty_fallback", zt_optional_i64_coalesce(empty, 99) == 99);
}

static void test_optional_text(void) {
    zt_text *present_value = zt_text_from_utf8_literal("hello");
    zt_text *fallback = zt_text_from_utf8_literal("fallback");
    zt_optional_text *present = zt_optional_text_present(present_value);
    zt_optional_text *empty = zt_optional_text_empty();
    zt_text *picked_present = zt_optional_text_coalesce(present, fallback);
    zt_text *picked_empty = zt_optional_text_coalesce(empty, fallback);

    assert_true("optional_text_present_flag", zt_optional_text_is_present(present));
    assert_true("optional_text_empty_flag", !zt_optional_text_is_present(empty));
    assert_text_equals("optional_text_present_value", picked_present, "hello");
    assert_text_equals("optional_text_empty_fallback", picked_empty, "fallback");
    assert_true("optional_text_present_rc", present_value->header.rc == 3);
    assert_true("optional_text_fallback_rc", fallback->header.rc == 2);

    zt_release(picked_empty);
    zt_release(picked_present);
    zt_release(empty);
    zt_release(present);
    zt_release(fallback);
    zt_release(present_value);
}

static void test_optional_list_i64(void) {
    const zt_int present_items[] = { 4, 7 };
    const zt_int fallback_items[] = { 9, 11, 13 };
    zt_list_i64 *present_value = zt_list_i64_from_array(present_items, sizeof(present_items) / sizeof(present_items[0]));
    zt_list_i64 *fallback = zt_list_i64_from_array(fallback_items, sizeof(fallback_items) / sizeof(fallback_items[0]));
    zt_optional_list_i64 *present = zt_optional_list_i64_present(present_value);
    zt_optional_list_i64 *empty = zt_optional_list_i64_empty();
    zt_list_i64 *picked_present = zt_optional_list_i64_coalesce(present, fallback);
    zt_list_i64 *picked_empty = zt_optional_list_i64_coalesce(empty, fallback);

    assert_true("optional_list_present_flag", zt_optional_list_i64_is_present(present));
    assert_true("optional_list_empty_flag", !zt_optional_list_i64_is_present(empty));
    assert_true("optional_list_present_len", zt_list_i64_len(picked_present) == 2);
    assert_true("optional_list_present_second", zt_list_i64_get(picked_present, 1) == 7);
    assert_true("optional_list_empty_len", zt_list_i64_len(picked_empty) == 3);
    assert_true("optional_list_empty_first", zt_list_i64_get(picked_empty, 0) == 9);
    assert_true("optional_list_present_rc", present_value->header.rc == 3);
    assert_true("optional_list_fallback_rc", fallback->header.rc == 2);

    zt_release(picked_empty);
    zt_release(picked_present);
    zt_release(empty);
    zt_release(present);
    zt_release(fallback);
    zt_release(present_value);
}

static void test_optional_list_i64_copy_mutate_value_semantics(void) {
    const zt_int items[] = { 1, 2, 3 };
    zt_list_i64 *base_list = zt_list_i64_from_array(items, sizeof(items) / sizeof(items[0]));
    zt_list_i64 *fallback = zt_list_i64_new();
    zt_optional_list_i64 *base = zt_optional_list_i64_present(base_list);
    zt_optional_list_i64 *alias = base;
    zt_list_i64 *alias_value;
    zt_list_i64 *mutated_value;
    zt_list_i64 *base_value;
    zt_list_i64 *alias_value_after;

    zt_retain(alias);

    alias_value = zt_optional_list_i64_coalesce(alias, fallback);
    mutated_value = zt_list_i64_set_owned(alias_value, 1, 99);

    zt_release(alias);
    alias = zt_optional_list_i64_present(mutated_value);
    zt_release(mutated_value);

    base_value = zt_optional_list_i64_coalesce(base, fallback);
    alias_value_after = zt_optional_list_i64_coalesce(alias, fallback);

    assert_true("optional_list_copy_mutate_base_unchanged", zt_list_i64_get(base_value, 1) == 2);
    assert_true("optional_list_copy_mutate_alias_changed", zt_list_i64_get(alias_value_after, 1) == 99);

    zt_release(alias_value_after);
    zt_release(base_value);
    zt_release(alias);
    zt_release(base);
    zt_release(fallback);
    zt_release(base_list);
}

static void test_outcome_i64_text(void) {
    zt_text *error = zt_text_from_utf8_literal("bad");
    zt_outcome_i64_text *success = zt_outcome_i64_text_success(42);
    zt_outcome_i64_text *failure = zt_outcome_i64_text_failure(error);
    zt_outcome_i64_text *propagated = zt_outcome_i64_text_propagate(failure);

    assert_true("outcome_success_flag", zt_outcome_i64_text_is_success(success));
    assert_true("outcome_failure_flag", !zt_outcome_i64_text_is_success(failure));
    assert_true("outcome_success_value", zt_outcome_i64_text_value(success) == 42);
    assert_true("outcome_failure_rc_after_propagate", failure->header.rc == 2);
    assert_text_equals("outcome_failure_error_text", failure->error, "bad");

    zt_release(propagated);
    zt_release(failure);
    zt_release(success);
    zt_release(error);
}

static void test_outcome_void_text(void) {
    zt_text *error = zt_text_from_utf8_literal("oops");
    zt_outcome_void_text *success = zt_outcome_void_text_success();
    zt_outcome_void_text *success_again = zt_outcome_void_text_success();
    zt_outcome_void_text *failure = zt_outcome_void_text_failure(error);
    zt_outcome_void_text *propagated = zt_outcome_void_text_propagate(failure);

    assert_true("outcome_void_success_flag", zt_outcome_void_text_is_success(success));
    assert_true("outcome_void_success_singleton_ptr", success == success_again);
    assert_true("outcome_void_success_singleton_kind", success->header.kind == (uint32_t)ZT_HEAP_IMMORTAL_OUTCOME_VOID_TEXT);
    assert_true("outcome_void_success_singleton_rc", success->header.rc == 1u);
    assert_true("outcome_void_failure_flag", !zt_outcome_void_text_is_success(failure));
    assert_true("outcome_void_failure_rc_after_propagate", failure->header.rc == 2);
    assert_text_equals("outcome_void_failure_error_text", failure->error, "oops");

    zt_release(propagated);
    zt_release(failure);
    zt_release(success);
    zt_release(error);
}

static void test_outcome_text_text(void) {
    zt_text *value = zt_text_from_utf8_literal("done");
    zt_text *error = zt_text_from_utf8_literal("bad");
    zt_outcome_text_text *success = zt_outcome_text_text_success(value);
    zt_outcome_text_text *failure = zt_outcome_text_text_failure(error);
    zt_text *unwrapped = zt_outcome_text_text_value(success);
    zt_outcome_text_text *propagated = zt_outcome_text_text_propagate(failure);

    assert_true("outcome_text_success_flag", zt_outcome_text_text_is_success(success));
    assert_true("outcome_text_failure_flag", !zt_outcome_text_text_is_success(failure));
    assert_text_equals("outcome_text_success_value", unwrapped, "done");
    assert_true("outcome_text_success_value_rc", value->header.rc == 3);
    assert_true("outcome_text_failure_rc_after_propagate", failure->header.rc == 2);
    assert_text_equals("outcome_text_failure_error_text", failure->error, "bad");

    zt_release(propagated);
    zt_release(unwrapped);
    zt_release(failure);
    zt_release(success);
    zt_release(error);
    zt_release(value);
}

static void test_optional_list_text(void) {
    zt_text *alpha = zt_text_from_utf8_literal("alpha");
    zt_text *beta = zt_text_from_utf8_literal("beta");
    zt_text *gamma = zt_text_from_utf8_literal("gamma");
    zt_text *items_present[] = { alpha, beta };
    zt_text *items_fallback[] = { gamma };
    zt_list_text *present_value = zt_list_text_from_array(items_present, 2);
    zt_list_text *fallback = zt_list_text_from_array(items_fallback, 1);
    zt_optional_list_text *present = zt_optional_list_text_present(present_value);
    zt_optional_list_text *empty = zt_optional_list_text_empty();
    zt_list_text *picked_present = zt_optional_list_text_coalesce(present, fallback);
    zt_list_text *picked_empty = zt_optional_list_text_coalesce(empty, fallback);
    zt_text *picked_present_first = zt_list_text_get(picked_present, 0);
    zt_text *picked_empty_first = zt_list_text_get(picked_empty, 0);

    assert_true("optional_list_text_present_flag", zt_optional_list_text_is_present(present));
    assert_true("optional_list_text_empty_flag", !zt_optional_list_text_is_present(empty));
    assert_text_equals("optional_list_text_present_first", picked_present_first, "alpha");
    assert_text_equals("optional_list_text_empty_first", picked_empty_first, "gamma");
    assert_true("optional_list_text_present_rc", present_value->header.rc == 3);
    assert_true("optional_list_text_fallback_rc", fallback->header.rc == 2);

    zt_release(picked_empty_first);
    zt_release(picked_present_first);
    zt_release(picked_empty);
    zt_release(picked_present);
    zt_release(empty);
    zt_release(present);
    zt_release(fallback);
    zt_release(present_value);
    zt_release(gamma);
    zt_release(beta);
    zt_release(alpha);
}

static void test_optional_map_text_text(void) {
    zt_text *key_mode = zt_text_from_utf8_literal("mode");
    zt_text *key_theme = zt_text_from_utf8_literal("theme");
    zt_text *value_debug = zt_text_from_utf8_literal("debug");
    zt_text *value_paper = zt_text_from_utf8_literal("paper");
    zt_text *keys_present[] = { key_mode };
    zt_text *values_present[] = { value_debug };
    zt_text *keys_fallback[] = { key_theme };
    zt_text *values_fallback[] = { value_paper };
    zt_map_text_text *present_value = zt_map_text_text_from_arrays(keys_present, values_present, 1);
    zt_map_text_text *fallback = zt_map_text_text_from_arrays(keys_fallback, values_fallback, 1);
    zt_optional_map_text_text *present = zt_optional_map_text_text_present(present_value);
    zt_optional_map_text_text *empty = zt_optional_map_text_text_empty();
    zt_map_text_text *picked_present = zt_optional_map_text_text_coalesce(present, fallback);
    zt_map_text_text *picked_empty = zt_optional_map_text_text_coalesce(empty, fallback);
    zt_text *picked_present_value = zt_map_text_text_get(picked_present, key_mode);
    zt_text *picked_empty_value = zt_map_text_text_get(picked_empty, key_theme);

    assert_true("optional_map_present_flag", zt_optional_map_text_text_is_present(present));
    assert_true("optional_map_empty_flag", !zt_optional_map_text_text_is_present(empty));
    assert_text_equals("optional_map_present_value", picked_present_value, "debug");
    assert_text_equals("optional_map_empty_value", picked_empty_value, "paper");
    assert_true("optional_map_present_rc", present_value->header.rc == 3);
    assert_true("optional_map_fallback_rc", fallback->header.rc == 2);

    zt_release(picked_empty_value);
    zt_release(picked_present_value);
    zt_release(picked_empty);
    zt_release(picked_present);
    zt_release(empty);
    zt_release(present);
    zt_release(fallback);
    zt_release(present_value);
    zt_release(value_paper);
    zt_release(value_debug);
    zt_release(key_theme);
    zt_release(key_mode);
}

static void test_outcome_list_i64_text(void) {
    const zt_int items[] = { 4, 7, 9 };
    zt_list_i64 *value = zt_list_i64_from_array(items, sizeof(items) / sizeof(items[0]));
    zt_text *error = zt_text_from_utf8_literal("bad");
    zt_outcome_list_i64_text *success = zt_outcome_list_i64_text_success(value);
    zt_outcome_list_i64_text *failure = zt_outcome_list_i64_text_failure(error);
    zt_list_i64 *unwrapped = zt_outcome_list_i64_text_value(success);
    zt_outcome_list_i64_text *propagated = zt_outcome_list_i64_text_propagate(failure);

    assert_true("outcome_list_i64_success_flag", zt_outcome_list_i64_text_is_success(success));
    assert_true("outcome_list_i64_failure_flag", !zt_outcome_list_i64_text_is_success(failure));
    assert_true("outcome_list_i64_len", zt_list_i64_len(unwrapped) == 3);
    assert_true("outcome_list_i64_second", zt_list_i64_get(unwrapped, 1) == 7);
    assert_true("outcome_list_i64_value_rc", value->header.rc == 3);
    assert_true("outcome_list_i64_failure_rc_after_propagate", failure->header.rc == 2);

    zt_release(propagated);
    zt_release(unwrapped);
    zt_release(failure);
    zt_release(success);
    zt_release(error);
    zt_release(value);
}

static void test_outcome_list_i64_text_copy_mutate_value_semantics(void) {
    const zt_int items[] = { 1, 2, 3 };
    zt_list_i64 *base_list = zt_list_i64_from_array(items, sizeof(items) / sizeof(items[0]));
    zt_text *error = zt_text_from_utf8_literal("bad");
    zt_outcome_list_i64_text *base = zt_outcome_list_i64_text_success(base_list);
    zt_outcome_list_i64_text *alias = base;
    zt_list_i64 *alias_value;
    zt_list_i64 *mutated_value;
    zt_list_i64 *base_value;
    zt_list_i64 *alias_value_after;

    zt_retain(alias);

    alias_value = zt_outcome_list_i64_text_value(alias);
    mutated_value = zt_list_i64_set_owned(alias_value, 1, 77);

    zt_release(alias);
    alias = zt_outcome_list_i64_text_success(mutated_value);
    zt_release(mutated_value);

    base_value = zt_outcome_list_i64_text_value(base);
    alias_value_after = zt_outcome_list_i64_text_value(alias);

    assert_true("outcome_list_copy_mutate_base_unchanged", zt_list_i64_get(base_value, 1) == 2);
    assert_true("outcome_list_copy_mutate_alias_changed", zt_list_i64_get(alias_value_after, 1) == 77);

    zt_release(alias_value_after);
    zt_release(base_value);
    zt_release(alias);
    zt_release(base);
    zt_release(error);
    zt_release(base_list);
}

static void test_outcome_list_text_text(void) {
    zt_text *alpha = zt_text_from_utf8_literal("alpha");
    zt_text *beta = zt_text_from_utf8_literal("beta");
    zt_text *error = zt_text_from_utf8_literal("bad");
    zt_text *items[] = { alpha, beta };
    zt_list_text *value = zt_list_text_from_array(items, 2);
    zt_outcome_list_text_text *success = zt_outcome_list_text_text_success(value);
    zt_outcome_list_text_text *failure = zt_outcome_list_text_text_failure(error);
    zt_list_text *unwrapped = zt_outcome_list_text_text_value(success);
    zt_text *first = zt_list_text_get(unwrapped, 0);
    zt_outcome_list_text_text *propagated = zt_outcome_list_text_text_propagate(failure);

    assert_true("outcome_list_text_success_flag", zt_outcome_list_text_text_is_success(success));
    assert_true("outcome_list_text_failure_flag", !zt_outcome_list_text_text_is_success(failure));
    assert_text_equals("outcome_list_text_first", first, "alpha");
    assert_true("outcome_list_text_value_rc", value->header.rc == 3);
    assert_true("outcome_list_text_failure_rc_after_propagate", failure->header.rc == 2);

    zt_release(propagated);
    zt_release(first);
    zt_release(unwrapped);
    zt_release(failure);
    zt_release(success);
    zt_release(value);
    zt_release(error);
    zt_release(beta);
    zt_release(alpha);
}

static void test_outcome_map_text_text(void) {
    zt_text *key = zt_text_from_utf8_literal("mode");
    zt_text *value_text = zt_text_from_utf8_literal("debug");
    zt_text *error = zt_text_from_utf8_literal("bad");
    zt_text *keys[] = { key };
    zt_text *values[] = { value_text };
    zt_map_text_text *value = zt_map_text_text_from_arrays(keys, values, 1);
    zt_outcome_map_text_text *success = zt_outcome_map_text_text_success(value);
    zt_outcome_map_text_text *failure = zt_outcome_map_text_text_failure(error);
    zt_map_text_text *unwrapped = zt_outcome_map_text_text_value(success);
    zt_text *picked = zt_map_text_text_get(unwrapped, key);
    zt_outcome_map_text_text *propagated = zt_outcome_map_text_text_propagate(failure);

    assert_true("outcome_map_success_flag", zt_outcome_map_text_text_is_success(success));
    assert_true("outcome_map_failure_flag", !zt_outcome_map_text_text_is_success(failure));
    assert_text_equals("outcome_map_value", picked, "debug");
    assert_true("outcome_map_value_rc", value->header.rc == 3);
    assert_true("outcome_map_failure_rc_after_propagate", failure->header.rc == 2);

    zt_release(propagated);
    zt_release(picked);
    zt_release(unwrapped);
    zt_release(failure);
    zt_release(success);
    zt_release(value);
    zt_release(error);
    zt_release(value_text);
    zt_release(key);
}

static void test_runtime_error_reporting(void) {
    const zt_runtime_error_info *error;

    zt_runtime_clear_error();
    assert_true("runtime_error_clear_flag", !zt_runtime_last_error()->has_error);

    zt_runtime_report_error(
        ZT_ERR_IO,
        "missing file",
        "ZT-IO001",
        zt_runtime_make_span("sample.zt", 9, 2)
    );
    error = zt_runtime_last_error();

    assert_true("runtime_error_has_error", error->has_error);
    assert_true("runtime_error_kind", error->kind == ZT_ERR_IO);
    assert_true("runtime_error_message", strcmp(error->message, "missing file") == 0);
    assert_true("runtime_error_code", strcmp(error->code, "ZT-IO001") == 0);
    assert_true("runtime_error_span_known", zt_runtime_span_is_known(error->span));
    assert_true("runtime_error_span_line", error->span.line == 9);
    assert_true("runtime_error_span_column", error->span.column == 2);
    assert_true("runtime_error_span_source", strcmp(error->span.source_name, "sample.zt") == 0);

    zt_runtime_clear_error();
    assert_true("runtime_error_clear_after_report", !zt_runtime_last_error()->has_error);
}

static void test_host_read_file_default(void) {
    FILE *file;
    zt_text *path = zt_text_from_utf8_literal(".ztc-tmp/runtime-host-read.txt");
    zt_outcome_text_text *result;
    zt_text *contents;

    file = fopen(".ztc-tmp/runtime-host-read.txt", "wb");
    if (file == NULL) {
        fprintf(stderr, "falha ao criar fixture de host read\n");
        exit(1);
    }

    fputs("hello from host", file);
    fclose(file);

    zt_host_set_api(NULL);
    result = zt_host_read_file(path);
    assert_true("host_read_default_success", zt_outcome_text_text_is_success(result));
    contents = zt_outcome_text_text_value(result);
    assert_text_equals("host_read_default_contents", contents, "hello from host");

    zt_release(contents);
    zt_release(result);
    zt_release(path);
}

static void test_host_override_dispatch(void) {
    zt_host_api api;
    zt_text *path = zt_text_from_utf8_literal("ignored.txt");
    zt_text *message = zt_text_from_utf8_literal("hello host");
    zt_outcome_text_text *read_result;
    zt_text *read_value;
    zt_outcome_void_text *stdout_result;
    zt_outcome_void_text *stderr_result;

    custom_host_read_calls = 0;
    custom_host_stdout_calls = 0;
    custom_host_stderr_calls = 0;

    api.read_file = custom_host_read;
    api.write_stdout = custom_host_stdout;
    api.write_stderr = custom_host_stderr;
    zt_host_set_api(&api);

    read_result = zt_host_read_file(path);
    read_value = zt_outcome_text_text_value(read_result);
    stdout_result = zt_host_write_stdout(message);
    stderr_result = zt_host_write_stderr(message);

    assert_text_equals("host_override_read_value", read_value, "custom");
    assert_true("host_override_read_calls", custom_host_read_calls == 1);
    assert_true("host_override_stdout_calls", custom_host_stdout_calls == 1);
    assert_true("host_override_stderr_calls", custom_host_stderr_calls == 1);
    assert_true("host_override_stdout_success", zt_outcome_void_text_is_success(stdout_result));
    assert_true("host_override_stderr_success", zt_outcome_void_text_is_success(stderr_result));

    zt_release(stderr_result);
    zt_release(stdout_result);
    zt_release(read_value);
    zt_release(read_result);
    zt_release(message);
    zt_release(path);
    zt_host_set_api(NULL);
}

static void test_error_kind_names(void) {
    assert_true("error_kind_assert", strcmp(zt_error_kind_name(ZT_ERR_ASSERT), "assert") == 0);
    assert_true("error_kind_check", strcmp(zt_error_kind_name(ZT_ERR_CHECK), "check") == 0);
    assert_true("error_kind_panic", strcmp(zt_error_kind_name(ZT_ERR_PANIC), "panic") == 0);
    assert_true("error_kind_unwrap", strcmp(zt_error_kind_name(ZT_ERR_UNWRAP), "unwrap") == 0);
}

static void test_assert_and_check_success(void) {
    zt_assert(true, "should not fail");
    zt_check(true, "should not fail");
}

int main(void) {
    test_text_from_utf8_and_len();
    test_bytes_basics();
    test_bytes_empty();
    test_bytes_std_helpers();
    test_text_utf8_bytes_roundtrip();
    test_text_from_utf8_bytes_invalid();
    test_retain_and_release();
    test_concat_and_eq();
    test_text_index();
    test_slice_semantics();
    test_list_i64_basics();
    test_list_i64_from_array_and_slice();
    test_list_i64_get_optional();
    test_list_i64_set_owned_copy_on_write();
    test_list_text_set_owned_copy_on_write();
    test_list_text_basics();
    test_list_text_get_optional();
    test_map_text_text_basics();
    test_map_text_text_set_owned_copy_on_write();
    test_optional_i64();
    test_optional_text();
    test_optional_list_i64();
    test_optional_list_i64_copy_mutate_value_semantics();
    test_optional_list_text();
    test_optional_map_text_text();
    test_outcome_i64_text();
    test_outcome_void_text();
    test_outcome_text_text();
    test_outcome_list_i64_text();
    test_outcome_list_i64_text_copy_mutate_value_semantics();
    test_outcome_list_text_text();
    test_outcome_map_text_text();
    test_runtime_error_reporting();
    test_host_read_file_default();
    test_host_override_dispatch();
    test_error_kind_names();
    test_assert_and_check_success();
    puts("Runtime C tests OK");
    return 0;
}



