import os

header_path = 'runtime/c/zenith_rt.h'
with open(header_path, 'r', encoding='utf-8') as f:
    content = f.read()

# Fix header file
good_header_tail = """zt_outcome_map_text_text zt_outcome_map_text_text_propagate(zt_outcome_map_text_text outcome);

zt_int zt_add_i64(zt_int a, zt_int b);
zt_int zt_sub_i64(zt_int a, zt_int b);
zt_int zt_mul_i64(zt_int a, zt_int b);
zt_int zt_div_i64(zt_int a, zt_int b);
zt_int zt_rem_i64(zt_int a, zt_int b);

#ifdef __cplusplus
}
#endif

#endif
"""

# replace everything from zt_outcome_map_text_text zt_outcome_map_text_text_propagate to the end
idx = content.find("zt_outcome_map_text_text zt_outcome_map_text_text_propagate(zt_outcome_map_text_text outcome);")
if idx != -1:
    content = content[:idx] + good_header_tail
    with open(header_path, 'w', encoding='utf-8') as f:
        f.write(content)

c_path = 'runtime/c/zenith_rt.c'
with open(c_path, 'r', encoding='utf-8') as f:
    c_content = f.read()

# Fix C file
idx2 = c_content.find("zt_outcome_void_text zt_host_write_stderr(const zt_text *value) {")

if idx2 != -1:
    idx3 = c_content.find("}", idx2)
    if idx3 != -1:
        good_c_tail = """

zt_int zt_add_i64(zt_int a, zt_int b) {
    zt_int result;
    if (__builtin_add_overflow(a, b, &result)) {
        zt_runtime_error(ZT_ERR_MATH, "arithmetic overflow");
    }
    return result;
}

zt_int zt_sub_i64(zt_int a, zt_int b) {
    zt_int result;
    if (__builtin_sub_overflow(a, b, &result)) {
        zt_runtime_error(ZT_ERR_MATH, "arithmetic overflow");
    }
    return result;
}

zt_int zt_mul_i64(zt_int a, zt_int b) {
    zt_int result;
    if (__builtin_mul_overflow(a, b, &result)) {
        zt_runtime_error(ZT_ERR_MATH, "arithmetic overflow");
    }
    return result;
}

zt_int zt_div_i64(zt_int a, zt_int b) {
    if (b == 0) {
        zt_runtime_error(ZT_ERR_MATH, "division by zero");
    }
    if (a == INT64_MIN && b == -1) {
        zt_runtime_error(ZT_ERR_MATH, "arithmetic overflow");
    }
    return a / b;
}

zt_int zt_rem_i64(zt_int a, zt_int b) {
    if (b == 0) {
        zt_runtime_error(ZT_ERR_MATH, "division by zero");
    }
    if (a == INT64_MIN && b == -1) {
        zt_runtime_error(ZT_ERR_MATH, "arithmetic overflow");
    }
    return a % b;
}
"""
        c_content = c_content[:idx3+1] + good_c_tail
        with open(c_path, 'w', encoding='utf-8') as f:
            f.write(c_content)

print("done")
