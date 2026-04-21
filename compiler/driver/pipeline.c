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
                "project",
                ZT_DIAG_PROJECT_MONOMORPHIZATION_LIMIT_EXCEEDED,
                span,
                "build.monomorphization_limit exceeded: found %zu generic instantiations, limit is %zu (examples: %s)",
                generic_instances.count,
                monomorphization_limit,
                preview);
        } else {
            zt_print_single_diag(
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

typedef struct zt_project_compile_result {
    zt_project_manifest manifest;
    char project_root[512];
    char manifest_path[512];
    char entry_path[512];
    zt_project_source_file_list source_files;
    zir_lower_result zir;
    int has_zir;
} zt_project_compile_result;

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

int zt_compile_project(const char *input_path, zt_project_compile_result *out) {
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

    if (!zt_project_load_file(out->manifest_path, &project)) {
        zt_print_project_parse_error(out->manifest_path, &project);
        return 0;
    }

    out->manifest = project.manifest;
    zt_apply_manifest_lang(&out->manifest);

    /* Set global accessibility state */
    zt_active_manifest = &out->manifest;
    if (!zt_use_action_first) {
        zt_active_profile = zt_cog_profile_from_text(out->manifest.accessibility_profile);
        if (zt_active_profile != ZT_COG_PROFILE_FULL) {
            zt_use_action_first = 1;
        }
    }
    zt_telemetry_enabled = out->manifest.accessibility_telemetry;
    zt_copy_text(zt_project_root_abs, sizeof(zt_project_root_abs), out->project_root);

    if (zt_project_manifest_kind(&out->manifest) == ZT_PROJECT_KIND_APP &&
            !zt_project_resolve_entry_source_path(
                &out->manifest,
                out->project_root,
                out->entry_path,
                sizeof(out->entry_path))) {
        zt_print_single_diag("project", ZT_DIAG_PROJECT_PATH_TOO_LONG, zt_source_span_unknown(), "app.entry source path is too long");
        return 0;
    }

    if (!zt_join_path(source_root_path, sizeof(source_root_path), out->project_root, out->manifest.source_root)) {
        zt_print_single_diag("project", ZT_DIAG_PROJECT_PATH_TOO_LONG, zt_source_span_unknown(), "source.root path is too long");
        goto fail;
    }

    if (!zt_path_is_dir(source_root_path)) {
        zt_print_single_diag("project", ZT_DIAG_PROJECT_INVALID_INPUT, zt_source_span_unknown(), "source.root '%s' is not a directory", source_root_path);
        goto fail;
    }

    if (!zt_project_discover_zt_files(source_root_path, &out->source_files)) {
        goto fail;
    }

    /* Load only the specific stdlib modules that are actually imported */
    {
        const char *stdlib_base = getenv("ZENITH_HOME");
        char default_base[768];
        char import_buf[256];
        size_t si;

        if (stdlib_base == NULL) {
            strcpy(default_base, "stdlib");
            stdlib_base = default_base;
        } else {
            if (!zt_join_path(default_base, sizeof(default_base), stdlib_base, "stdlib")) {
                strcpy(default_base, "stdlib");
            }
            stdlib_base = default_base;
        }

        for (si = 0; si < out->source_files.count; si++) {
            const char *src_path = out->source_files.items[si].path;
            FILE *sf = fopen(src_path, "r");
            if (sf != NULL) {
                char line[512];
                while (fgets(line, sizeof(line), sf) != NULL) {
                    /* Look for: import std.X or import std.X as Y */
                    const char *imp = strstr(line, "import std.");
                    if (imp != NULL) {
                        const char *mod_start = imp + 7; /* skip "import " -> "std.X..." */
                        const char *end = mod_start;
                        char mod_path[768];
                        size_t j;
                        int already_loaded;

                        while (*end && *end != ' ' && *end != '\t' && *end != '\n' && *end != '\r') end++;
                        if ((size_t)(end - mod_start) < sizeof(import_buf)) {
                            memcpy(import_buf, mod_start, end - mod_start);
                            import_buf[end - mod_start] = '\0';

                            /* Convert std.io -> stdlib_base/std/io.zt */
                            snprintf(mod_path, sizeof(mod_path), "%s", stdlib_base);
                            for (j = 0; import_buf[j]; j++) {
                                if (import_buf[j] == '.') import_buf[j] = '/';
                            }
                            if (strlen(mod_path) + 1 + strlen(import_buf) + 3 < sizeof(mod_path)) {
                                strcat(mod_path, "/");
                                strcat(mod_path, import_buf);
                                strcat(mod_path, ".zt");
                            }

                            /* Check if already loaded */
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
                }
                fclose(sf);
            }
        }
    }

    if (out->source_files.count == 0) {
        zt_print_single_diag("project", ZT_DIAG_PROJECT_ERROR, zt_source_span_unknown(), "source.root '%s' contains no .zt files", source_root_path);
        goto fail;
    }

    if (!zt_parse_project_sources(&out->source_files)) {
        goto fail;
    }

    zt_diag_list proj_diags = zt_diag_list_make();
    if (!zt_validate_source_namespaces(&out->source_files, &out->manifest, &proj_diags)) {
        zt_print_diagnostics("project", &proj_diags);
        zt_diag_list_dispose(&proj_diags);
        goto fail;
    }
    zt_diag_list_dispose(&proj_diags);

    program_root = zt_build_combined_project_ast(&out->source_files, &out->manifest);
    if (program_root == NULL) {
        zt_print_single_diag("project", ZT_DIAG_PROJECT_ERROR, zt_source_span_unknown(), "unable to aggregate project sources");
        goto fail;
    }
    program_ready = 1;

    bound = zt_bind_file(program_root);
    bound_ready = 1;
    if (bound.diagnostics.count != 0) {
        zt_print_diagnostics("binding", &bound.diagnostics);
        goto fail;
    }

    checked = zt_check_file(program_root);
    checked_ready = 1;
    if (checked.diagnostics.count != 0) {
        zt_print_diagnostics("type", &checked.diagnostics);
        goto fail;
    }

    hir = zt_lower_ast_to_hir(program_root);
    hir_ready = 1;
    if (hir.diagnostics.count != 0 || hir.module == NULL) {
        zt_print_diagnostics("hir", &hir.diagnostics);
        goto fail;
    }

    zir = zir_lower_hir_to_zir(hir.module);
    zir_ready = 1;
    if (zir.diagnostics.count != 0) {
        zt_print_diagnostics("zir", &zir.diagnostics);
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
            "zir.verify",
            zt_diag_code_from_zir_verifier(verifier.code),
            span,
            "%s",
            verifier.message[0] != '\0' ? verifier.message : "ZIR verification failed");
        goto fail;
    }

    if (!zt_enforce_monomorphization_limit(out->manifest_path, out->manifest.build_monomorphization_limit, &zir.module)) {
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

int zt_compile_c_file(const char *c_path, const char *exe_path) {
    char compile_cmd[2048];
    int compile_result;

#ifdef _WIN32
    snprintf(compile_cmd, sizeof(compile_cmd),
             "gcc -Wall -Wextra -Wno-unused-function -I. -o \"%s\" \"%s\" runtime/c/zenith_rt.c -lws2_32",
             exe_path, c_path);
#else
    snprintf(compile_cmd, sizeof(compile_cmd),
             "gcc -Wall -Wextra -Wno-unused-function -I. -o \"%s\" \"%s\" runtime/c/zenith_rt.c -lm",
             exe_path, c_path);
#endif

    if (!zt_ci_mode_enabled) {
        printf("compiling: %s\n", compile_cmd);
    }
    compile_result = system(compile_cmd);
    if (compile_result != 0) {
        zt_print_single_diag(
            "backend.c.emit",
            ZT_DIAG_BACKEND_C_EMIT_ERROR,
            zt_source_span_make(c_path != NULL ? c_path : "<c>", 1, 1, 1),
            "native compilation failed with code %d",
            compile_result);
        return 0;
    }

    if (!zt_ci_mode_enabled) {
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

int zt_run_executable(const char *exe_path) {
    char run_cmd[1024];
    char system_path[1000];
    int run_result;
    int exit_code;

    zt_normalize_system_path(exe_path, system_path, sizeof(system_path));

    snprintf(run_cmd, sizeof(run_cmd), "\"%s\"", system_path);
    if (!zt_ci_mode_enabled) {
        printf("running: %s\n", system_path);
    }
    run_result = system(run_cmd);
    if (run_result < 0) {
        zt_print_single_diag(
            "runtime",
            ZT_DIAG_PROJECT_INVALID_INPUT,
            zt_source_span_unknown(),
            "failed to execute '%s'",
            system_path);
        return -1;
    }

    exit_code = zt_decode_process_exit(run_result);
    if (!zt_ci_mode_enabled) {
        printf("exit code: %d\n", exit_code);
    }
    return exit_code;
}
int zt_collect_project_sources(
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
        zt_print_project_parse_error(manifest_path, &project);
        return 0;
    }

    *manifest = project.manifest;
    zt_apply_manifest_lang(manifest);

    if (!zt_join_path(source_root_path, sizeof(source_root_path), project_root, manifest->source_root)) {
        zt_print_single_diag("project", ZT_DIAG_PROJECT_PATH_TOO_LONG, zt_source_span_unknown(), "source.root path is too long");
        return 0;
    }

    if (!zt_path_is_dir(source_root_path)) {
        zt_print_single_diag("project", ZT_DIAG_PROJECT_INVALID_INPUT, zt_source_span_unknown(), "source.root '%s' is not a directory", source_root_path);
        return 0;
    }

    if (!zt_project_discover_zt_files(source_root_path, source_files)) {
        zt_project_source_file_list_dispose(source_files);
        return 0;
    }

    if (source_files->count == 0) {
        zt_print_single_diag("project", ZT_DIAG_PROJECT_ERROR, zt_source_span_unknown(), "source.root '%s' contains no .zt files", source_root_path);
        zt_project_source_file_list_dispose(source_files);
        return 0;
    }

    if (!zt_parse_project_sources(source_files)) {
        zt_project_source_file_list_dispose(source_files);
        return 0;
    }

    {
        zt_diag_list proj_diags = zt_diag_list_make();
        if (!zt_validate_source_namespaces(source_files, manifest, &proj_diags)) {
            zt_print_diagnostics("project", &proj_diags);
            zt_diag_list_dispose(&proj_diags);
            zt_project_source_file_list_dispose(source_files);
            return 0;
        }
        zt_diag_list_dispose(&proj_diags);
    }

    return 1;
}
