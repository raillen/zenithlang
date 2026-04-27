/*
 * pipeline.c - Compilation pipeline for the Zenith compiler driver.
 *
 * Responsibilities:
 *   - Project compilation (parse -> bind -> check -> HIR -> ZIR -> verify)
 *   - C code emission and native compilation
 *   - Monomorphization limit enforcement
 *   - Executable running and process management
 *   - Project source collection
 */

#include "compiler/driver/driver_internal.h"
#include "compiler/frontend/lexer/lexer.h"

static void zt_runtime_root(char *buffer, size_t capacity);
static int zt_home_has_stdlib(const char *home_root);

static int zt_diagnostics_profile_is_strict(const zt_project_manifest *manifest) {
    return manifest != NULL && strcmp(manifest->diag_profile, "strict") == 0;
}

static int zt_handle_stage_diagnostics(
        zt_driver_context *ctx,
        const zt_project_manifest *manifest,
        const char *stage,
        zt_diag_list *diagnostics) {
    if (diagnostics == NULL || diagnostics->count == 0) return 1;

    if (zt_diagnostics_profile_is_strict(manifest)) {
        zt_diag_list_promote_warnings(diagnostics);
    }

    zt_print_diagnostics(ctx, stage, diagnostics);
    return !zt_diag_list_has_errors(diagnostics);
}

/*  String set (local utility for generic instance tracking)  */

typedef struct zt_string_set {
    char **items;
    size_t count;
    size_t capacity;
} zt_string_set;

static void zt_string_set_init(zt_string_set *set) {
    if (set == NULL) return;
    set->items = NULL;
    set->count = 0;
    set->capacity = 0;
}

static void zt_string_set_dispose(zt_string_set *set) {
    size_t i;
    if (set == NULL) return;
    for (i = 0; i < set->count; i += 1) {
        free(set->items[i]);
    }
    free(set->items);
    set->items = NULL;
    set->count = 0;
    set->capacity = 0;
}

static int zt_string_set_add(zt_string_set *set, const char *value) {
    size_t i;
    char *copy;

    if (set == NULL || value == NULL || value[0] == '\0') return 1;

    for (i = 0; i < set->count; i += 1) {
        if (strcmp(set->items[i], value) == 0) return 1;
    }

    if (set->count == set->capacity) {
        size_t new_capacity = set->capacity == 0 ? 8 : set->capacity * 2;
        char **new_items = (char **)realloc(set->items, new_capacity * sizeof(char *));
        if (new_items == NULL) return 0;
        set->items = new_items;
        set->capacity = new_capacity;
    }

    copy = zt_heap_strdup(value);
    if (copy == NULL) return 0;

    set->items[set->count] = copy;
    set->count += 1;
    return 1;
}

static int zt_string_set_add_slice(zt_string_set *set, const char *text, size_t start, size_t end) {
    char *value;
    size_t length;
    int ok;

    if (set == NULL || text == NULL || end <= start) return 1;

    length = end - start;
    value = (char *)malloc(length + 1);
    if (value == NULL) return 0;

    memcpy(value, text + start, length);
    value[length] = '\0';

    ok = zt_string_set_add(set, value);
    free(value);
    return ok;
}

static int zt_native_token_text_equals(zt_token token, const char *text) {
    size_t text_len;

    if (token.text == NULL || text == NULL) return 0;

    text_len = strlen(text);
    return token.length == text_len && strncmp(token.text, text, text_len) == 0;
}

static int zt_native_token_is_import_part(zt_token token) {
    return token.kind == ZT_TOKEN_IDENTIFIER ||
           token.kind == ZT_TOKEN_LIST ||
           token.kind == ZT_TOKEN_MAP;
}

static zt_token zt_native_next_non_comment_token(zt_lexer *lexer) {
    zt_token token;

    do {
        token = zt_lexer_next_token(lexer);
    } while (token.kind == ZT_TOKEN_COMMENT);

    return token;
}

static int zt_native_collect_std_imports_from_source(
        const char *source_name,
        const char *source_text,
        zt_string_set *imports) {
    zt_lexer *lexer;
    zt_token token;
    zt_token pending;
    int has_pending = 0;

    if (source_text == NULL || imports == NULL) return 0;

    lexer = zt_lexer_make(source_name, source_text, strlen(source_text));
    if (lexer == NULL) return 0;

    for (;;) {
        if (has_pending) {
            token = pending;
            has_pending = 0;
        } else {
            token = zt_native_next_non_comment_token(lexer);
        }
        if (token.kind == ZT_TOKEN_EOF) break;

        if (token.kind != ZT_TOKEN_IMPORT) {
            continue;
        }

        {
            zt_token root = zt_native_next_non_comment_token(lexer);
            char import_buf[256];
            size_t import_len = 0;

            if (root.kind != ZT_TOKEN_IDENTIFIER || !zt_native_token_text_equals(root, "std")) {
                continue;
            }

            memcpy(import_buf, "std", 4);
            import_len = 3;

            while (1) {
                zt_token separator = zt_native_next_non_comment_token(lexer);
                zt_token part;

                if (separator.kind != ZT_TOKEN_DOT) {
                    pending = separator;
                    has_pending = 1;
                    break;
                }

                part = zt_native_next_non_comment_token(lexer);
                if (!zt_native_token_is_import_part(part) ||
                    import_len + part.length + 2 > sizeof(import_buf)) {
                    pending = part;
                    has_pending = 1;
                    break;
                }

                import_buf[import_len++] = '.';
                memcpy(import_buf + import_len, part.text, part.length);
                import_len += part.length;
                import_buf[import_len] = '\0';
            }

            if (import_len > 3 && !zt_string_set_add(imports, import_buf)) {
                zt_lexer_dispose(lexer);
                return 0;
            }
        }
    }

    zt_lexer_dispose(lexer);
    return 1;
}

static int zt_native_collect_std_imports_from_file(const char *src_path, zt_string_set *imports) {
    char *source_text;
    int ok;

    if (src_path == NULL || imports == NULL) return 0;

    source_text = zt_read_file(src_path);
    if (source_text == NULL) return 0;

    ok = zt_native_collect_std_imports_from_source(src_path, source_text, imports);
    free(source_text);
    return ok;
}

static int zt_type_ident_char(int ch) {
    return isalnum((unsigned char)ch) || ch == '_' || ch == '.';
}

static char *zt_strip_type_whitespace(const char *text) {
    size_t i;
    size_t out = 0;
    size_t length;
    char *clean;

    if (text == NULL) return NULL;

    length = strlen(text);
    clean = (char *)malloc(length + 1);
    if (clean == NULL) return NULL;

    for (i = 0; i < length; i += 1) {
        if (!isspace((unsigned char)text[i])) {
            clean[out++] = text[i];
        }
    }

    clean[out] = '\0';
    return clean;
}

static void zt_collect_generic_instances_in_range(
        const char *text,
        size_t start,
        size_t end,
        zt_string_set *set) {
    size_t i;

    if (text == NULL || set == NULL || end <= start) return;

    i = start;
    while (i < end) {
        if (!zt_type_ident_char((unsigned char)text[i])) {
            i += 1;
            continue;
        }

        {
            size_t ident_start = i;
            size_t ident_end = i;
            size_t j;
            size_t depth;

            while (ident_end < end && zt_type_ident_char((unsigned char)text[ident_end])) {
                ident_end += 1;
            }

            j = ident_end;
            if (j >= end || text[j] != '<') {
                i = ident_end;
                continue;
            }

            depth = 1;
            j += 1;
            while (j < end && depth > 0) {
                if (text[j] == '<') depth += 1;
                else if (text[j] == '>') depth -= 1;
                j += 1;
            }

            if (depth != 0) {
                return;
            }

            if (!zt_string_set_add_slice(set, text, ident_start, j)) {
                return;
            }

            zt_collect_generic_instances_in_range(text, ident_end + 1, j - 1, set);
            i = j;
        }
    }
}

static void zt_collect_generic_instances_from_type_name(const char *type_name, zt_string_set *set) {
    char *clean;

    if (type_name == NULL || type_name[0] == '\0' || set == NULL) return;

    clean = zt_strip_type_whitespace(type_name);
    if (clean == NULL) return;

    zt_collect_generic_instances_in_range(clean, 0, strlen(clean), set);
    free(clean);
}

static void zt_collect_generic_instances_from_expr(const zir_expr *expr, zt_string_set *set) {
    size_t i;

    if (expr == NULL || set == NULL) return;

    switch (expr->kind) {
        case ZIR_EXPR_COPY:
        case ZIR_EXPR_LIST_LEN:
        case ZIR_EXPR_MAP_LEN:
        case ZIR_EXPR_OPTIONAL_PRESENT:
        case ZIR_EXPR_OPTIONAL_IS_PRESENT:
        case ZIR_EXPR_OUTCOME_SUCCESS:
        case ZIR_EXPR_OUTCOME_FAILURE:
        case ZIR_EXPR_OUTCOME_IS_SUCCESS:
        case ZIR_EXPR_OUTCOME_VALUE:
        case ZIR_EXPR_TRY_PROPAGATE:
            zt_collect_generic_instances_from_expr(expr->as.single.value, set);
            return;
        case ZIR_EXPR_UNARY:
            zt_collect_generic_instances_from_expr(expr->as.unary.operand, set);
            return;
        case ZIR_EXPR_BINARY:
            zt_collect_generic_instances_from_expr(expr->as.binary.left, set);
            zt_collect_generic_instances_from_expr(expr->as.binary.right, set);
            return;
        case ZIR_EXPR_CALL_DIRECT:
        case ZIR_EXPR_CALL_EXTERN:
        case ZIR_EXPR_CALL_RUNTIME_INTRINSIC:
            for (i = 0; i < expr->as.call.args.count; i += 1) {
                zt_collect_generic_instances_from_expr(expr->as.call.args.items[i], set);
            }
            return;
        case ZIR_EXPR_MAKE_STRUCT:
            zt_collect_generic_instances_from_type_name(expr->as.make_struct.type_name, set);
            for (i = 0; i < expr->as.make_struct.fields.count; i += 1) {
                zt_collect_generic_instances_from_expr(expr->as.make_struct.fields.items[i].value, set);
            }
            return;
        case ZIR_EXPR_MAKE_LIST: {
            size_t len;
            char *type_name;
            zt_collect_generic_instances_from_type_name(expr->as.make_list.item_type_name, set);
            len = strlen(expr->as.make_list.item_type_name != NULL ? expr->as.make_list.item_type_name : "") + 16;
            type_name = (char *)malloc(len);
            if (type_name != NULL) {
                snprintf(type_name, len, "list<%s>", expr->as.make_list.item_type_name != NULL ? expr->as.make_list.item_type_name : "");
                zt_collect_generic_instances_from_type_name(type_name, set);
                free(type_name);
            }
            for (i = 0; i < expr->as.make_list.items.count; i += 1) {
                zt_collect_generic_instances_from_expr(expr->as.make_list.items.items[i], set);
            }
            return;
        }
        case ZIR_EXPR_MAKE_MAP: {
            size_t len;
            char *type_name;
            zt_collect_generic_instances_from_type_name(expr->as.make_map.key_type_name, set);
            zt_collect_generic_instances_from_type_name(expr->as.make_map.value_type_name, set);
            len = strlen(expr->as.make_map.key_type_name != NULL ? expr->as.make_map.key_type_name : "") +
                  strlen(expr->as.make_map.value_type_name != NULL ? expr->as.make_map.value_type_name : "") + 24;
            type_name = (char *)malloc(len);
            if (type_name != NULL) {
                snprintf(type_name, len, "map<%s,%s>",
                    expr->as.make_map.key_type_name != NULL ? expr->as.make_map.key_type_name : "",
                    expr->as.make_map.value_type_name != NULL ? expr->as.make_map.value_type_name : "");
                zt_collect_generic_instances_from_type_name(type_name, set);
                free(type_name);
            }
            for (i = 0; i < expr->as.make_map.entries.count; i += 1) {
                zt_collect_generic_instances_from_expr(expr->as.make_map.entries.items[i].key, set);
                zt_collect_generic_instances_from_expr(expr->as.make_map.entries.items[i].value, set);
            }
            return;
        }
        case ZIR_EXPR_GET_FIELD:
            zt_collect_generic_instances_from_expr(expr->as.field.object, set);
            return;
        case ZIR_EXPR_SET_FIELD:
            zt_collect_generic_instances_from_expr(expr->as.field.object, set);
            zt_collect_generic_instances_from_expr(expr->as.field.value, set);
            return;
        case ZIR_EXPR_INDEX_SEQ:
        case ZIR_EXPR_COALESCE:
        case ZIR_EXPR_LIST_PUSH:
            zt_collect_generic_instances_from_expr(expr->as.sequence.first, set);
            zt_collect_generic_instances_from_expr(expr->as.sequence.second, set);
            return;
        case ZIR_EXPR_SLICE_SEQ:
        case ZIR_EXPR_LIST_SET:
        case ZIR_EXPR_MAP_SET:
            zt_collect_generic_instances_from_expr(expr->as.sequence.first, set);
            zt_collect_generic_instances_from_expr(expr->as.sequence.second, set);
            zt_collect_generic_instances_from_expr(expr->as.sequence.third, set);
            return;
        case ZIR_EXPR_OPTIONAL_EMPTY: {
            size_t len;
            char *type_name;
            zt_collect_generic_instances_from_type_name(expr->as.type_only.type_name, set);
            len = strlen(expr->as.type_only.type_name != NULL ? expr->as.type_only.type_name : "") + 20;
            type_name = (char *)malloc(len);
            if (type_name != NULL) {
                snprintf(type_name, len, "optional<%s>", expr->as.type_only.type_name != NULL ? expr->as.type_only.type_name : "");
                zt_collect_generic_instances_from_type_name(type_name, set);
                free(type_name);
            }
            return;
        }
        default:
            return;
    }
}

static void zt_collect_generic_instances_from_module(const zir_module *module_decl, zt_string_set *set) {
    size_t i;

    if (module_decl == NULL || set == NULL) return;

    for (i = 0; i < module_decl->struct_count; i += 1) {
        size_t f;
        const zir_struct_decl *struct_decl = &module_decl->structs[i];
        for (f = 0; f < struct_decl->field_count; f += 1) {
            zt_collect_generic_instances_from_type_name(struct_decl->fields[f].type_name, set);
        }
    }

    for (i = 0; i < module_decl->enum_count; i += 1) {
        size_t v;
        const zir_enum_decl *enum_decl = &module_decl->enums[i];
        for (v = 0; v < enum_decl->variant_count; v += 1) {
            size_t f;
            const zir_enum_variant_decl *variant = &enum_decl->variants[v];
            for (f = 0; f < variant->field_count; f += 1) {
                zt_collect_generic_instances_from_type_name(variant->fields[f].type_name, set);
            }
        }
    }

    for (i = 0; i < module_decl->function_count; i += 1) {
        size_t p;
        size_t b;
        const zir_function *function_decl = &module_decl->functions[i];

        zt_collect_generic_instances_from_type_name(function_decl->return_type, set);
        zt_collect_generic_instances_from_type_name(function_decl->receiver_type_name, set);

        for (p = 0; p < function_decl->param_count; p += 1) {
            zt_collect_generic_instances_from_type_name(function_decl->params[p].type_name, set);
            zt_collect_generic_instances_from_expr(function_decl->params[p].where_clause, set);
        }

        for (b = 0; b < function_decl->block_count; b += 1) {
            size_t instr;
            const zir_block *block = &function_decl->blocks[b];

            for (instr = 0; instr < block->instruction_count; instr += 1) {
                const zir_instruction *instruction = &block->instructions[instr];
                zt_collect_generic_instances_from_type_name(instruction->type_name, set);
                zt_collect_generic_instances_from_expr(instruction->expr, set);
            }

            zt_collect_generic_instances_from_expr(block->terminator.value, set);
            zt_collect_generic_instances_from_expr(block->terminator.condition, set);
            zt_collect_generic_instances_from_expr(block->terminator.message, set);
        }
    }
}

static void zt_format_generic_preview(const zt_string_set *set, char *dest, size_t capacity) {
    size_t i;
    size_t max_items;

    if (dest == NULL || capacity == 0) return;
    dest[0] = '\0';

    if (set == NULL || set->count == 0) return;

    max_items = set->count < 3 ? set->count : 3;
    for (i = 0; i < max_items; i += 1) {
        size_t used = strlen(dest);
        size_t left = used < capacity ? capacity - used : 0;
        if (left <= 1) break;
        if (i > 0) {
            snprintf(dest + used, left, ", ");
            used = strlen(dest);
            left = used < capacity ? capacity - used : 0;
            if (left <= 1) break;
        }
        snprintf(dest + used, left, "%s", set->items[i]);
    }

    if (set->count > max_items) {
        size_t used = strlen(dest);
        size_t left = used < capacity ? capacity - used : 0;
        if (left > 1) {
            snprintf(dest + used, left, ", ...");
        }
    }
}

static int zt_enforce_monomorphization_limit(
        zt_driver_context *ctx,
        const char *manifest_path,
        size_t monomorphization_limit,
        const zir_module *module_decl) {
    zt_string_set generic_instances;

    if (module_decl == NULL || monomorphization_limit == 0) return 1;

    zt_string_set_init(&generic_instances);
    zt_collect_generic_instances_from_module(module_decl, &generic_instances);

    if (generic_instances.count > monomorphization_limit) {
        char preview[256];
        zt_source_span span = zt_source_span_make(
            manifest_path != NULL && manifest_path[0] != '\0' ? manifest_path : "<project>",
            1,
            1,
            1);

        zt_format_generic_preview(&generic_instances, preview, sizeof(preview));
        if (preview[0] != '\0') {
            zt_print_single_diag(
                ctx,
                "project",
                ZT_DIAG_PROJECT_MONOMORPHIZATION_LIMIT_EXCEEDED,
                span,
                "build.monomorphization_limit exceeded: found %zu generic instantiations, limit is %zu (examples: %s)",
                generic_instances.count,
                monomorphization_limit,
                preview);
        } else {
            zt_print_single_diag(
                ctx,
                "project",
                ZT_DIAG_PROJECT_MONOMORPHIZATION_LIMIT_EXCEEDED,
                span,
                "build.monomorphization_limit exceeded: found %zu generic instantiations, limit is %zu",
                generic_instances.count,
                monomorphization_limit);
        }
        zt_string_set_dispose(&generic_instances);
        return 0;
    }

    zt_string_set_dispose(&generic_instances);
    return 1;
}

void zt_project_compile_result_init(zt_project_compile_result *result) {
    memset(result, 0, sizeof(*result));
}

void zt_project_compile_result_dispose(zt_project_compile_result *result) {
    if (result == NULL) return;
    if (result->has_zir) {
        zir_lower_result_dispose(&result->zir);
        result->has_zir = 0;
    }
    zt_project_source_file_list_dispose(&result->source_files);
}

int zt_compile_project(
        zt_driver_context *ctx,
        const char *input_path,
        zt_project_compile_result *out) {
    zt_project_parse_result project;
    char source_root_path[512];
    zt_ast_node *program_root = NULL;
    zt_bind_result bound;
    zt_check_result checked;
    zt_hir_lower_result hir = { 0 };
    zir_lower_result zir;
    zir_verifier_result verifier;
    int program_ready = 0;
    int bound_ready = 0;
    int checked_ready = 0;
    int hir_ready = 0;
    int zir_ready = 0;

    zt_project_compile_result_init(out);
    zt_project_source_file_list_init(&out->source_files);

    if (!zt_resolve_project_paths(
            input_path,
            out->project_root,
            sizeof(out->project_root),
            out->manifest_path,
            sizeof(out->manifest_path))) {
        return 0;
    }

    if (ctx == NULL) return 0;

    if (!zt_project_load_file(out->manifest_path, &project)) {
        zt_print_project_parse_error(ctx, out->manifest_path, &project);
        return 0;
    }

    out->manifest = project.manifest;
    zt_apply_manifest_lang(&out->manifest);
    zt_driver_context_activate_project(ctx, &out->manifest, out->project_root);

    if (zt_project_manifest_kind(&out->manifest) == ZT_PROJECT_KIND_APP &&
            !zt_project_resolve_entry_source_path(
                &out->manifest,
                out->project_root,
                out->entry_path,
                sizeof(out->entry_path))) {
        zt_print_single_diag(ctx, "project", ZT_DIAG_PROJECT_PATH_TOO_LONG, zt_source_span_unknown(), "app.entry source path is too long");
        return 0;
    }

    if (!zt_join_path(source_root_path, sizeof(source_root_path), out->project_root, out->manifest.source_root)) {
        zt_print_single_diag(ctx, "project", ZT_DIAG_PROJECT_PATH_TOO_LONG, zt_source_span_unknown(), "source.root path is too long");
        goto fail;
    }

    if (!zt_path_is_dir(source_root_path)) {
        zt_print_single_diag(ctx, "project", ZT_DIAG_PROJECT_INVALID_INPUT, zt_source_span_unknown(), "source.root '%s' is not a directory", source_root_path);
        goto fail;
    }

    if (!zt_project_discover_zt_files(source_root_path, &out->source_files)) {
        goto fail;
    }

    /* Load only the specific stdlib modules that are actually imported */
    {
        const char *stdlib_base = getenv("ZENITH_HOME");
        char runtime_base[768];
        char default_base[768];
        int skip_std_import_scan = 0;
        size_t si;
        zt_string_set std_imports;

        zt_string_set_init(&std_imports);

        zt_runtime_root(runtime_base, sizeof(runtime_base));

        if (zt_project_manifest_kind(&out->manifest) == ZT_PROJECT_KIND_LIB &&
                strncmp(out->manifest.lib_root_namespace, "std", 3) == 0 &&
                zt_path_suffix_matches(source_root_path, "stdlib")) {
            skip_std_import_scan = 1;
        }

        if (zt_home_has_stdlib(runtime_base)) {
            if (!zt_join_path(default_base, sizeof(default_base), runtime_base, "stdlib")) {
                strcpy(default_base, "stdlib");
            }
            stdlib_base = default_base;
        } else if (stdlib_base != NULL && stdlib_base[0] != '\0' && zt_home_has_stdlib(stdlib_base)) {
            if (!zt_join_path(default_base, sizeof(default_base), stdlib_base, "stdlib")) {
                strcpy(default_base, "stdlib");
            }
            stdlib_base = default_base;
        } else {
            strcpy(default_base, "stdlib");
            stdlib_base = default_base;
        }

        if (!skip_std_import_scan) {
            for (si = 0; si < out->source_files.count; si++) {
                const char *src_path = out->source_files.items[si].path;
                if (!zt_native_collect_std_imports_from_file(src_path, &std_imports)) {
                    zt_string_set_dispose(&std_imports);
                    zt_print_single_diag(
                        ctx,
                        "project",
                        ZT_DIAG_PROJECT_ERROR,
                        zt_source_span_unknown(),
                        "failed to scan stdlib imports in '%s'",
                        src_path);
                    goto fail;
                }
            }

            for (si = 0; si < std_imports.count; si++) {
                char import_buf[256];
                char mod_path[768];
                size_t j;
                int already_loaded;

                if (!zt_copy_text(import_buf, sizeof(import_buf), std_imports.items[si])) {
                    continue;
                }

                snprintf(mod_path, sizeof(mod_path), "%s", stdlib_base);
                for (j = 0; import_buf[j]; j++) {
                    if (import_buf[j] == '.') import_buf[j] = '/';
                }
                if (strlen(mod_path) + 1 + strlen(import_buf) + 3 >= sizeof(mod_path)) {
                    continue;
                }

                strcat(mod_path, "/");
                strcat(mod_path, import_buf);
                strcat(mod_path, ".zt");

                already_loaded = 0;
                for (j = 0; j < out->source_files.count; j++) {
                    if (strcmp(out->source_files.items[j].path, mod_path) == 0) {
                        already_loaded = 1;
                        break;
                    }
                }

                if (!already_loaded && zt_path_is_file(mod_path)) {
                    zt_project_source_file_list_push(&out->source_files, mod_path);
                }
            }
        }

        zt_string_set_dispose(&std_imports);
    }

    if (out->source_files.count == 0) {
        zt_print_single_diag(ctx, "project", ZT_DIAG_PROJECT_ERROR, zt_source_span_unknown(), "source.root '%s' contains no .zt files", source_root_path);
        goto fail;
    }

    if (!zt_parse_project_sources(ctx, &out->source_files)) {
        goto fail;
    }

    zt_diag_list proj_diags = zt_diag_list_make();
    if (!zt_validate_source_namespaces(&out->source_files, &out->manifest, &proj_diags)) {
        zt_print_diagnostics(ctx, "project", &proj_diags);
        zt_diag_list_dispose(&proj_diags);
        goto fail;
    }
    zt_diag_list_dispose(&proj_diags);

    program_root = zt_build_combined_project_ast(&out->source_files, &out->manifest);
    if (program_root == NULL) {
        zt_print_single_diag(ctx, "project", ZT_DIAG_PROJECT_ERROR, zt_source_span_unknown(), "unable to aggregate project sources");
        goto fail;
    }
    program_ready = 1;

    bound = zt_bind_file(program_root);
    bound_ready = 1;
    if (!zt_handle_stage_diagnostics(ctx, &out->manifest, "binding", &bound.diagnostics)) {
        goto fail;
    }

    checked = zt_check_file(program_root);
    checked_ready = 1;
    if (!zt_handle_stage_diagnostics(ctx, &out->manifest, "type", &checked.diagnostics)) {
        goto fail;
    }

    hir = zt_lower_ast_to_hir(program_root);
    hir_ready = 1;
    if (!zt_handle_stage_diagnostics(ctx, &out->manifest, "hir", &hir.diagnostics) || hir.module == NULL) {
        goto fail;
    }

    zir = zir_lower_hir_to_zir(hir.module);
    zir_ready = 1;
    if (!zt_handle_stage_diagnostics(ctx, &out->manifest, "zir", &zir.diagnostics)) {
        goto fail;
    }

    if (!zir_verify_module(&zir.module, &verifier)) {
        zt_source_span span = zt_source_span_make("<zir>", 1, 1, 1);
        if (verifier.has_span) {
            const char *source_name = verifier.source_name != NULL && verifier.source_name[0] != '\0'
                ? verifier.source_name
                : "<zir>";
            size_t line = verifier.line > 0 ? verifier.line : 1;
            size_t column = verifier.column > 0 ? verifier.column : 1;
            span = zt_source_span_make(source_name, line, column, column);
        }
        zt_print_single_diag(
            ctx,
            "zir.verify",
            zt_diag_code_from_zir_verifier(verifier.code),
            span,
            "%s",
            verifier.message[0] != '\0' ? verifier.message : "ZIR verification failed");
        goto fail;
    }

    if (!zt_enforce_monomorphization_limit(ctx, out->manifest_path, out->manifest.build_monomorphization_limit, &zir.module)) {
        goto fail;
    }

    out->zir = zir;
    out->has_zir = 1;
    zir_ready = 0;

    if (hir_ready) zt_hir_lower_result_dispose(&hir);
    if (checked_ready) zt_check_result_dispose(&checked);
    if (bound_ready) zt_bind_result_dispose(&bound);
    if (program_ready) { (void)program_root; }
    return 1;

fail:
    if (zir_ready) zir_lower_result_dispose(&zir);
    if (hir_ready) zt_hir_lower_result_dispose(&hir);
    if (checked_ready) zt_check_result_dispose(&checked);
    if (bound_ready) zt_bind_result_dispose(&bound);
    if (program_ready) { (void)program_root; }
    zt_project_source_file_list_dispose(&out->source_files);
    return 0;
}

/*
 * zt_compile_single_file — compile a standalone .zt file without a project.
 *
 * Creates a synthetic manifest in memory, loads the single file plus any
 * stdlib imports, then runs the full pipeline (parse -> bind -> check ->
 * HIR -> ZIR -> verify). Namespace-to-path validation is intentionally
 * skipped so that any .zt file can be compiled from any location.
 */
int zt_compile_single_file(
        zt_driver_context *ctx,
        const char *zt_file_path,
        zt_project_compile_result *out) {
    zt_ast_node *program_root = NULL;
    zt_bind_result bound;
    zt_check_result checked;
    zt_hir_lower_result hir = { 0 };
    zir_lower_result zir;
    zir_verifier_result verifier;
    int program_ready = 0;
    int bound_ready = 0;
    int checked_ready = 0;
    int hir_ready = 0;
    int zir_ready = 0;
    char abs_path[512];
    const char *file_base;
    const char *dot;
    size_t stem_len;

    zt_project_compile_result_init(out);
    zt_project_source_file_list_init(&out->source_files);

    /* Resolve absolute path of the .zt file */
    if (zt_file_path == NULL || zt_file_path[0] == '\0') {
        zt_print_single_diag(ctx, "script", ZT_DIAG_PROJECT_INVALID_INPUT, zt_source_span_unknown(),
            "no .zt file path provided");
        return 0;
    }

    if (!zt_path_is_file(zt_file_path)) {
        zt_print_single_diag(ctx, "script", ZT_DIAG_PROJECT_INVALID_INPUT, zt_source_span_unknown(),
            "file not found: '%s'", zt_file_path);
        return 0;
    }

    if (!zt_copy_text(abs_path, sizeof(abs_path), zt_file_path)) {
        zt_print_single_diag(ctx, "script", ZT_DIAG_PROJECT_PATH_TOO_LONG, zt_source_span_unknown(),
            "file path is too long");
        return 0;
    }

    /* Derive project_root from the file's parent directory */
    if (!zt_dirname(out->project_root, sizeof(out->project_root), abs_path)) {
        if (!zt_get_current_dir(out->project_root, sizeof(out->project_root))) {
            zt_copy_text(out->project_root, sizeof(out->project_root), ".");
        }
    }

    /* Extract stem (filename without extension) for the synthetic manifest */
    {
        size_t i;
        size_t last_sep = 0;
        int has_sep = 0;
        for (i = 0; abs_path[i] != '\0'; i++) {
            if (abs_path[i] == '/' || abs_path[i] == '\\') {
                last_sep = i;
                has_sep = 1;
            }
        }
        file_base = has_sep ? abs_path + last_sep + 1 : abs_path;
        dot = strrchr(file_base, '.');
        stem_len = dot != NULL ? (size_t)(dot - file_base) : strlen(file_base);
        if (stem_len == 0) stem_len = strlen(file_base);
    }

    /* Build synthetic manifest */
    zt_project_manifest_init(&out->manifest);
    snprintf(out->manifest.project_name, sizeof(out->manifest.project_name), "%.*s", (int)stem_len, file_base);
    snprintf(out->manifest.project_kind, sizeof(out->manifest.project_kind), "app");
    snprintf(out->manifest.version, sizeof(out->manifest.version), "0.0.0");
    snprintf(out->manifest.source_root, sizeof(out->manifest.source_root), ".");
    snprintf(out->manifest.build_target, sizeof(out->manifest.build_target), "native");
    snprintf(out->manifest.build_profile, sizeof(out->manifest.build_profile), "debug");
    snprintf(out->manifest.build_output, sizeof(out->manifest.build_output), "build");
    snprintf(out->manifest.output_dir, sizeof(out->manifest.output_dir), "build");
    snprintf(out->manifest.test_root, sizeof(out->manifest.test_root), "tests");
    snprintf(out->manifest.zdoc_root, sizeof(out->manifest.zdoc_root), "zdoc");
    snprintf(out->manifest.accessibility_profile, sizeof(out->manifest.accessibility_profile), "balanced");
    out->manifest.build_monomorphization_limit = ZT_PROJECT_DEFAULT_MONOMORPHIZATION_LIMIT;

    /* app_entry will be set after parsing, when we know the file's namespace */
    snprintf(out->manifest.app_entry, sizeof(out->manifest.app_entry), "script");
    snprintf(out->manifest.entry, sizeof(out->manifest.entry), "script");
    snprintf(out->manifest.output_name, sizeof(out->manifest.output_name), "%.*s", (int)stem_len, file_base);

    out->manifest_path[0] = '\0';

    if (ctx != NULL) {
        zt_driver_context_activate_project(ctx, &out->manifest, out->project_root);
    }

    /* Add the single .zt file */
    zt_project_source_file_list_push(&out->source_files, abs_path);

    /* Scan and load stdlib imports */
    {
        const char *stdlib_base = getenv("ZENITH_HOME");
        char runtime_base[768];
        char default_base[768];
        size_t si;
        zt_string_set std_imports;

        zt_string_set_init(&std_imports);
        zt_runtime_root(runtime_base, sizeof(runtime_base));

        if (zt_home_has_stdlib(runtime_base)) {
            if (!zt_join_path(default_base, sizeof(default_base), runtime_base, "stdlib")) {
                strcpy(default_base, "stdlib");
            }
            stdlib_base = default_base;
        } else if (stdlib_base != NULL && stdlib_base[0] != '\0' && zt_home_has_stdlib(stdlib_base)) {
            if (!zt_join_path(default_base, sizeof(default_base), stdlib_base, "stdlib")) {
                strcpy(default_base, "stdlib");
            }
            stdlib_base = default_base;
        } else {
            strcpy(default_base, "stdlib");
            stdlib_base = default_base;
        }

        if (!zt_native_collect_std_imports_from_file(abs_path, &std_imports)) {
            zt_string_set_dispose(&std_imports);
            zt_print_single_diag(ctx, "script", ZT_DIAG_PROJECT_ERROR, zt_source_span_unknown(),
                "failed to scan stdlib imports in '%s'", abs_path);
            goto fail;
        }

        for (si = 0; si < std_imports.count; si++) {
            char import_buf[256];
            char mod_path[768];
            size_t j;
            int already_loaded;

            if (!zt_copy_text(import_buf, sizeof(import_buf), std_imports.items[si])) {
                continue;
            }

            snprintf(mod_path, sizeof(mod_path), "%s", stdlib_base);
            for (j = 0; import_buf[j]; j++) {
                if (import_buf[j] == '.') import_buf[j] = '/';
            }
            if (strlen(mod_path) + 1 + strlen(import_buf) + 3 >= sizeof(mod_path)) {
                continue;
            }

            strcat(mod_path, "/");
            strcat(mod_path, import_buf);
            strcat(mod_path, ".zt");

            already_loaded = 0;
            for (j = 0; j < out->source_files.count; j++) {
                if (strcmp(out->source_files.items[j].path, mod_path) == 0) {
                    already_loaded = 1;
                    break;
                }
            }

            if (!already_loaded && zt_path_is_file(mod_path)) {
                zt_project_source_file_list_push(&out->source_files, mod_path);
            }
        }

        zt_string_set_dispose(&std_imports);
    }

    /* Parse sources */
    if (!zt_parse_project_sources(ctx, &out->source_files)) {
        goto fail;
    }

    /* Update manifest entry/app_entry from the parsed namespace */
    {
        const zt_ast_node *root = out->source_files.items[0].parsed.root;
        if (root != NULL && root->kind == ZT_AST_FILE && root->as.file.module_name != NULL) {
            zt_copy_text(out->manifest.app_entry, sizeof(out->manifest.app_entry), root->as.file.module_name);
            zt_copy_text(out->manifest.entry, sizeof(out->manifest.entry), root->as.file.module_name);
        }
    }

    /* Skip zt_validate_source_namespaces — single-file mode does not
     * require namespace-to-path alignment. */

    program_root = zt_build_combined_project_ast(&out->source_files, &out->manifest);
    if (program_root == NULL) {
        zt_print_single_diag(ctx, "script", ZT_DIAG_PROJECT_ERROR, zt_source_span_unknown(),
            "unable to aggregate source");
        goto fail;
    }
    program_ready = 1;

    bound = zt_bind_file(program_root);
    bound_ready = 1;
    if (!zt_handle_stage_diagnostics(ctx, &out->manifest, "binding", &bound.diagnostics)) {
        goto fail;
    }

    checked = zt_check_file(program_root);
    checked_ready = 1;
    if (!zt_handle_stage_diagnostics(ctx, &out->manifest, "type", &checked.diagnostics)) {
        goto fail;
    }

    hir = zt_lower_ast_to_hir(program_root);
    hir_ready = 1;
    if (!zt_handle_stage_diagnostics(ctx, &out->manifest, "hir", &hir.diagnostics) || hir.module == NULL) {
        goto fail;
    }

    zir = zir_lower_hir_to_zir(hir.module);
    zir_ready = 1;
    if (!zt_handle_stage_diagnostics(ctx, &out->manifest, "zir", &zir.diagnostics)) {
        goto fail;
    }

    if (!zir_verify_module(&zir.module, &verifier)) {
        zt_source_span span = zt_source_span_make("<zir>", 1, 1, 1);
        if (verifier.has_span) {
            const char *source_name = verifier.source_name != NULL && verifier.source_name[0] != '\0'
                ? verifier.source_name
                : "<zir>";
            size_t line = verifier.line > 0 ? verifier.line : 1;
            size_t column = verifier.column > 0 ? verifier.column : 1;
            span = zt_source_span_make(source_name, line, column, column);
        }
        zt_print_single_diag(
            ctx,
            "zir.verify",
            zt_diag_code_from_zir_verifier(verifier.code),
            span,
            "%s",
            verifier.message[0] != '\0' ? verifier.message : "ZIR verification failed");
        goto fail;
    }

    if (!zt_enforce_monomorphization_limit(ctx, abs_path, out->manifest.build_monomorphization_limit, &zir.module)) {
        goto fail;
    }

    out->zir = zir;
    out->has_zir = 1;
    zir_ready = 0;

    if (hir_ready) zt_hir_lower_result_dispose(&hir);
    if (checked_ready) zt_check_result_dispose(&checked);
    if (bound_ready) zt_bind_result_dispose(&bound);
    if (program_ready) { (void)program_root; }
    return 1;

fail:
    if (zir_ready) zir_lower_result_dispose(&zir);
    if (hir_ready) zt_hir_lower_result_dispose(&hir);
    if (checked_ready) zt_check_result_dispose(&checked);
    if (bound_ready) zt_bind_result_dispose(&bound);
    if (program_ready) { (void)program_root; }
    zt_project_source_file_list_dispose(&out->source_files);
    return 0;
}

int zt_emit_module_to_c(
        const zir_module *module_decl,
        c_emitter *emitter,
        c_emit_result *out_error) {
    c_emit_result emit_result;

    c_emitter_init(emitter);
    c_emit_result_init(&emit_result);

    if (!c_emitter_emit_module(emitter, module_decl, &emit_result)) {
        if (out_error != NULL) {
            *out_error = emit_result;
        }
        c_emitter_dispose(emitter);
        return 0;
    }

    if (out_error != NULL) {
        out_error->ok = 1;
        out_error->code = C_EMIT_OK;
        out_error->message[0] = '\0';
    }

    return 1;
}

static int zt_get_file_mtime(const char *path, time_t *out_mtime) {
    struct stat info;
    if (path == NULL || out_mtime == NULL) return 0;
    if (stat(path, &info) != 0) return 0;
    *out_mtime = info.st_mtime;
    return 1;
}

static int zt_get_executable_dir(char *buffer, size_t capacity) {
    char exe_path[1024];

    if (buffer == NULL || capacity == 0) return 0;

#ifdef _WIN32
    {
        DWORD length = GetModuleFileNameA(NULL, exe_path, (DWORD)sizeof(exe_path));
        if (length == 0 || length >= (DWORD)sizeof(exe_path)) return 0;
        exe_path[length] = '\0';
    }
#else
    {
        ssize_t length = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);
        if (length <= 0 || (size_t)length >= sizeof(exe_path)) return 0;
        exe_path[length] = '\0';
    }
#endif

    return zt_dirname(buffer, capacity, exe_path);
}

static int zt_home_has_runtime(const char *home_root) {
    char runtime_c_path[1024];
    if (home_root == NULL || home_root[0] == '\0') return 0;
    if (!zt_join_path(runtime_c_path, sizeof(runtime_c_path), home_root, "runtime/c/zenith_rt.c")) return 0;
    return zt_path_is_file(runtime_c_path);
}

static int zt_home_has_stdlib(const char *home_root) {
    char stdlib_path[1024];
    if (home_root == NULL || home_root[0] == '\0') return 0;
    if (!zt_join_path(stdlib_path, sizeof(stdlib_path), home_root, "stdlib")) return 0;
    return zt_path_is_dir(stdlib_path);
}

static void zt_runtime_root(char *buffer, size_t capacity) {
    const char *zenith_home;
    char executable_dir[768];

    if (buffer == NULL || capacity == 0) return;
    if (zt_get_executable_dir(executable_dir, sizeof(executable_dir)) &&
            zt_home_has_runtime(executable_dir)) {
        zt_copy_text(buffer, capacity, executable_dir);
        return;
    }

    zenith_home = getenv("ZENITH_HOME");
    if (zenith_home != NULL && zenith_home[0] != '\0' && zt_home_has_runtime(zenith_home)) {
        zt_copy_text(buffer, capacity, zenith_home);
        return;
    }

    zt_copy_text(buffer, capacity, ".");
}

static int zt_join_runtime_dep(char *buffer, size_t capacity, const char *relative_dep) {
    char runtime_root[768];
    if (buffer == NULL || capacity == 0 || relative_dep == NULL) return 0;
    zt_runtime_root(runtime_root, sizeof(runtime_root));
    return zt_join_path(buffer, capacity, runtime_root, relative_dep);
}

static int zt_runtime_object_is_stale(const char *runtime_obj_path) {
    static const char *deps[] = {
        "runtime/c/zenith_rt.c",
        "runtime/c/zenith_rt.h",
        "runtime/c/zenith_rt_templates.h"
    };
    size_t i;
    time_t obj_mtime;
    char dep_path[1024];

    if (!zt_get_file_mtime(runtime_obj_path, &obj_mtime)) return 1;

    for (i = 0; i < (sizeof(deps) / sizeof(deps[0])); i += 1) {
        time_t dep_mtime;
        if (!zt_join_runtime_dep(dep_path, sizeof(dep_path), deps[i])) {
            return 1;
        }
        if (!zt_get_file_mtime(dep_path, &dep_mtime) || dep_mtime > obj_mtime) {
            return 1;
        }
    }

    return 0;
}

static void zt_native_trim_line(char *line) {
    size_t len;
    if (line == NULL) return;
    len = strlen(line);
    while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
        line[len - 1] = '\0';
        len -= 1;
    }
}

static unsigned long zt_native_process_id(void) {
#ifdef _WIN32
    return (unsigned long)GetCurrentProcessId();
#else
    return (unsigned long)getpid();
#endif
}

static void zt_native_sleep_millis(unsigned long millis) {
#ifdef _WIN32
    Sleep((DWORD)millis);
#else
    usleep((unsigned int)(millis * 1000u));
#endif
}

int zt_native_make_temp_path(
        char *buffer,
        size_t capacity,
        const char *prefix,
        const char *extension) {
    static unsigned long counter = 0;
    unsigned long pid = zt_native_process_id();
    unsigned long serial = 0;

    if (buffer == NULL || capacity == 0) return 0;
    if (!zt_make_dirs(".ztc-tmp/native")) return 0;

    counter += 1;
    serial = counter;

    return snprintf(
               buffer,
               capacity,
               ".ztc-tmp/native/%s-%lu-%lu%s",
               prefix != NULL ? prefix : "tmp",
               pid,
               serial,
               extension != NULL ? extension : "") < (int)capacity;
}

#ifndef _WIN32
static int zt_native_decode_status(int status);
#endif

static char *zt_native_dup_prefixed_arg(const char *prefix, const char *value) {
    size_t prefix_len;
    size_t value_len;
    size_t total_len;
    char *buffer;

    if (prefix == NULL || value == NULL) return NULL;

    prefix_len = strlen(prefix);
    value_len = strlen(value);
    total_len = prefix_len + value_len + 1;
    buffer = (char *)malloc(total_len);
    if (buffer == NULL) return NULL;

    memcpy(buffer, prefix, prefix_len);
    memcpy(buffer + prefix_len, value, value_len + 1);
    return buffer;
}

static int zt_native_display_needs_quotes(const char *arg) {
    const unsigned char *cursor = (const unsigned char *)arg;
    if (arg == NULL || arg[0] == '\0') return 1;
    while (*cursor != '\0') {
        if (isspace(*cursor) || *cursor == '"' || *cursor == '\'' || *cursor == '\\') {
            return 1;
        }
        cursor += 1;
    }
    return 0;
}

static int zt_native_format_command(char *buffer, size_t capacity, const char *const *argv) {
    size_t used = 0;
    size_t index;

    if (buffer == NULL || capacity == 0) return 0;
    buffer[0] = '\0';
    if (argv == NULL || argv[0] == NULL) return 0;

    for (index = 0; argv[index] != NULL; index += 1) {
        const char *arg = argv[index];
        size_t arg_len = strlen(arg);
        size_t pos;

        if (index > 0) {
            if (used + 1 >= capacity) return 0;
            buffer[used++] = ' ';
        }

        if (!zt_native_display_needs_quotes(arg)) {
            if (used + arg_len >= capacity) return 0;
            memcpy(buffer + used, arg, arg_len);
            used += arg_len;
            continue;
        }

        if (used + 1 >= capacity) return 0;
        buffer[used++] = '"';
        for (pos = 0; pos < arg_len; pos += 1) {
            if (arg[pos] == '"' || arg[pos] == '\\') {
                if (used + 1 >= capacity) return 0;
                buffer[used++] = '\\';
            }
            if (used + 1 >= capacity) return 0;
            buffer[used++] = arg[pos];
        }
        if (used + 1 >= capacity) return 0;
        buffer[used++] = '"';
    }

    if (used >= capacity) return 0;
    buffer[used] = '\0';
    return 1;
}

#ifdef _WIN32
static int zt_native_append_windows_quoted_arg(char *buffer, size_t capacity, size_t *offset, const char *arg) {
    const char *cursor = arg != NULL ? arg : "";

    if (buffer == NULL || offset == NULL || *offset >= capacity) return 0;
    if (*offset + 1 >= capacity) return 0;
    buffer[(*offset)++] = '"';

    while (*cursor != '\0') {
        size_t slash_count = 0;
        while (cursor[slash_count] == '\\') {
            slash_count += 1;
        }

        if (cursor[slash_count] == '\0') {
            size_t extra = slash_count * 2;
            if (*offset + extra >= capacity) return 0;
            memset(buffer + *offset, '\\', extra);
            *offset += extra;
            break;
        }

        if (cursor[slash_count] == '"') {
            size_t extra = (slash_count * 2) + 1;
            if (*offset + extra + 1 >= capacity) return 0;
            memset(buffer + *offset, '\\', extra);
            *offset += extra;
            buffer[(*offset)++] = '"';
            cursor += slash_count + 1;
            continue;
        }

        if (*offset + slash_count + 1 >= capacity) return 0;
        memset(buffer + *offset, '\\', slash_count);
        *offset += slash_count;
        buffer[(*offset)++] = cursor[slash_count];
        cursor += slash_count + 1;
    }

    if (*offset + 1 >= capacity) return 0;
    buffer[(*offset)++] = '"';
    return 1;
}

static char *zt_native_build_windows_command_line(const char *const *argv) {
    size_t total = 1;
    size_t index;
    size_t offset = 0;
    char *buffer;

    if (argv == NULL || argv[0] == NULL) return NULL;

    for (index = 0; argv[index] != NULL; index += 1) {
        total += (strlen(argv[index]) * 2) + 4;
    }

    buffer = (char *)malloc(total);
    if (buffer == NULL) return NULL;

    for (index = 0; argv[index] != NULL; index += 1) {
        if (index > 0) {
            buffer[offset++] = ' ';
        }
        if (!zt_native_append_windows_quoted_arg(buffer, total, &offset, argv[index])) {
            free(buffer);
            return NULL;
        }
    }
    buffer[offset] = '\0';
    return buffer;
}
#endif

static int zt_native_write_capture_message(
        const char *capture_path,
        const char *program,
        const char *message) {
    FILE *file;

    if (capture_path == NULL || capture_path[0] == '\0') return 0;
    file = fopen(capture_path, "wb");
    if (file == NULL) return 0;
    if (program != NULL && program[0] != '\0') {
        fprintf(file, "%s: %s\n", program, message != NULL ? message : "failed");
    } else if (message != NULL) {
        fprintf(file, "%s\n", message);
    }
    fclose(file);
    return 1;
}

int zt_native_spawn_process(
        const char *const *argv,
        const char *capture_path,
        int *out_spawn_failed) {
    if (out_spawn_failed != NULL) {
        *out_spawn_failed = 0;
    }

    if (argv == NULL || argv[0] == NULL) return 1;

#ifdef _WIN32
    {
        SECURITY_ATTRIBUTES security = {0};
        STARTUPINFOA startup;
        PROCESS_INFORMATION process;
        HANDLE output_handle = INVALID_HANDLE_VALUE;
        char *command_line = NULL;
        DWORD exit_code = 0;

        memset(&startup, 0, sizeof(startup));
        memset(&process, 0, sizeof(process));

        command_line = zt_native_build_windows_command_line(argv);
        if (command_line == NULL) {
            if (out_spawn_failed != NULL) *out_spawn_failed = 1;
            zt_native_write_capture_message(capture_path, argv[0], "failed to allocate command line");
            return 127;
        }

        security.nLength = sizeof(security);
        security.bInheritHandle = TRUE;
        security.lpSecurityDescriptor = NULL;

        startup.cb = sizeof(startup);
        startup.dwFlags = STARTF_USESTDHANDLES;
        startup.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
        startup.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
        startup.hStdError = GetStdHandle(STD_ERROR_HANDLE);

        if (capture_path != NULL && capture_path[0] != '\0') {
            output_handle = CreateFileA(
                capture_path,
                GENERIC_WRITE,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                &security,
                CREATE_ALWAYS,
                FILE_ATTRIBUTE_NORMAL,
                NULL);
            if (output_handle == INVALID_HANDLE_VALUE) {
                free(command_line);
                if (out_spawn_failed != NULL) *out_spawn_failed = 1;
                return 127;
            }
            startup.hStdOutput = output_handle;
            startup.hStdError = output_handle;
        }

        if (!CreateProcessA(
                NULL,
                command_line,
                NULL,
                NULL,
                TRUE,
                0,
                NULL,
                NULL,
                &startup,
                &process)) {
            DWORD error_code = GetLastError();
            if (output_handle != INVALID_HANDLE_VALUE) CloseHandle(output_handle);
            free(command_line);
            if (out_spawn_failed != NULL) *out_spawn_failed = 1;
            if (error_code == ERROR_FILE_NOT_FOUND || error_code == ERROR_PATH_NOT_FOUND) {
                zt_native_write_capture_message(capture_path, argv[0], "command not found");
                return 127;
            }
            zt_native_write_capture_message(capture_path, argv[0], "failed to start process");
            return 126;
        }

        if (output_handle != INVALID_HANDLE_VALUE) CloseHandle(output_handle);
        free(command_line);

        WaitForSingleObject(process.hProcess, INFINITE);
        if (!GetExitCodeProcess(process.hProcess, &exit_code)) {
            exit_code = 126;
        }
        CloseHandle(process.hThread);
        CloseHandle(process.hProcess);
        return (int)exit_code;
    }
#else
    {
        pid_t pid;
        int status = 0;

        pid = fork();
        if (pid < 0) {
            if (out_spawn_failed != NULL) *out_spawn_failed = 1;
            zt_native_write_capture_message(capture_path, argv[0], strerror(errno));
            return 127;
        }

        if (pid == 0) {
            if (capture_path != NULL && capture_path[0] != '\0') {
                FILE *capture_file = fopen(capture_path, "wb");
                if (capture_file == NULL) {
                    fprintf(stderr, "%s: %s\n", argv[0], strerror(errno));
                    _exit(126);
                }
                if (dup2(fileno(capture_file), STDOUT_FILENO) < 0 ||
                        dup2(fileno(capture_file), STDERR_FILENO) < 0) {
                    fprintf(stderr, "%s: %s\n", argv[0], strerror(errno));
                    fclose(capture_file);
                    _exit(126);
                }
                fclose(capture_file);
            }

            execvp(argv[0], (char *const *)argv);
            if (errno == ENOENT) {
                fprintf(stderr, "%s: command not found\n", argv[0]);
                _exit(127);
            }
            fprintf(stderr, "%s: %s\n", argv[0], strerror(errno));
            _exit(126);
        }

        if (waitpid(pid, &status, 0) < 0) {
            if (out_spawn_failed != NULL) *out_spawn_failed = 1;
            zt_native_write_capture_message(capture_path, argv[0], strerror(errno));
            return 127;
        }

        return zt_native_decode_status(status);
    }
#endif
}

int zt_native_remove_file_if_exists(const char *path) {
    if (path == NULL || path[0] == '\0') return 1;
    if (remove(path) == 0) return 1;
    return errno == ENOENT;
}

static void zt_runtime_object_lock_release(const char *lock_path) {
    if (lock_path == NULL || lock_path[0] == '\0') return;
#ifdef _WIN32
    if (_rmdir(lock_path) != 0 && errno != ENOENT) {
        (void)errno;
    }
#else
    if (rmdir(lock_path) != 0 && errno != ENOENT) {
        (void)errno;
    }
#endif
}

static int zt_runtime_object_lock_acquire(
        zt_driver_context *ctx,
        const char *runtime_obj_path,
        const char *lock_path,
        int *out_lock_owned) {
    size_t attempts = 0;

    if (out_lock_owned != NULL) {
        *out_lock_owned = 0;
    }

    while (ZT_MKDIR(lock_path) != 0) {
        if (errno != EEXIST) {
            zt_print_single_diag(
                ctx,
                "backend.c.emit",
                ZT_DIAG_PROJECT_INVALID_INPUT,
                zt_source_span_unknown(),
                "failed to create runtime cache lock '%s'",
                lock_path);
            return 0;
        }

        if (!zt_runtime_object_is_stale(runtime_obj_path)) {
            return 1;
        }

        if (attempts >= 200) {
            zt_print_single_diag(
                ctx,
                "backend.c.emit",
                ZT_DIAG_PROJECT_INVALID_INPUT,
                zt_source_span_unknown(),
                "timed out waiting for runtime cache lock '%s'",
                lock_path);
            return 0;
        }

        attempts += 1;
        zt_native_sleep_millis(50);
    }

    if (out_lock_owned != NULL) {
        *out_lock_owned = 1;
    }
    return 1;
}

static int zt_ensure_runtime_object_current(zt_driver_context *ctx, const char *runtime_obj_path);

static int zt_native_line_is_warning(const char *line) {
    if (line == NULL) return 0;
    return strstr(line, ": warning:") != NULL;
}

static int zt_native_line_is_compiler_not_found(const char *line) {
    if (line == NULL) return 0;
    if (strstr(line, "gcc: command not found") != NULL) return 1;
    if (strstr(line, "not recognized as an internal or external command") != NULL) return 1;
    if (strstr(line, "'gcc' não é reconhecido") != NULL) return 1;
    return 0;
}

static int zt_native_line_is_error_undefined_reference(const char *line) {
    if (line == NULL) return 0;
    return strstr(line, "undefined reference to") != NULL;
}

static int zt_native_line_is_error_fatal(const char *line) {
    if (line == NULL) return 0;
    return strstr(line, ": fatal error:") != NULL;
}

static int zt_native_line_is_error(const char *line) {
    if (line == NULL) return 0;
    if (strstr(line, ": error:") != NULL) return 1;
    if (zt_native_line_is_error_undefined_reference(line)) return 1;
    if (strstr(line, "collect2: error:") != NULL) return 1;
    if (zt_native_line_is_compiler_not_found(line)) return 1;
    return 0;
}

static int zt_native_line_is_filtered_warning(const char *line) {
    if (line == NULL) return 0;
    if (strstr(line, "warning: unused parameter") != NULL &&
            strstr(line, "[-Wunused-parameter]") != NULL) {
        return 1;
    }
    return 0;
}

static int zt_native_line_is_warning_format_truncation(const char *line) {
    if (line == NULL) return 0;
    return strstr(line, "[-Wformat-truncation") != NULL;
}

static int zt_native_line_is_warning_sign_compare(const char *line) {
    if (line == NULL) return 0;
    return strstr(line, "[-Wsign-compare]") != NULL;
}

static int zt_native_line_is_warning_unused_variable(const char *line) {
    if (line == NULL) return 0;
    return strstr(line, "[-Wunused-variable]") != NULL;
}

static void zt_native_capture_first(char *dest, size_t capacity, const char *line) {
    if (dest == NULL || capacity == 0 || line == NULL) return;
    if (dest[0] != '\0') return;
    snprintf(dest, capacity, "%s", line);
}

#ifndef _WIN32
static int zt_native_decode_status(int status) {
#ifdef _WIN32
    return status;
#else
    if (status < 0) return status;
    if (WIFEXITED(status)) return WEXITSTATUS(status);
    return status;
#endif
}
#endif

static int zt_run_native_compile_command(zt_driver_context *ctx, const char *const *argv) {
    char capture_path[512];
    FILE *pipe;
    char line[2048];
    char first_warning_generic[512];
    char first_warning_format_truncation[512];
    char first_warning_sign_compare[512];
    char first_warning_unused_variable[512];
    char first_error_generic[512];
    char first_error_undefined_reference[512];
    char first_error_fatal[512];
    char first_error_compiler_not_found[512];
    char first_error[512];
    char first_other[512];
    size_t filtered_unused_parameter = 0;
    size_t warning_generic_count = 0;
    size_t warning_format_truncation_count = 0;
    size_t warning_sign_compare_count = 0;
    size_t warning_unused_variable_count = 0;
    size_t error_undefined_reference_count = 0;
    size_t error_fatal_count = 0;
    size_t error_compiler_not_found_count = 0;
    int spawn_failed = 0;
    int exit_code;

    first_warning_generic[0] = '\0';
    first_warning_format_truncation[0] = '\0';
    first_warning_sign_compare[0] = '\0';
    first_warning_unused_variable[0] = '\0';
    first_error_generic[0] = '\0';
    first_error_undefined_reference[0] = '\0';
    first_error_fatal[0] = '\0';
    first_error_compiler_not_found[0] = '\0';
    first_error[0] = '\0';
    first_other[0] = '\0';
    capture_path[0] = '\0';

    if (argv == NULL || argv[0] == NULL) return 1;

    if (ctx != NULL && ctx->native_raw_output) {
        return zt_native_spawn_process(argv, NULL, &spawn_failed);
    }

    if (!zt_native_make_temp_path(capture_path, sizeof(capture_path), "native-compile", ".log")) {
        return zt_native_spawn_process(argv, NULL, &spawn_failed);
    }

    exit_code = zt_native_spawn_process(argv, capture_path, &spawn_failed);

    pipe = fopen(capture_path, "rb");
    if (pipe == NULL) {
        return exit_code;
    }
    while (fgets(line, sizeof(line), pipe) != NULL) {
        zt_native_trim_line(line);
        if (line[0] == '\0') continue;

        if (zt_native_line_is_filtered_warning(line)) {
            filtered_unused_parameter += 1;
            continue;
        }

        if (zt_native_line_is_compiler_not_found(line)) {
            error_compiler_not_found_count += 1;
            zt_native_capture_first(first_error_compiler_not_found, sizeof(first_error_compiler_not_found), line);
            continue;
        }

        if (zt_native_line_is_warning(line)) {
            if (zt_native_line_is_warning_format_truncation(line)) {
                warning_format_truncation_count += 1;
                zt_native_capture_first(
                    first_warning_format_truncation,
                    sizeof(first_warning_format_truncation),
                    line);
            } else if (zt_native_line_is_warning_sign_compare(line)) {
                warning_sign_compare_count += 1;
                zt_native_capture_first(
                    first_warning_sign_compare,
                    sizeof(first_warning_sign_compare),
                    line);
            } else if (zt_native_line_is_warning_unused_variable(line)) {
                warning_unused_variable_count += 1;
                zt_native_capture_first(
                    first_warning_unused_variable,
                    sizeof(first_warning_unused_variable),
                    line);
            } else {
                warning_generic_count += 1;
                zt_native_capture_first(first_warning_generic, sizeof(first_warning_generic), line);
            }
            continue;
        }

        if (zt_native_line_is_error(line)) {
            if (zt_native_line_is_error_undefined_reference(line)) {
                error_undefined_reference_count += 1;
                zt_native_capture_first(
                    first_error_undefined_reference,
                    sizeof(first_error_undefined_reference),
                    line);
            } else if (zt_native_line_is_error_fatal(line)) {
                error_fatal_count += 1;
                zt_native_capture_first(first_error_fatal, sizeof(first_error_fatal), line);
            } else {
                zt_native_capture_first(first_error_generic, sizeof(first_error_generic), line);
            }
            continue;
        }

        zt_native_capture_first(first_other, sizeof(first_other), line);
    }

    fclose(pipe);
    zt_native_remove_file_if_exists(capture_path);

    if (ctx == NULL || !ctx->ci_mode_enabled) {
        if (filtered_unused_parameter > 0) {
            fprintf(stderr, "warning[native.unused_parameter]\n");
            fprintf(stderr, "ACTION\n  no action needed in Zenith source.\n");
            fprintf(stderr, "WHY\n  generated C keeps stable function signatures and may keep unused args.\n");
            fprintf(stderr, "NEXT\n  continue build. use --native-raw to inspect raw gcc output.\n");
            fprintf(stderr, "count\n  %zu occurrence(s) hidden.\n", filtered_unused_parameter);
        }

        if (warning_format_truncation_count > 0) {
            fprintf(stderr, "warning[native.format_truncation]\n");
            fprintf(stderr, "ACTION\n  review generated C buffers and snprintf usage in backend/runtime.\n");
            fprintf(stderr, "WHY\n  gcc detected possible output truncation during formatting.\n");
            fprintf(stderr, "NEXT\n  inspect backend.c emitter paths. use --native-raw for full context.\n");
            fprintf(stderr, "count\n  %zu occurrence(s).\n", warning_format_truncation_count);
            if (first_warning_format_truncation[0] != '\0') {
                fprintf(stderr, "sample\n  %s\n", first_warning_format_truncation);
            }
        }

        if (warning_sign_compare_count > 0) {
            fprintf(stderr, "warning[native.sign_compare]\n");
            fprintf(stderr, "ACTION\n  align signed/unsigned comparisons in generated C.\n");
            fprintf(stderr, "WHY\n  gcc found comparisons that may behave differently by platform.\n");
            fprintf(stderr, "NEXT\n  inspect emitted integer types in backend.c. use --native-raw for details.\n");
            fprintf(stderr, "count\n  %zu occurrence(s).\n", warning_sign_compare_count);
            if (first_warning_sign_compare[0] != '\0') {
                fprintf(stderr, "sample\n  %s\n", first_warning_sign_compare);
            }
        }

        if (warning_unused_variable_count > 0) {
            fprintf(stderr, "warning[native.unused_variable]\n");
            fprintf(stderr, "ACTION\n  review generated temporary variables in backend C output.\n");
            fprintf(stderr, "WHY\n  gcc detected variables that are written but never used.\n");
            fprintf(stderr, "NEXT\n  refine emitter flow for that construct. use --native-raw for raw lines.\n");
            fprintf(stderr, "count\n  %zu occurrence(s).\n", warning_unused_variable_count);
            if (first_warning_unused_variable[0] != '\0') {
                fprintf(stderr, "sample\n  %s\n", first_warning_unused_variable);
            }
        }

        if (warning_generic_count > 0) {
            fprintf(stderr, "warning[native.compiler]\n");
            fprintf(stderr, "ACTION\n  review native warnings if they affect your platform/toolchain.\n");
            fprintf(stderr, "WHY\n  gcc reported warnings while compiling generated C.\n");
            fprintf(stderr, "NEXT\n  rerun with --native-raw for full compiler output.\n");
            fprintf(stderr, "count\n  %zu occurrence(s).\n", warning_generic_count);
            if (first_warning_generic[0] != '\0') {
                fprintf(stderr, "sample\n  %s\n", first_warning_generic);
            }
        }

        if (exit_code != 0) {
            if (error_compiler_not_found_count > 0 || (spawn_failed && first_other[0] == '\0' && first_error_generic[0] == '\0')) {
                fprintf(stderr, "error[native.compiler.not_found]\n");
                fprintf(stderr, "ACTION\n  install gcc and ensure it is available in PATH.\n");
                fprintf(stderr, "WHY\n  the native compiler executable was not found.\n");
                fprintf(stderr, "NEXT\n  configure toolchain, then rerun build. use --native-raw for full output.\n");
                if (first_error_compiler_not_found[0] != '\0') {
                    fprintf(stderr, "first_error\n  %s\n", first_error_compiler_not_found);
                }
            } else if (error_undefined_reference_count > 0) {
                fprintf(stderr, "error[native.linker.undefined_reference]\n");
                fprintf(stderr, "ACTION\n  verify extern symbol names and linker flags.\n");
                fprintf(stderr, "WHY\n  the linker could not resolve one or more referenced symbols.\n");
                fprintf(stderr, "NEXT\n  review build.linker_flags and extern declarations. use --native-raw if needed.\n");
                fprintf(stderr, "count\n  %zu unresolved reference(s).\n", error_undefined_reference_count);
                if (first_error_undefined_reference[0] != '\0') {
                    fprintf(stderr, "first_error\n  %s\n", first_error_undefined_reference);
                }
            } else if (error_fatal_count > 0) {
                fprintf(stderr, "error[native.compiler.fatal]\n");
                fprintf(stderr, "ACTION\n  fix the first fatal compiler error in generated C.\n");
                fprintf(stderr, "WHY\n  gcc stopped after a fatal issue.\n");
                fprintf(stderr, "NEXT\n  rerun with --native-raw and inspect the file/line context.\n");
                if (first_error_fatal[0] != '\0') {
                    fprintf(stderr, "first_error\n  %s\n", first_error_fatal);
                }
            } else if (first_other[0] != '\0') {
                fprintf(stderr, "error[native.compiler]\n");
                fprintf(stderr, "ACTION\n  inspect native compiler output.\n");
                fprintf(stderr, "WHY\n  gcc failed while compiling/linking generated C.\n");
                fprintf(stderr, "NEXT\n  rerun with --native-raw for full output.\n");
                fprintf(stderr, "first_line\n  %s\n", first_other);
            } else {
                if (first_error_generic[0] != '\0') {
                    snprintf(first_error, sizeof(first_error), "%s", first_error_generic);
                }
                fprintf(stderr, "error[native.compiler]\n");
                fprintf(stderr, "ACTION\n  inspect native compiler output.\n");
                fprintf(stderr, "WHY\n  gcc failed while compiling/linking generated C.\n");
                fprintf(stderr, "NEXT\n  rerun with --native-raw for full output.\n");
                if (first_error[0] != '\0') {
                    fprintf(stderr, "first_error\n  %s\n", first_error);
                }
            }
        }
    }

    return exit_code;
}

static int zt_compile_runtime_object(zt_driver_context *ctx, const char *runtime_obj_path) {
    char runtime_c_path[1024];
    char runtime_root[768];
    char *runtime_include = NULL;
    const char *argv[10];
    int compile_result;

    if (!zt_make_dirs(".ztc-tmp/runtime")) {
        zt_print_single_diag(
            ctx,
            "backend.c.emit",
            ZT_DIAG_PROJECT_INVALID_INPUT,
            zt_source_span_unknown(),
            "failed to prepare runtime object cache directory");
        return 0;
    }

    if (!zt_join_runtime_dep(runtime_c_path, sizeof(runtime_c_path), "runtime/c/zenith_rt.c")) {
        zt_print_single_diag(
            ctx,
            "backend.c.emit",
            ZT_DIAG_PROJECT_PATH_TOO_LONG,
            zt_source_span_unknown(),
            "runtime source path is too long");
        return 0;
    }

    zt_runtime_root(runtime_root, sizeof(runtime_root));
    runtime_include = zt_native_dup_prefixed_arg("-I", runtime_root);
    if (runtime_include == NULL) {
        zt_print_single_diag(
            ctx,
            "backend.c.emit",
            ZT_DIAG_BACKEND_C_EMIT_ERROR,
            zt_source_span_unknown(),
            "failed to allocate runtime include flag");
        return 0;
    }

    argv[0] = "gcc";
    argv[1] = "-Wall";
    argv[2] = "-Wextra";
    argv[3] = "-Wno-unused-function";
    argv[4] = "-I.";
    argv[5] = runtime_include;
    argv[6] = "-c";
    argv[7] = runtime_c_path;
    argv[8] = "-o";
    argv[9] = runtime_obj_path;

    if (ctx == NULL || !ctx->ci_mode_enabled) {
        char display_cmd[2048];
        const char *display_argv[11];
        memcpy(display_argv, argv, sizeof(argv));
        display_argv[10] = NULL;
        if (zt_native_format_command(display_cmd, sizeof(display_cmd), display_argv)) {
            printf("compiling runtime cache: %s\n", display_cmd);
        } else {
            printf("compiling runtime cache with gcc\n");
        }
        fflush(stdout);
    }

    {
        const char *compile_argv[11];
        memcpy(compile_argv, argv, sizeof(argv));
        compile_argv[10] = NULL;
        compile_result = zt_run_native_compile_command(ctx, compile_argv);
    }
    free(runtime_include);
    if (compile_result != 0) {
        zt_print_single_diag(
            ctx,
            "backend.c.emit",
            ZT_DIAG_BACKEND_C_EMIT_ERROR,
            zt_source_span_make(runtime_c_path, 1, 1, 1),
            "runtime object compilation failed with code %d",
            compile_result);
        return 0;
    }

    return 1;
}

static int zt_ensure_runtime_object_current(zt_driver_context *ctx, const char *runtime_obj_path) {
    char lock_path[512];
    int needs_compile;
    int ok = 1;
    int lock_owned = 0;

    if (!zt_make_dirs(".ztc-tmp/runtime")) {
        zt_print_single_diag(
            ctx,
            "backend.c.emit",
            ZT_DIAG_PROJECT_INVALID_INPUT,
            zt_source_span_unknown(),
            "failed to prepare runtime object cache directory");
        return 0;
    }

    if (snprintf(lock_path, sizeof(lock_path), "%s.lock", runtime_obj_path) >= (int)sizeof(lock_path)) {
        zt_print_single_diag(
            ctx,
            "backend.c.emit",
            ZT_DIAG_PROJECT_PATH_TOO_LONG,
            zt_source_span_unknown(),
            "runtime cache lock path is too long");
        return 0;
    }

    if (!zt_runtime_object_lock_acquire(ctx, runtime_obj_path, lock_path, &lock_owned)) {
        return 0;
    }

    needs_compile = zt_runtime_object_is_stale(runtime_obj_path);
    if (needs_compile) {
        ok = zt_compile_runtime_object(ctx, runtime_obj_path);
    }

    if (lock_owned) {
        zt_runtime_object_lock_release(lock_path);
    }

    return ok;
}

int zt_compile_c_file(
        zt_driver_context *ctx,
        const char *c_path,
        const char *exe_path,
        const zt_project_manifest *manifest) {
    const char *runtime_obj_path = ".ztc-tmp/runtime/zenith_rt.o";
    char runtime_root[768];
    char response_path[512];
    char *runtime_include = NULL;
    char *response_arg = NULL;
    const char *extra_linker_flags = "";
    const char *argv[16];
    size_t argv_count = 0;
    int compile_result;

    if (manifest != NULL && manifest->build_linker_flags[0] != '\0') {
        extra_linker_flags = manifest->build_linker_flags;
    }

    if (!zt_ensure_runtime_object_current(ctx, runtime_obj_path)) {
        return 0;
    }

    zt_runtime_root(runtime_root, sizeof(runtime_root));
    runtime_include = zt_native_dup_prefixed_arg("-I", runtime_root);
    if (runtime_include == NULL) {
        zt_print_single_diag(
            ctx,
            "backend.c.emit",
            ZT_DIAG_BACKEND_C_EMIT_ERROR,
            zt_source_span_unknown(),
            "failed to allocate runtime include flag");
        return 0;
    }

    response_path[0] = '\0';
    if (extra_linker_flags[0] != '\0') {
        if (!zt_native_make_temp_path(response_path, sizeof(response_path), "native-link-flags", ".rsp")) {
            free(runtime_include);
            zt_print_single_diag(
                ctx,
                "backend.c.emit",
                ZT_DIAG_BACKEND_C_EMIT_ERROR,
                zt_source_span_unknown(),
                "failed to prepare native linker response file");
            return 0;
        }
        if (!zt_write_file(response_path, extra_linker_flags)) {
            free(runtime_include);
            zt_print_single_diag(
                ctx,
                "backend.c.emit",
                ZT_DIAG_BACKEND_C_EMIT_ERROR,
                zt_source_span_unknown(),
                "failed to write native linker response file");
            return 0;
        }
        response_arg = zt_native_dup_prefixed_arg("@", response_path);
        if (response_arg == NULL) {
            free(runtime_include);
            zt_native_remove_file_if_exists(response_path);
            zt_print_single_diag(
                ctx,
                "backend.c.emit",
                ZT_DIAG_BACKEND_C_EMIT_ERROR,
                zt_source_span_unknown(),
                "failed to allocate native linker response argument");
            return 0;
        }
    }

    argv[argv_count++] = "gcc";
    argv[argv_count++] = "-Wall";
    argv[argv_count++] = "-Wextra";
    argv[argv_count++] = "-Wno-unused-function";
    argv[argv_count++] = "-I.";
    argv[argv_count++] = runtime_include;
    argv[argv_count++] = "-o";
    argv[argv_count++] = exe_path;
    argv[argv_count++] = c_path;
    argv[argv_count++] = runtime_obj_path;
    if (response_arg != NULL) {
        argv[argv_count++] = response_arg;
    }
#ifdef _WIN32
    argv[argv_count++] = "-lws2_32";
#else
    argv[argv_count++] = "-lm";
    argv[argv_count++] = "-ldl";
#endif
    argv[argv_count] = NULL;

    if (ctx == NULL || !ctx->ci_mode_enabled) {
        char display_cmd[3072];
        if (zt_native_format_command(display_cmd, sizeof(display_cmd), argv)) {
            printf("compiling: %s\n", display_cmd);
        } else {
            printf("compiling with gcc\n");
        }
        fflush(stdout);
    }
    compile_result = zt_run_native_compile_command(ctx, argv);
    free(runtime_include);
    free(response_arg);
    zt_native_remove_file_if_exists(response_path);
    if (compile_result != 0) {
        zt_print_single_diag(
            ctx,
            "backend.c.emit",
            ZT_DIAG_BACKEND_C_EMIT_ERROR,
            zt_source_span_make(c_path != NULL ? c_path : "<c>", 1, 1, 1),
            "native compilation failed with code %d",
            compile_result);
        return 0;
    }

    if (ctx == NULL || !ctx->ci_mode_enabled) {
        printf("built: %s\n", exe_path);
    }
    return 1;
}

int zt_decode_process_exit(int status) {
#ifdef _WIN32
    return status;
#else
    if (WIFEXITED(status)) return WEXITSTATUS(status);
    return status;
#endif
}

int zt_run_executable(zt_driver_context *ctx, const char *exe_path) {
    char system_path[1000];
    const char *argv[2];
    int run_result;

    zt_normalize_system_path(exe_path, system_path, sizeof(system_path));
    if (ctx == NULL || !ctx->ci_mode_enabled) {
        printf("running: %s\n", system_path);
    }
    argv[0] = system_path;
    argv[1] = NULL;
    run_result = zt_native_spawn_process(argv, NULL, NULL);
    if (run_result < 0) {
        zt_print_single_diag(
            ctx,
            "runtime",
            ZT_DIAG_PROJECT_INVALID_INPUT,
            zt_source_span_unknown(),
            "failed to execute '%s'",
            system_path);
        return -1;
    }

    if (ctx == NULL || !ctx->ci_mode_enabled) {
        printf("exit code: %d\n", run_result);
    }
    return run_result;
}
int zt_collect_project_sources(
        zt_driver_context *ctx,
        const char *input_path,
        zt_project_manifest *manifest,
        char *project_root,
        size_t project_root_capacity,
        zt_project_source_file_list *source_files) {
    zt_project_parse_result project;
    char manifest_path[512];
    char source_root_path[512];

    if (manifest == NULL || project_root == NULL || source_files == NULL) return 0;

    zt_project_source_file_list_init(source_files);

    if (!zt_resolve_project_paths(
            input_path,
            project_root,
            project_root_capacity,
            manifest_path,
            sizeof(manifest_path))) {
        return 0;
    }

    if (!zt_project_load_file(manifest_path, &project)) {
        zt_print_project_parse_error(ctx, manifest_path, &project);
        return 0;
    }

    *manifest = project.manifest;
    zt_apply_manifest_lang(manifest);

    if (!zt_join_path(source_root_path, sizeof(source_root_path), project_root, manifest->source_root)) {
        zt_print_single_diag(ctx, "project", ZT_DIAG_PROJECT_PATH_TOO_LONG, zt_source_span_unknown(), "source.root path is too long");
        return 0;
    }

    if (!zt_path_is_dir(source_root_path)) {
        zt_print_single_diag(ctx, "project", ZT_DIAG_PROJECT_INVALID_INPUT, zt_source_span_unknown(), "source.root '%s' is not a directory", source_root_path);
        return 0;
    }

    if (!zt_project_discover_zt_files(source_root_path, source_files)) {
        zt_project_source_file_list_dispose(source_files);
        return 0;
    }
    zt_project_discover_packages(project_root, source_files);

    if (source_files->count == 0) {
        zt_print_single_diag(ctx, "project", ZT_DIAG_PROJECT_ERROR, zt_source_span_unknown(), "source.root '%s' contains no .zt files", source_root_path);
        zt_project_source_file_list_dispose(source_files);
        return 0;
    }

    if (!zt_parse_project_sources(ctx, source_files)) {
        zt_project_source_file_list_dispose(source_files);
        return 0;
    }

    {
        zt_diag_list proj_diags = zt_diag_list_make();
        if (!zt_validate_source_namespaces(source_files, manifest, &proj_diags)) {
            zt_print_diagnostics(ctx, "project", &proj_diags);
            zt_diag_list_dispose(&proj_diags);
            zt_project_source_file_list_dispose(source_files);
            return 0;
        }
        zt_diag_list_dispose(&proj_diags);
    }

    return 1;
}
