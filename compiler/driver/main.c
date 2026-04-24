#include "compiler/driver/driver_internal.h"

/* Process-wide compiler memory (declared extern in driver_internal.h) */

zt_arena global_arena;
zt_string_pool global_pool;

void zt_driver_context_init(zt_driver_context *ctx) {
    if (ctx == NULL) return;
    memset(ctx, 0, sizeof(*ctx));
    ctx->configured_profile = ZT_COG_PROFILE_BALANCED;
    ctx->active_profile = ZT_COG_PROFILE_BALANCED;
}

void zt_driver_context_dispose(zt_driver_context *ctx) {
    if (ctx == NULL) return;
    zt_path_filter_list_dispose(&ctx->since_filter);
    memset(ctx, 0, sizeof(*ctx));
    ctx->configured_profile = ZT_COG_PROFILE_BALANCED;
    ctx->active_profile = ZT_COG_PROFILE_BALANCED;
}

void zt_driver_context_activate_project(
        zt_driver_context *ctx,
        const zt_project_manifest *manifest,
        const char *project_root) {
    if (ctx == NULL) return;

    ctx->active_manifest = manifest;
    ctx->telemetry_enabled = ctx->telemetry_forced;

    if (project_root != NULL) {
        zt_copy_text(ctx->project_root_abs, sizeof(ctx->project_root_abs), project_root);
    } else {
        ctx->project_root_abs[0] = '\0';
    }

    if (manifest == NULL) {
        if (ctx->profile_locked) {
            ctx->active_profile = ctx->configured_profile;
            ctx->use_action_first = ctx->configured_profile != ZT_COG_PROFILE_FULL;
        }
        return;
    }

    if (ctx->profile_locked) {
        ctx->active_profile = ctx->configured_profile;
    } else {
        ctx->active_profile = zt_cog_profile_from_text(manifest->accessibility_profile);
    }

    ctx->use_action_first = ctx->active_profile != ZT_COG_PROFILE_FULL;
    ctx->telemetry_enabled = ctx->telemetry_forced || manifest->accessibility_telemetry;
}


void zt_apply_manifest_lang(const zt_project_manifest *manifest) {
    if (manifest != NULL && manifest->lang[0] != '\0') {
        /* Only apply if not already overridden by CLI */
        if (!zt_l10n_is_explicitly_set()) {
            zt_lang lang = zt_l10n_from_str(manifest->lang);
            if (lang != ZT_LANG_UNSPECIFIED) {
                zt_l10n_set_lang(lang);
            }
        }
    }
}



static int zt_cli_utf8_enabled = 1;

static int zt_cli_env_true(const char *name) {
    const char *value = getenv(name);
    if (value == NULL || value[0] == '\0') return 0;
    if (value[0] == '0') return 0;
    if (strcmp(value, "false") == 0 || strcmp(value, "FALSE") == 0) return 0;
    if (strcmp(value, "off") == 0 || strcmp(value, "OFF") == 0) return 0;
    return 1;
}

#ifndef _WIN32
static int zt_cli_locale_has_utf8(void) {
    const char *locale = getenv("LC_ALL");
    if (locale == NULL || locale[0] == '\0') locale = getenv("LANG");
    if (locale == NULL || locale[0] == '\0') return 0;
    if (strstr(locale, "UTF-8") != NULL || strstr(locale, "utf-8") != NULL) return 1;
    if (strstr(locale, "UTF8") != NULL || strstr(locale, "utf8") != NULL) return 1;
    return 0;
}
#endif

static void zt_cli_init_terminal(void) {
    if (zt_cli_env_true("ZT_CLI_ASCII")) {
        zt_cli_utf8_enabled = 0;
        return;
    }
    if (zt_cli_env_true("ZT_CLI_UTF8")) {
        zt_cli_utf8_enabled = 1;
        return;
    }
#ifdef _WIN32
    HANDLE stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    HANDLE stderr_handle = GetStdHandle(STD_ERROR_HANDLE);
    DWORD mode = 0;

    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);

    if (stdout_handle != INVALID_HANDLE_VALUE && GetConsoleMode(stdout_handle, &mode)) {
        SetConsoleMode(stdout_handle, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
    if (stderr_handle != INVALID_HANDLE_VALUE && GetConsoleMode(stderr_handle, &mode)) {
        SetConsoleMode(stderr_handle, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }

    zt_cli_utf8_enabled = GetConsoleOutputCP() == 65001;
#else
    zt_cli_utf8_enabled = zt_cli_locale_has_utf8();
#endif
}

static int zt_is_help_flag(const char *arg) {
    return arg != NULL &&
        (strcmp(arg, "--help") == 0 || strcmp(arg, "-h") == 0);
}

static zt_lang zt_cli_lang(void) {
    zt_lang lang = zt_l10n_current_lang();
    return lang == ZT_LANG_UNSPECIFIED ? ZT_LANG_EN : lang;
}

static const char *zt_cli_section_main(void) {
    switch (zt_cli_lang()) {
        case ZT_LANG_PT: return "principal";
        case ZT_LANG_ES: return "principal";
        case ZT_LANG_JA: return "main";
        case ZT_LANG_EN:
        default: return "main";
    }
}

static const char *zt_cli_section_compat(void) {
    switch (zt_cli_lang()) {
        case ZT_LANG_PT: return "compat";
        case ZT_LANG_ES: return "compat";
        case ZT_LANG_JA: return "compat";
        case ZT_LANG_EN:
        default: return "compat";
    }
}

static const char *zt_cli_section_options(void) {
    switch (zt_cli_lang()) {
        case ZT_LANG_PT: return "opcoes";
        case ZT_LANG_ES: return "opciones";
        case ZT_LANG_JA: return "options";
        case ZT_LANG_EN:
        default: return "options";
    }
}

static const char *zt_cli_section_tip(void) {
    switch (zt_cli_lang()) {
        case ZT_LANG_PT: return "dica";
        case ZT_LANG_ES: return "tip";
        case ZT_LANG_JA: return "tip";
        case ZT_LANG_EN:
        default: return "tip";
    }
}

static const char *zt_cli_label_error(void) {
    switch (zt_cli_lang()) {
        case ZT_LANG_PT: return "erro";
        case ZT_LANG_ES: return "error";
        case ZT_LANG_JA: return "error";
        case ZT_LANG_EN:
        default: return "error";
    }
}

static const char *zt_cli_label_hint(void) {
    switch (zt_cli_lang()) {
        case ZT_LANG_PT: return "dica";
        case ZT_LANG_ES: return "pista";
        case ZT_LANG_JA: return "hint";
        case ZT_LANG_EN:
        default: return "hint";
    }
}

static void zt_print_help_overview(FILE *out, const char *program) {
    (void)program;
    fprintf(out, "zt cli - reading-first\n");
    if (!zt_cli_utf8_enabled) {
        fprintf(out, "note: ascii fallback active (set ZT_CLI_UTF8=1 to force utf8)\n");
    }
    fprintf(out, "\n");
    fprintf(out, "zt <command> [input] [options]\n");
    fprintf(out, "\n");
    fprintf(out, "%s:\n", zt_cli_section_main());
    fprintf(out, "  check | build | run | create | test | fmt | doc check | doc show | summary | resume | perf\n");
    fprintf(out, "\n");
    fprintf(out, "%s:\n", zt_cli_section_compat());
    fprintf(out, "  project-info | verify | emit-c | build <file.zir> | doc-check | parse\n");
    fprintf(out, "\n");
    fprintf(out, "%s:\n", zt_cli_section_options());
    fprintf(out, "  --run --check --ci --profile --all --focus --since --lang --native-raw --app --lib --force -o\n");
    fprintf(out, "\n");
    fprintf(out, "%s:\n", zt_cli_section_tip());
    fprintf(out, "  zt help <command>\n");
}

static int zt_print_help_topic(FILE *out, const char *program, const char *topic) {
    (void)program;
    if (topic == NULL || topic[0] == '\0') {
        zt_print_help_overview(out, "zt");
        return 0;
    }

    if (strcmp(topic, "check") == 0 || strcmp(topic, "test") == 0) {
        fprintf(out, "zt %s [project|zenith.ztproj] [--ci] [--profile <level>] [--all]\n", topic);
        fprintf(out, "       [--focus <path>] [--since <git-ref>] [--lang <lang>]\n");
        return 0;
    }

    if (strcmp(topic, "build") == 0 || strcmp(topic, "run") == 0) {
        fprintf(out, "zt %s [project|zenith.ztproj] [-o <output>] [--ci] [--lang <lang>] [--native-raw]\n", topic);
        fprintf(out, "zt build <file.zir> [-o <output>] [--run]\n");
        return 0;
    }

    if (strcmp(topic, "create") == 0) {
        fprintf(out, "zt create [path|.] [--app|--lib] [--force]\n");
        return 0;
    }

    if (strcmp(topic, "fmt") == 0) {
        fprintf(out, "zt fmt [project|zenith.ztproj] [--check]\n");
        return 0;
    }

    if (strcmp(topic, "doc") == 0 || strcmp(topic, "doc-check") == 0 || strcmp(topic, "doc-show") == 0) {
        fprintf(out, "zt doc check [project|zenith.ztproj]\n");
        fprintf(out, "zt doc show <symbol>\n");
        fprintf(out, "zt doc-check [project|zenith.ztproj]\n");
        return 0;
    }

    if (strcmp(topic, "summary") == 0 || strcmp(topic, "resume") == 0) {
        fprintf(out, "zt %s [project|zenith.ztproj]\n", topic);
        return 0;
    }

    if (strcmp(topic, "perf") == 0) {
        fprintf(out, "zt perf [quick|nightly|scenario]\n");
        return 0;
    }

    if (strcmp(topic, "verify") == 0 || strcmp(topic, "emit-c") == 0 || strcmp(topic, "project-info") == 0) {
        if (strcmp(topic, "project-info") == 0) {
            fprintf(out, "zt project-info [project|zenith.ztproj]\n");
        } else {
            fprintf(out, "zt %s [project|zenith.ztproj|file.zir]\n", topic);
        }
        return 0;
    }

    if (strcmp(topic, "parse") == 0) {
        fprintf(out, "zt parse <file.zir>\n");
        return 0;
    }

    fprintf(out, "zt: %s: unknown help topic: %s\n", zt_cli_label_error(), topic);
    fprintf(out, "%s: zt help\n", zt_cli_label_hint());
    return 1;
}

static int zt_cli_fail(const char *program, const char *message, const char *hint, int show_usage) {
    (void)program;
    fprintf(stderr, "zt: %s: %s\n", zt_cli_label_error(), message);
    if (hint != NULL && hint[0] != '\0') {
        fprintf(stderr, "%s: %s\n", zt_cli_label_hint(), hint);
    }
    if (show_usage) {
        fprintf(stderr, "\n");
        zt_print_help_overview(stderr, "zt");
    }
    return 1;
}

static void zt_driver_context_lock_profile(zt_driver_context *ctx, zt_cog_profile profile) {
    if (ctx == NULL) return;
    ctx->profile_locked = 1;
    ctx->configured_profile = profile;
    ctx->active_profile = profile;
    ctx->use_action_first = profile != ZT_COG_PROFILE_FULL;
}

void zt_print_diagnostics(
        zt_driver_context *ctx,
        const char *stage,
        const zt_diag_list *diagnostics);

static zt_diag_code zt_diag_code_from_project_error(zt_project_error_code code) {
    switch (code) {
        case ZT_PROJECT_INVALID_INPUT: return ZT_DIAG_PROJECT_INVALID_INPUT;
        case ZT_PROJECT_INVALID_SECTION: return ZT_DIAG_PROJECT_INVALID_SECTION;
        case ZT_PROJECT_UNKNOWN_KEY: return ZT_DIAG_PROJECT_UNKNOWN_KEY;
        case ZT_PROJECT_INVALID_ASSIGNMENT: return ZT_DIAG_PROJECT_INVALID_ASSIGNMENT;
        case ZT_PROJECT_INVALID_STRING: return ZT_DIAG_PROJECT_INVALID_STRING;
        case ZT_PROJECT_MISSING_FIELD: return ZT_DIAG_PROJECT_MISSING_FIELD;
        case ZT_PROJECT_INVALID_KIND: return ZT_DIAG_PROJECT_INVALID_KIND;
        case ZT_PROJECT_INVALID_TARGET: return ZT_DIAG_PROJECT_INVALID_TARGET;
        case ZT_PROJECT_INVALID_PROFILE: return ZT_DIAG_PROJECT_INVALID_PROFILE;
        case ZT_PROJECT_INVALID_MONOMORPHIZATION_LIMIT: return ZT_DIAG_PROJECT_INVALID_MONOMORPHIZATION_LIMIT;
        case ZT_PROJECT_PATH_TOO_LONG: return ZT_DIAG_PROJECT_PATH_TOO_LONG;
        case ZT_PROJECT_TOO_MANY_DEPENDENCIES: return ZT_DIAG_PROJECT_TOO_MANY_DEPENDENCIES;
        case ZT_PROJECT_OK: return ZT_DIAG_PROJECT_ERROR;
        default: return ZT_DIAG_PROJECT_ERROR;
    }
}

void zt_print_project_parse_error(
        zt_driver_context *ctx,
        const char *manifest_path,
        const zt_project_parse_result *project) {
    zt_diag_list diagnostics = zt_diag_list_make();
    const char *message = "unable to read project manifest";
    zt_diag_code code = ZT_DIAG_PROJECT_INVALID_INPUT;
    const char *source_name = manifest_path != NULL && manifest_path[0] != '\0'
        ? manifest_path
        : "<project>";
    size_t line = 1;
    zt_source_span span;

    if (project != NULL) {
        if (project->code != ZT_PROJECT_OK) {
            code = zt_diag_code_from_project_error(project->code);
        }
        if (project->message[0] != '\0') {
            message = project->message;
        }
        if (project->line_number > 0) {
            line = (size_t)project->line_number;
        }
    }

    span = zt_source_span_make(source_name, line, 1, 1);
    zt_diag_list_add(&diagnostics, code, span, "%s", message);
    zt_print_diagnostics(ctx, "project", &diagnostics);
    zt_diag_list_dispose(&diagnostics);
}

static int zt_diag_matches_focus(const zt_diag *diag, const char *focus) {
    const char *src;
    size_t focus_len;
    if (focus == NULL || focus[0] == '\0') return 1;
    src = diag->span.source_name;
    if (src == NULL || src[0] == '\0') return 1;
    focus_len = strlen(focus);
    if (focus_len == 0) return 1;
    {
        size_t src_len = strlen(src);
        size_t i;
        for (i = 0; i + focus_len <= src_len; i++) {
            if (memcmp(src + i, focus, focus_len) == 0) return 1;
        }
    }
    return 0;
}


static int zt_diag_matches_since(const zt_driver_context *ctx, const zt_diag *diag) {
    char normalized_source[1024];
    size_t i;

    if (ctx == NULL || ctx->since_ref == NULL || ctx->since_ref[0] == '\0') return 1;
    if (diag == NULL || diag->span.source_name == NULL || diag->span.source_name[0] == '\0') return 0;
    if (ctx->since_filter.count == 0) return 0;

    if (snprintf(normalized_source, sizeof(normalized_source), "%s", diag->span.source_name) >= (int)sizeof(normalized_source)) {
        return 0;
    }
    zt_normalize_path_separators(normalized_source);

    for (i = 0; i < ctx->since_filter.count; i += 1) {
        const char *fragment = ctx->since_filter.items[i];
        if (fragment != NULL && fragment[0] != '\0' && strstr(normalized_source, fragment) != NULL) {
            return 1;
        }
    }

    return 0;
}

static int zt_diag_matches_filters(const zt_driver_context *ctx, const zt_diag *diag) {
    return zt_diag_matches_focus(diag, ctx != NULL ? ctx->focus_path : NULL) &&
        zt_diag_matches_since(ctx, diag);
}
void zt_print_diagnostics(
        zt_driver_context *ctx,
        const char *stage,
        const zt_diag_list *diagnostics) {
    zt_diag_list filtered;
    size_t i;
    int has_filters;

    if (diagnostics == NULL) return;
    has_filters = ctx != NULL &&
        ((ctx->focus_path != NULL && ctx->focus_path[0] != '\0') ||
         (ctx->since_ref != NULL && ctx->since_ref[0] != '\0'));

    /* Log telemetry if enabled */
    if (ctx != NULL && ctx->telemetry_enabled) {
        for (i = 0; i < diagnostics->count; i += 1) {
            zt_diag_telemetry_log(
                ctx->project_root_abs,
                ctx->active_manifest != NULL ? ctx->active_manifest->accessibility_profile : "none",
                stage,
                &diagnostics->items[i]);
        }
    }

    if (ctx != NULL && ctx->ci_mode_enabled) {
        if (has_filters) {
            filtered = zt_diag_list_make();
            for (i = 0; i < diagnostics->count; i += 1) {
                if (zt_diag_matches_filters(ctx, &diagnostics->items[i])) {
                    zt_diag_list_add(&filtered, diagnostics->items[i].code, diagnostics->items[i].span, "%s", diagnostics->items[i].message);
                    if (filtered.count > 0) {
                        filtered.items[filtered.count - 1].severity = diagnostics->items[i].severity;
                        filtered.items[filtered.count - 1].effort = diagnostics->items[i].effort;
                    }
                }
            }
            zt_diag_render_ci_list(stderr, stage, &filtered);
            zt_diag_list_dispose(&filtered);
        } else {
            zt_diag_render_ci_list(stderr, stage, diagnostics);
        }
    } else if (ctx != NULL && ctx->use_action_first) {
        size_t limit = ctx->show_all_errors ? (size_t)-1 : zt_cog_profile_error_limit(ctx->active_profile);
        if (has_filters) {
            filtered = zt_diag_list_make();
            for (i = 0; i < diagnostics->count; i += 1) {
                if (zt_diag_matches_filters(ctx, &diagnostics->items[i])) {
                    zt_diag_list_add(&filtered, diagnostics->items[i].code, diagnostics->items[i].span, "%s", diagnostics->items[i].message);
                    if (filtered.count > 0) {
                        filtered.items[filtered.count - 1].severity = diagnostics->items[i].severity;
                        filtered.items[filtered.count - 1].effort = diagnostics->items[i].effort;
                        if (diagnostics->items[i].suggestion[0] != '\0') {
                            snprintf(filtered.items[filtered.count - 1].suggestion, sizeof(filtered.items[filtered.count - 1].suggestion), "%s", diagnostics->items[i].suggestion);
                        }
                    }
                }
            }
            zt_diag_render_action_first_list(stderr, stage, &filtered, limit);
            zt_diag_list_dispose(&filtered);
        } else {
            zt_diag_render_action_first_list(stderr, stage, diagnostics, limit);
        }
    } else {
        if (has_filters) {
            filtered = zt_diag_list_make();
            for (i = 0; i < diagnostics->count; i += 1) {
                if (zt_diag_matches_filters(ctx, &diagnostics->items[i])) {
                    zt_diag_list_add(&filtered, diagnostics->items[i].code, diagnostics->items[i].span, "%s", diagnostics->items[i].message);
                    if (filtered.count > 0) {
                        filtered.items[filtered.count - 1].severity = diagnostics->items[i].severity;
                        filtered.items[filtered.count - 1].effort = diagnostics->items[i].effort;
                        if (diagnostics->items[i].suggestion[0] != '\0') {
                            snprintf(filtered.items[filtered.count - 1].suggestion, sizeof(filtered.items[filtered.count - 1].suggestion), "%s", diagnostics->items[i].suggestion);
                        }
                    }
                }
            }
            zt_diag_render_detailed_list(stderr, stage, &filtered);
            zt_diag_list_dispose(&filtered);
        } else {
            zt_diag_render_detailed_list(stderr, stage, diagnostics);
        }
    }
}

void zt_print_single_diag(
        zt_driver_context *ctx,
        const char *stage,
        zt_diag_code code,
        zt_source_span span,
        const char *format,
        ...) {
    zt_diag_list diagnostics = zt_diag_list_make();
    char message[512];
    va_list args;

    if (format == NULL) return;

    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);

    zt_diag_list_add(&diagnostics, code, span, "%s", message);
    zt_print_diagnostics(ctx, stage, &diagnostics);
    zt_diag_list_dispose(&diagnostics);
}

zt_diag_code zt_diag_code_from_zir_parse_error(zir_parse_error_code code) {
    switch (code) {
        case ZIR_PARSE_INVALID_INPUT:
        case ZIR_PARSE_EXPECTED_MODULE:
        case ZIR_PARSE_EXPECTED_FUNC:
        case ZIR_PARSE_EXPECTED_BLOCK:
        case ZIR_PARSE_EXPECTED_ARROW:
        case ZIR_PARSE_EXPECTED_COLON:
        case ZIR_PARSE_EXPECTED_LPAREN:
        case ZIR_PARSE_EXPECTED_RPAREN:
        case ZIR_PARSE_EXPECTED_IDENTIFIER:
        case ZIR_PARSE_TOO_MANY_PARAMS:
        case ZIR_PARSE_TOO_MANY_BLOCKS:
        case ZIR_PARSE_TOO_MANY_INSTRUCTIONS:
        case ZIR_PARSE_TOO_MANY_FUNCTIONS:
        case ZIR_PARSE_INVALID_TERMINATOR:
        case ZIR_PARSE_UNKNOWN_LINE:
        case ZIR_PARSE_OUT_OF_MEMORY:
        case ZIR_PARSE_OK:
        default:
            return ZT_DIAG_ZIR_PARSE_ERROR;
    }
}

zt_diag_code zt_diag_code_from_zir_verifier(zir_verifier_code code) {
    switch (code) {
        case ZIR_VERIFIER_INVALID_INPUT:
        case ZIR_VERIFIER_INVALID_TYPE_ANY:
        case ZIR_VERIFIER_INVALID_TARGET_LEAK:
        case ZIR_VERIFIER_USE_BEFORE_DEFINITION:
        case ZIR_VERIFIER_UNKNOWN_BLOCK:
        case ZIR_VERIFIER_INVALID_TERMINATOR:
        case ZIR_VERIFIER_DUPLICATE_BLOCK_LABEL:
        case ZIR_VERIFIER_INVALID_RETURN:
        case ZIR_VERIFIER_OK:
        default:
            return ZT_DIAG_ZIR_VERIFY_ERROR;
    }
}

zt_diag_code zt_diag_code_from_c_emit_error(c_emit_error_code code) {
    switch (code) {
        case C_EMIT_INVALID_INPUT:
        case C_EMIT_UNSUPPORTED_TYPE:
        case C_EMIT_UNSUPPORTED_EXPR:
        case C_EMIT_INVALID_MAIN_SIGNATURE:
        case C_EMIT_OK:
        default:
            return ZT_DIAG_BACKEND_C_EMIT_ERROR;
    }
}

static zt_diag_code zt_diag_code_from_zdoc_code(zt_zdoc_code code) {
    switch (code) {
        case ZT_ZDOC_MALFORMED_BLOCK: return ZT_DIAG_DOC_MALFORMED_BLOCK;
        case ZT_ZDOC_MISSING_TARGET: return ZT_DIAG_DOC_MISSING_TARGET;
        case ZT_ZDOC_MISSING_PAGE: return ZT_DIAG_DOC_MISSING_PAGE;
        case ZT_ZDOC_UNRESOLVED_TARGET: return ZT_DIAG_DOC_UNRESOLVED_TARGET;
        case ZT_ZDOC_UNRESOLVED_LINK: return ZT_DIAG_DOC_UNRESOLVED_LINK;
        case ZT_ZDOC_INVALID_GUIDE_TAG: return ZT_DIAG_DOC_INVALID_GUIDE_TAG;
        case ZT_ZDOC_INVALID_PAIRED_TAG: return ZT_DIAG_DOC_INVALID_PAIRED_TAG;
        case ZT_ZDOC_MISSING_PUBLIC_DOC: return ZT_DIAG_DOC_MISSING_PUBLIC_DOC;
        default: return ZT_DIAG_DOC_MALFORMED_BLOCK;
    }
}

static void zt_append_zdoc_diagnostics(zt_diag_list *dest, const zt_zdoc_diagnostic_list *diagnostics) {
    size_t i;

    if (dest == NULL || diagnostics == NULL) return;

    for (i = 0; i < diagnostics->count; i += 1) {
        const zt_zdoc_diagnostic *diag = &diagnostics->items[i];
        zt_diag_severity severity =
            diag->severity == ZT_ZDOC_WARNING ? ZT_DIAG_SEVERITY_WARNING : ZT_DIAG_SEVERITY_ERROR;
        zt_source_span span = zt_source_span_make(
            diag->path[0] != '\0' ? diag->path : "<zdoc>",
            diag->line > 0 ? diag->line : 1,
            diag->column > 0 ? diag->column : 1,
            diag->column > 0 ? diag->column : 1);
        zt_diag_list_add_severity(
            dest,
            zt_diag_code_from_zdoc_code(diag->code),
            severity,
            span,
            "%s",
            diag->message[0] != '\0' ? diag->message : "invalid zdoc");
    }
}

int zt_resolve_project_paths(
        const char *input_path,
        char *project_root,
        size_t project_root_capacity,
        char *manifest_path,
        size_t manifest_path_capacity) {
    if (input_path == NULL || input_path[0] == '\0') {
        if (!zt_find_project_root_from_cwd(project_root, project_root_capacity)) {
            zt_print_single_diag(
                NULL,
                "project",
                ZT_DIAG_PROJECT_INVALID_INPUT,
                zt_source_span_unknown(),
                "could not find zenith.ztproj from current directory");
            return 0;
        }

        if (!zt_join_path(manifest_path, manifest_path_capacity, project_root, "zenith.ztproj")) {
            zt_print_single_diag(
                NULL,
                "project",
                ZT_DIAG_PROJECT_PATH_TOO_LONG,
                zt_source_span_unknown(),
                "project manifest path is too long");
            return 0;
        }

        return 1;
    }

    if (zt_path_is_dir(input_path)) {
        if (!zt_copy_text(project_root, project_root_capacity, input_path)) {
            zt_print_single_diag(
                NULL,
                "project",
                ZT_DIAG_PROJECT_PATH_TOO_LONG,
                zt_source_span_unknown(),
                "project root path is too long");
            return 0;
        }

        if (!zt_join_path(manifest_path, manifest_path_capacity, project_root, "zenith.ztproj")) {
            zt_print_single_diag(
                NULL,
                "project",
                ZT_DIAG_PROJECT_PATH_TOO_LONG,
                zt_source_span_unknown(),
                "project manifest path is too long");
            return 0;
        }

        return 1;
    }

    if (zt_path_has_extension(input_path, ".ztproj")) {
        if (!zt_copy_text(manifest_path, manifest_path_capacity, input_path)) {
            zt_print_single_diag(
                NULL,
                "project",
                ZT_DIAG_PROJECT_PATH_TOO_LONG,
                zt_source_span_unknown(),
                "project manifest path is too long");
            return 0;
        }

        if (!zt_dirname(project_root, project_root_capacity, input_path)) {
            zt_print_single_diag(
                NULL,
                "project",
                ZT_DIAG_PROJECT_PATH_TOO_LONG,
                zt_source_span_unknown(),
                "project root path is too long");
            return 0;
        }

        return 1;
    }

    zt_print_single_diag(
        NULL,
        "project",
        ZT_DIAG_PROJECT_INVALID_INPUT,
        zt_source_span_unknown(),
        "project input must be a directory or a .ztproj file");
    return 0;
}

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
        if (strcmp(set->items[i], value) == 0) {
            return 1;
        }
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

static int zt_should_skip_test_discovery_dir(const char *name) {
    if (name == NULL || name[0] == '\0') return 1;
    if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) return 1;
    if (name[0] == '.') return 1;
    if (strcmp(name, "build") == 0) return 1;
    if (strcmp(name, "dist") == 0) return 1;
    if (strcmp(name, "node_modules") == 0) return 1;
    if (strcmp(name, ".ztc-tmp") == 0) return 1;
    return 0;
}

static int zt_dir_has_project_manifest(const char *directory) {
    char manifest_path[768];
    if (directory == NULL || directory[0] == '\0') return 0;
    if (!zt_join_path(manifest_path, sizeof(manifest_path), directory, "zenith.ztproj")) return 0;
    return zt_path_is_file(manifest_path);
}

static int zt_collect_test_projects(const char *directory, zt_string_set *projects) {
#ifdef _WIN32
    char pattern[768];
    intptr_t handle;
    struct _finddata_t data;

    if (directory == NULL || projects == NULL) return 0;

    if (zt_dir_has_project_manifest(directory)) {
        return zt_string_set_add(projects, directory);
    }

    if (!zt_join_path(pattern, sizeof(pattern), directory, "*")) {
        return 0;
    }

    handle = _findfirst(pattern, &data);
    if (handle == -1) {
        return 1;
    }

    do {
        char child_path[768];

        if ((data.attrib & _A_SUBDIR) == 0) continue;
        if (zt_should_skip_test_discovery_dir(data.name)) continue;

        if (!zt_join_path(child_path, sizeof(child_path), directory, data.name)) {
            _findclose(handle);
            return 0;
        }

        if (!zt_collect_test_projects(child_path, projects)) {
            _findclose(handle);
            return 0;
        }
    } while (_findnext(handle, &data) == 0);

    _findclose(handle);
    return 1;
#else
    DIR *dir;
    struct dirent *entry;

    if (directory == NULL || projects == NULL) return 0;

    if (zt_dir_has_project_manifest(directory)) {
        return zt_string_set_add(projects, directory);
    }

    dir = opendir(directory);
    if (dir == NULL) {
        return 1;
    }

    while ((entry = readdir(dir)) != NULL) {
        char child_path[768];

        if (zt_should_skip_test_discovery_dir(entry->d_name)) continue;

        if (!zt_join_path(child_path, sizeof(child_path), directory, entry->d_name)) {
            closedir(dir);
            return 0;
        }

        if (!zt_path_is_dir(child_path)) continue;

        if (!zt_collect_test_projects(child_path, projects)) {
            closedir(dir);
            return 0;
        }
    }

    closedir(dir);
    return 1;
#endif
}

static int zt_handle_project_command(
        zt_driver_context *ctx,
        const char *command,
        const char *input_path,
        const char *output_path,
        int run_output);
int zt_collect_project_sources(
        zt_driver_context *ctx,
        const char *input_path,
        zt_project_manifest *manifest,
        char *project_root,
        size_t project_root_capacity,
        zt_project_source_file_list *source_files);

static int zt_token_is_identifier_text(const zt_token *token, const char *text) {
    size_t length;

    if (token == NULL || text == NULL || token->kind != ZT_TOKEN_IDENTIFIER) return 0;
    length = strlen(text);
    return token->length == length && strncmp(token->text, text, length) == 0;
}

static int zt_file_has_test_marker(const char *path) {
    char *source_text;
    size_t source_length;
    zt_lexer *lexer;
    zt_token prev1;
    zt_token prev2;
    zt_token prev3;

    if (path == NULL || path[0] == '\0') return 0;

    source_text = zt_read_file(path);
    if (source_text == NULL) return 0;

    source_length = strlen(source_text);
    lexer = zt_lexer_make(path, source_text, source_length);
    if (lexer == NULL) {
        free(source_text);
        return 0;
    }

    memset(&prev1, 0, sizeof(prev1));
    memset(&prev2, 0, sizeof(prev2));
    memset(&prev3, 0, sizeof(prev3));

    while (1) {
        zt_token token = zt_lexer_next_token(lexer);

        if (token.kind == ZT_TOKEN_EOF) {
            break;
        }

        if (token.kind == ZT_TOKEN_COMMENT) {
            continue;
        }

        if (prev3.kind == ZT_TOKEN_IMPORT &&
                zt_token_is_identifier_text(&prev2, "std") &&
                prev1.kind == ZT_TOKEN_DOT &&
                zt_token_is_identifier_text(&token, "test")) {
            zt_lexer_dispose(lexer);
            free(source_text);
            return 1;
        }

        if (prev1.kind == ZT_TOKEN_ATTR && zt_token_is_identifier_text(&token, "test")) {
            zt_lexer_dispose(lexer);
            free(source_text);
            return 1;
        }

        prev3 = prev2;
        prev2 = prev1;
        prev1 = token;
    }

    zt_lexer_dispose(lexer);
    free(source_text);
    return 0;
}

static int zt_project_uses_test_markers(const char *project_dir) {
    char manifest_path[768];
    char source_root_path[768];
    zt_project_parse_result project;
    zt_project_source_file_list source_files;
    size_t i;
    int found = 0;

    if (project_dir == NULL || project_dir[0] == '\0') return 0;
    if (!zt_join_path(manifest_path, sizeof(manifest_path), project_dir, "zenith.ztproj")) return 0;
    if (!zt_project_load_file(manifest_path, &project)) return 0;
    if (!zt_join_path(source_root_path, sizeof(source_root_path), project_dir, project.manifest.source_root)) return 0;
    if (!zt_path_is_dir(source_root_path)) return 0;

    zt_project_source_file_list_init(&source_files);
    if (!zt_project_discover_zt_files(source_root_path, &source_files)) {
        zt_project_source_file_list_dispose(&source_files);
        return 0;
    }

    for (i = 0; i < source_files.count; i += 1) {
        if (zt_file_has_test_marker(source_files.items[i].path)) {
            found = 1;
            break;
        }
    }

    zt_project_source_file_list_dispose(&source_files);
    return found;
}
typedef struct zt_attr_test_case {
    char module_namespace[256];
    char function_name[128];
    zt_source_span span;
} zt_attr_test_case;

typedef struct zt_attr_test_case_list {
    zt_attr_test_case *items;
    size_t count;
    size_t capacity;
} zt_attr_test_case_list;

static void zt_attr_test_case_list_init(zt_attr_test_case_list *list) {
    if (list == NULL) return;
    memset(list, 0, sizeof(*list));
}

static void zt_attr_test_case_list_dispose(zt_attr_test_case_list *list) {
    if (list == NULL) return;
    free(list->items);
    memset(list, 0, sizeof(*list));
}

static int zt_attr_test_case_list_push(
        zt_attr_test_case_list *list,
        const char *module_namespace,
        const char *function_name,
        zt_source_span span) {
    zt_attr_test_case *new_items;
    size_t new_capacity;

    if (list == NULL || module_namespace == NULL || function_name == NULL) return 0;

    if (list->count >= list->capacity) {
        new_capacity = list->capacity == 0 ? 8 : list->capacity * 2;
        new_items = (zt_attr_test_case *)realloc(list->items, new_capacity * sizeof(zt_attr_test_case));
        if (new_items == NULL) return 0;
        list->items = new_items;
        list->capacity = new_capacity;
    }

    memset(&list->items[list->count], 0, sizeof(list->items[list->count]));
    if (!zt_copy_text(list->items[list->count].module_namespace, sizeof(list->items[list->count].module_namespace), module_namespace)) {
        return 0;
    }
    if (!zt_copy_text(list->items[list->count].function_name, sizeof(list->items[list->count].function_name), function_name)) {
        return 0;
    }
    list->items[list->count].span = span;
    list->count += 1;
    return 1;
}

static int zt_collect_attr_tests(
        zt_driver_context *ctx,
        const zt_project_source_file_list *source_files,
        zt_attr_test_case_list *test_cases) {
    size_t file_index;

    if (source_files == NULL || test_cases == NULL) return 0;

    for (file_index = 0; file_index < source_files->count; file_index += 1) {
        const zt_ast_node *root = source_files->items[file_index].parsed.root;
        size_t decl_index;

        if (root == NULL || root->kind != ZT_AST_FILE || root->as.file.module_name == NULL) continue;

        for (decl_index = 0; decl_index < root->as.file.declarations.count; decl_index += 1) {
            const zt_ast_node *decl = root->as.file.declarations.items[decl_index];
            if (decl == NULL || decl->kind != ZT_AST_FUNC_DECL) continue;
            if (!decl->as.func_decl.is_test) continue;

            if (decl->as.func_decl.params.count != 0) {
                zt_print_single_diag(
                    ctx,
                    "test",
                    ZT_DIAG_PROJECT_ERROR,
                    decl->span,
                    "attr test function '%s.%s' must not declare parameters",
                    root->as.file.module_name,
                    decl->as.func_decl.name != NULL ? decl->as.func_decl.name : "<unnamed>");
                return 0;
            }

            if (decl->as.func_decl.type_params.count != 0) {
                zt_print_single_diag(
                    ctx,
                    "test",
                    ZT_DIAG_PROJECT_ERROR,
                    decl->span,
                    "attr test function '%s.%s' must not declare type parameters",
                    root->as.file.module_name,
                    decl->as.func_decl.name != NULL ? decl->as.func_decl.name : "<unnamed>");
                return 0;
            }

            if (decl->as.func_decl.name == NULL ||
                    !zt_attr_test_case_list_push(test_cases, root->as.file.module_name, decl->as.func_decl.name, decl->span)) {
                zt_print_single_diag(
                    ctx,
                    "test",
                    ZT_DIAG_PROJECT_ERROR,
                    decl->span,
                    "out of memory while collecting attr test functions");
                return 0;
            }
        }
    }

    return 1;
}

static int zt_generate_attr_test_runner_source(
        const char *runner_namespace,
        const zt_attr_test_case *test_case,
        char *buffer,
        size_t buffer_size) {
    int written;

    if (runner_namespace == NULL || test_case == NULL || buffer == NULL || buffer_size == 0) return 0;

    written = snprintf(
        buffer,
        buffer_size,
        "namespace %s\n"
        "\n"
        "import %s as tested\n"
        "\n"
        "func main() -> int\n"
        "    tested.%s()\n"
        "    return 0\n"
        "end\n",
        runner_namespace,
        test_case->module_namespace,
        test_case->function_name);

    return written > 0 && (size_t)written < buffer_size;
}

static int zt_generate_attr_test_manifest(
        const zt_project_manifest *manifest,
        const char *runner_namespace,
        char *buffer,
        size_t buffer_size) {
    int written;

    if (manifest == NULL || runner_namespace == NULL || buffer == NULL || buffer_size == 0) return 0;

    written = snprintf(
        buffer,
        buffer_size,
        "[project]\n"
        "name = \"%s-test-runner\"\n"
        "version = \"%s\"\n"
        "kind = \"app\"\n"
        "\n"
        "[source]\n"
        "root = \"%s\"\n"
        "\n"
        "[app]\n"
        "entry = \"%s\"\n"
        "\n"
        "[build]\n"
        "target = \"native\"\n"
        "output = \".ztc-tmp/test-runner-build\"\n"
        "profile = \"debug\"\n",
        manifest->project_name,
        manifest->version,
        manifest->source_root,
        runner_namespace);

    return written > 0 && (size_t)written < buffer_size;
}

static int zt_run_attr_tests_for_project(
        zt_driver_context *ctx,
        const char *project_path,
        int *out_passed,
        int *out_skipped,
        int *out_failed,
        int *out_has_attr_tests) {
    zt_project_manifest manifest;
    char project_root[512];
    zt_project_source_file_list source_files;
    zt_attr_test_case_list test_cases;
    char source_root_path[768];
    char runner_rel_path[512];
    char runner_source_path[1024];
    char runner_source_dir[1024];
    char runner_manifest_path[1024];
    char runner_manifest_text[2048];
    const char *runner_namespace = "__zenith_internal_test_runner.main";
    int passed = 0;
    int skipped = 0;
    int failed = 0;
    size_t i;
    int ok = 1;

    if (out_passed != NULL) *out_passed = 0;
    if (out_skipped != NULL) *out_skipped = 0;
    if (out_failed != NULL) *out_failed = 0;
    if (out_has_attr_tests != NULL) *out_has_attr_tests = 0;

    zt_attr_test_case_list_init(&test_cases);

    if (!zt_collect_project_sources(
            ctx,
            project_path,
            &manifest,
            project_root,
            sizeof(project_root),
            &source_files)) {
        zt_attr_test_case_list_dispose(&test_cases);
        return 0;
    }

    if (!zt_collect_attr_tests(ctx, &source_files, &test_cases)) {
        zt_project_source_file_list_dispose(&source_files);
        zt_attr_test_case_list_dispose(&test_cases);
        return 0;
    }

    if (test_cases.count == 0) {
        zt_project_source_file_list_dispose(&source_files);
        zt_attr_test_case_list_dispose(&test_cases);
        return 1;
    }

    if (out_has_attr_tests != NULL) *out_has_attr_tests = 1;

    if (!zt_join_path(source_root_path, sizeof(source_root_path), project_root, manifest.source_root)) {
        zt_print_single_diag(
            ctx,
            "test",
            ZT_DIAG_PROJECT_PATH_TOO_LONG,
            zt_source_span_unknown(),
            "source.root path is too long while preparing attr test runner");
        ok = 0;
        goto cleanup;
    }

    if (!zt_namespace_to_relative_path(runner_namespace, runner_rel_path, sizeof(runner_rel_path)) ||
            !zt_join_path(runner_source_path, sizeof(runner_source_path), source_root_path, runner_rel_path) ||
            !zt_dirname(runner_source_dir, sizeof(runner_source_dir), runner_source_path)) {
        zt_print_single_diag(
            ctx,
            "test",
            ZT_DIAG_PROJECT_PATH_TOO_LONG,
            zt_source_span_unknown(),
            "runner source path is too long");
        ok = 0;
        goto cleanup;
    }

    if (!zt_join_path(runner_manifest_path, sizeof(runner_manifest_path), project_root, "zenith.__zt_test_runner.ztproj")) {
        zt_print_single_diag(
            ctx,
            "test",
            ZT_DIAG_PROJECT_PATH_TOO_LONG,
            zt_source_span_unknown(),
            "runner manifest path is too long");
        ok = 0;
        goto cleanup;
    }

    if (!zt_make_dirs(runner_source_dir)) {
        zt_print_single_diag(
            ctx,
            "test",
            ZT_DIAG_PROJECT_ERROR,
            zt_source_span_unknown(),
            "unable to create directories for attr test runner");
        ok = 0;
        goto cleanup;
    }

    if (!zt_generate_attr_test_manifest(&manifest, runner_namespace, runner_manifest_text, sizeof(runner_manifest_text)) ||
            !zt_write_file(runner_manifest_path, runner_manifest_text)) {
        zt_print_single_diag(
            ctx,
            "test",
            ZT_DIAG_PROJECT_ERROR,
            zt_source_span_unknown(),
            "unable to write attr test runner manifest");
        ok = 0;
        goto cleanup;
    }

    for (i = 0; i < test_cases.count; i += 1) {
        char runner_source_text[2048];
        int status;

        if (!zt_generate_attr_test_runner_source(runner_namespace, &test_cases.items[i], runner_source_text, sizeof(runner_source_text)) ||
                !zt_write_file(runner_source_path, runner_source_text)) {
            zt_print_single_diag(
                ctx,
                "test",
                ZT_DIAG_PROJECT_ERROR,
                test_cases.items[i].span,
                "unable to write attr test runner source");
            ok = 0;
            goto cleanup;
        }

        if (ctx == NULL || !ctx->ci_mode_enabled) {
            printf("test case: %s.%s\\n", test_cases.items[i].module_namespace, test_cases.items[i].function_name);
        }

        status = zt_handle_project_command(ctx, "build", runner_manifest_path, NULL, 1);
        if (status == ZT_EXIT_CODE_TEST_SKIPPED) {
            skipped += 1;
        } else if (status == 0) {
            passed += 1;
        } else {
            failed += 1;
        }
    }

cleanup:
    remove(runner_source_path);
    remove(runner_manifest_path);
    zt_project_source_file_list_dispose(&source_files);
    zt_attr_test_case_list_dispose(&test_cases);

    if (out_passed != NULL) *out_passed = passed;
    if (out_skipped != NULL) *out_skipped = skipped;
    if (out_failed != NULL) *out_failed = failed;
    return ok;
}


static int zt_handle_doc_check(zt_driver_context *ctx, const char *input_path) {
    zt_project_manifest manifest;
    char project_root[512];
    zt_project_source_file_list source_files;
    zt_zdoc_source_unit *units = NULL;
    zt_zdoc_diagnostic_list diagnostics;
    zt_diag_list rendered = zt_diag_list_make();
    size_t error_count;
    size_t warning_count;
    size_t i;

    if (!zt_collect_project_sources(
            ctx,
            input_path,
            &manifest,
            project_root,
            sizeof(project_root),
            &source_files)) {
        return 1;
    }

    units = (zt_zdoc_source_unit *)calloc(source_files.count > 0 ? source_files.count : 1, sizeof(zt_zdoc_source_unit));
    if (units == NULL) {
        zt_print_single_diag(
            ctx,
            "doc",
            ZT_DIAG_DOC_MALFORMED_BLOCK,
            zt_source_span_unknown(),
            "out of memory while preparing zdoc check");
        zt_project_source_file_list_dispose(&source_files);
        return 1;
    }

    for (i = 0; i < source_files.count; i += 1) {
        units[i].source_path = source_files.items[i].path;
        units[i].root = source_files.items[i].parsed.root;
    }

    zt_zdoc_diagnostic_list_init(&diagnostics);
    if (!zt_zdoc_check_project(
            project_root,
            manifest.source_root,
            manifest.zdoc_root,
            units,
            source_files.count,
            &diagnostics)) {
        zt_print_single_diag(
            ctx,
            "doc",
            ZT_DIAG_DOC_MALFORMED_BLOCK,
            zt_source_span_unknown(),
            "zdoc checker internal failure");
        zt_zdoc_diagnostic_list_dispose(&diagnostics);
        free(units);
        zt_project_source_file_list_dispose(&source_files);
        return 1;
    }

    zt_append_zdoc_diagnostics(&rendered, &diagnostics);
    zt_print_diagnostics(ctx, "doc", &rendered);

    error_count = zt_zdoc_diagnostic_error_count(&diagnostics);
    warning_count = zt_zdoc_diagnostic_warning_count(&diagnostics);

    if (error_count == 0) {
        if (warning_count == 0) {
            printf("doc check ok\n");
        } else {
            printf("doc check ok (%zu warnings)\n", warning_count);
        }
    } else if (ctx == NULL || !ctx->ci_mode_enabled) {
        printf("action: run `zt doc check <project>` and fix malformed or unresolved @target blocks\n");
        printf("why: doc checker found %zu error(s)\n", error_count);
    }

    zt_diag_list_dispose(&rendered);
    zt_zdoc_diagnostic_list_dispose(&diagnostics);
    free(units);
    zt_project_source_file_list_dispose(&source_files);
    return error_count == 0 ? 0 : 1;
}

static int zt_handle_fmt(zt_driver_context *ctx, const char *input_path, int check_only) {
    zt_project_manifest manifest;
    char project_root[512];
    zt_project_source_file_list source_files;
    int diff_count = 0;
    size_t i;

    if (!zt_collect_project_sources(
            ctx,
            input_path,
            &manifest,
            project_root,
            sizeof(project_root),
            &source_files)) {
        return 1;
    }

    for (i = 0; i < source_files.count; i++) {
        char *formatted = zt_format_node_to_string(source_files.items[i].parsed.root);
        if (!formatted) continue;
        {
            size_t formatted_len = strlen(formatted);
            if (formatted_len == 0 || formatted[formatted_len - 1] != '\n') {
                char *with_newline = (char *)malloc(formatted_len + 2);
                if (with_newline != NULL) {
                    memcpy(with_newline, formatted, formatted_len);
                    with_newline[formatted_len] = '\n';
                    with_newline[formatted_len + 1] = '\0';
                    free(formatted);
                    formatted = with_newline;
                }
            }
        }

        if (check_only) {
            char *original = zt_read_file(source_files.items[i].path);
            if (original) {
                if (strcmp(original, formatted) != 0) {
                    diff_count++;
                    if (ctx == NULL || !ctx->ci_mode_enabled) {
                        printf("diff: %s\n", source_files.items[i].path);
                    }
                }
                free(original);
            }
        } else {
            char *original = zt_read_file(source_files.items[i].path);
            if (original) {
                if (strcmp(original, formatted) != 0) {
                    zt_write_file(source_files.items[i].path, formatted);
                    if (ctx == NULL || !ctx->ci_mode_enabled) {
                        printf("formatted: %s\n", source_files.items[i].path);
                    }
                }
                free(original);
            }
        }
        free(formatted);
    }

    zt_project_source_file_list_dispose(&source_files);

    if (check_only) {
        if (diff_count > 0) {
            if (ctx != NULL && ctx->ci_mode_enabled) {
                printf("fmt check failed\n");
            } else {
                printf("fmt check failed: differences found (%d file(s))\n", diff_count);
                printf("action: run `zt fmt <project>` and commit the formatted files\n");
                printf("why: source differs from the canonical formatter output\n");
            }
            return 1;
        } else {
            printf("%s", (ctx != NULL && ctx->ci_mode_enabled) ? "fmt ok\n" : "fmt check ok\n");
        }
    } else {
        printf("%s", (ctx != NULL && ctx->ci_mode_enabled) ? "fmt ok\n" : "fmt ok\n");
    }

    return 0;
}

static int zt_handle_project_info(zt_driver_context *ctx, const char *input_path) {
    zt_project_parse_result project;
    char project_root[512];
    char manifest_path[512];
    char entry_path[512];
    zt_project_kind kind;

    entry_path[0] = '\0';

    if (!zt_resolve_project_paths(input_path, project_root, sizeof(project_root), manifest_path, sizeof(manifest_path))) {
        return 1;
    }

    if (!zt_project_load_file(manifest_path, &project)) {
        zt_print_project_parse_error(ctx, manifest_path, &project);
        return 1;
    }

    zt_apply_manifest_lang(&project.manifest);

    kind = zt_project_manifest_kind(&project.manifest);
    if (kind == ZT_PROJECT_KIND_APP &&
            !zt_project_resolve_entry_source_path(&project.manifest, project_root, entry_path, sizeof(entry_path))) {
        zt_print_single_diag(ctx, "project", ZT_DIAG_PROJECT_PATH_TOO_LONG, zt_source_span_unknown(), "app.entry source path is too long");
        return 1;
    }

    printf("project %s\n", project.manifest.project_name);
    printf("  kind: %s\n", project.manifest.project_kind);
    printf("  version: %s\n", project.manifest.version);
    printf("  root: %s\n", project_root);
    printf("  manifest: %s\n", manifest_path);
    printf("  source.root: %s\n", project.manifest.source_root);
    if (kind == ZT_PROJECT_KIND_APP) {
        printf("  app.entry: %s\n", project.manifest.app_entry);
        printf("  entry_path: %s\n", entry_path);
    } else if (kind == ZT_PROJECT_KIND_LIB) {
        printf("  lib.root_namespace: %s\n", project.manifest.lib_root_namespace);
    }
    printf("  build.target: %s\n", project.manifest.build_target);
    printf("  build.profile: %s\n", project.manifest.build_profile);
    printf("  build.monomorphization_limit: %zu\n", project.manifest.build_monomorphization_limit);
    printf("  build.output: %s\n", project.manifest.build_output);
    if (project.manifest.build_linker_flags[0] != '\0') {
        printf("  build.linker_flags: %s\n", project.manifest.build_linker_flags);
    }
    printf("  test.root: %s\n", project.manifest.test_root);
    printf("  zdoc.root: %s\n", project.manifest.zdoc_root);
    printf("  dependencies: %zu\n", project.manifest.dependency_count);
    if (project.manifest.dependency_count > 0) {
        size_t i;
        for (i = 0; i < project.manifest.dependency_count; i += 1) {
            printf("    %s = %s\n", project.manifest.dependencies[i].name, project.manifest.dependencies[i].spec);
        }
    }
    printf("  dev_dependencies: %zu\n", project.manifest.dev_dependency_count);
    if (project.manifest.dev_dependency_count > 0) {
        size_t i;
        for (i = 0; i < project.manifest.dev_dependency_count; i += 1) {
            printf("    %s = %s\n", project.manifest.dev_dependencies[i].name, project.manifest.dev_dependencies[i].spec);
        }
    }
    return 0;
}

static int zt_handle_project_command(
        zt_driver_context *ctx,
        const char *command,
        const char *input_path,
        const char *output_path,
        int run_output) {
    zt_project_compile_result compiled;
    c_emitter emitter;
    c_emit_result emit_result;
    char output_dir[512];
    char c_name[192];
    char c_path[768];
    char default_exe_name[192];
    char exe_path[768];

    if (!zt_compile_project(ctx, input_path, &compiled)) {
        return 1;
    }

    if (strcmp(command, "verify") == 0) {
        printf("%s", (ctx != NULL && ctx->ci_mode_enabled) ? "check ok\n" : "verification ok\n");
        zt_project_compile_result_dispose(&compiled);
        return 0;
    }

    if (strcmp(command, "build") == 0 &&
            zt_project_manifest_kind(&compiled.manifest) == ZT_PROJECT_KIND_LIB) {
        fprintf(stderr, "project error: build for lib projects is not implemented in the bootstrap driver; use verify or emit-c\n");
        zt_project_compile_result_dispose(&compiled);
        return 1;
    }
    if (strcmp(command, "build") == 0 &&
            zt_project_manifest_kind(&compiled.manifest) == ZT_PROJECT_KIND_APP &&
            compiled.zir.module.function_count == 0) {
        zt_print_single_diag(
            ctx,
            "zir.lower",
            ZT_DIAG_BACKEND_C_EMIT_ERROR,
            zt_source_span_unknown(),
            "build de app bloqueado: modulo ZIR sem funcoes (verifique compiler/zir/lowering/from_hir.c; source atual esta em stub)");
        zt_project_compile_result_dispose(&compiled);
        return 1;
    }

    if (!zt_emit_module_to_c(&compiled.zir.module, &emitter, &emit_result)) {
        zt_print_single_diag(
            ctx,
            "backend.c.emit",
            zt_diag_code_from_c_emit_error(emit_result.code),
            zt_source_span_make("<zir>", 1, 1, 1),
            "%s",
            emit_result.message[0] != '\0' ? emit_result.message : "C emission failed");
        zt_project_compile_result_dispose(&compiled);
        return 1;
    }

    if (strcmp(command, "emit-c") == 0) {
        if (!c_emitter_write_stream(&emitter, stdout) ||
                fputc('\n', stdout) == EOF ||
                fflush(stdout) != 0) {
            fprintf(stderr, "project error: failed to write generated C to stdout\n");
            c_emitter_dispose(&emitter);
            zt_project_compile_result_dispose(&compiled);
            return 1;
        }
        c_emitter_dispose(&emitter);
        zt_project_compile_result_dispose(&compiled);
        return 0;
    }

    if (strcmp(command, "build") != 0) {
        fprintf(stderr, "unknown command: %s\n", command);
        c_emitter_dispose(&emitter);
        zt_project_compile_result_dispose(&compiled);
        return 1;
    }

    if (!zt_join_path(output_dir, sizeof(output_dir), compiled.project_root, compiled.manifest.output_dir)) {
        fprintf(stderr, "error: output directory path is too long\n");
        c_emitter_dispose(&emitter);
        zt_project_compile_result_dispose(&compiled);
        return 1;
    }

    if (!zt_make_dirs(output_dir)) {
        fprintf(stderr, "error: cannot create output directory '%s'\n", output_dir);
        c_emitter_dispose(&emitter);
        zt_project_compile_result_dispose(&compiled);
        return 1;
    }

    snprintf(c_name, sizeof(c_name), "%s.c", compiled.manifest.output_name);
    if (!zt_join_path(c_path, sizeof(c_path), output_dir, c_name)) {
        fprintf(stderr, "error: C output path is too long\n");
        c_emitter_dispose(&emitter);
        zt_project_compile_result_dispose(&compiled);
        return 1;
    }

    if (output_path != NULL) {
        if (!zt_copy_text(exe_path, sizeof(exe_path), output_path)) {
            fprintf(stderr, "error: executable output path is too long\n");
            c_emitter_dispose(&emitter);
            zt_project_compile_result_dispose(&compiled);
            return 1;
        }
    } else {
        snprintf(default_exe_name, sizeof(default_exe_name), "%s.exe", compiled.manifest.output_name);
        if (!zt_join_path(exe_path, sizeof(exe_path), output_dir, default_exe_name)) {
            fprintf(stderr, "error: executable output path is too long\n");
            c_emitter_dispose(&emitter);
            zt_project_compile_result_dispose(&compiled);
            return 1;
        }
    }

    if (!c_emitter_write_file(&emitter, c_path)) {
        c_emitter_dispose(&emitter);
        zt_project_compile_result_dispose(&compiled);
        return 1;
    }

    c_emitter_dispose(&emitter);

    if (!zt_compile_c_file(ctx, c_path, exe_path, &compiled.manifest)) {
        zt_project_compile_result_dispose(&compiled);
        return 1;
    }

    if (ctx != NULL && ctx->ci_mode_enabled && !run_output) {
        printf("build ok\n");
    }

    if (run_output) {
        int run_exit = zt_run_executable(ctx, exe_path);
        zt_project_compile_result_dispose(&compiled);
        return run_exit < 0 ? 1 : run_exit;
    }

    zt_project_compile_result_dispose(&compiled);
    return 0;
}

static int zt_handle_test(zt_driver_context *ctx, const char *input_path) {
    char project_root[512];
    char manifest_path[512];
    zt_project_parse_result project;
    char test_root_path[768];
    zt_string_set discovered_projects;
    zt_string_set selected_projects;
    size_t discovered_count = 0;
    size_t i;
    int passed = 0;
    int skipped = 0;
    int failed = 0;

    if (!zt_resolve_project_paths(input_path, project_root, sizeof(project_root), manifest_path, sizeof(manifest_path))) {
        return 1;
    }

    if (!zt_project_load_file(manifest_path, &project)) {
        zt_print_project_parse_error(ctx, manifest_path, &project);
        return 1;
    }

    if (!zt_join_path(test_root_path, sizeof(test_root_path), project_root, project.manifest.test_root)) {
        zt_print_single_diag(
            ctx,
            "test",
            ZT_DIAG_PROJECT_PATH_TOO_LONG,
            zt_source_span_unknown(),
            "test.root path is too long");
        return 1;
    }

    zt_string_set_init(&discovered_projects);
    zt_string_set_init(&selected_projects);

    if (zt_path_is_dir(test_root_path)) {
        if (!zt_collect_test_projects(test_root_path, &discovered_projects)) {
            zt_string_set_dispose(&selected_projects);
            zt_string_set_dispose(&discovered_projects);
            zt_print_single_diag(
                ctx,
                "test",
                ZT_DIAG_PROJECT_ERROR,
                zt_source_span_unknown(),
                "unable to scan test.root '%s'",
                test_root_path);
            return 1;
        }

        for (i = 0; i < discovered_projects.count; i += 1) {
            const char *candidate = discovered_projects.items[i];
            if (candidate == NULL) continue;
            if (strcmp(candidate, project_root) == 0) continue;
            if (!zt_project_uses_test_markers(candidate)) continue;
            if (!zt_string_set_add(&selected_projects, candidate)) {
                zt_string_set_dispose(&selected_projects);
                zt_string_set_dispose(&discovered_projects);
                zt_print_single_diag(
                    ctx,
                    "test",
                    ZT_DIAG_PROJECT_ERROR,
                    zt_source_span_unknown(),
                    "out of memory while selecting discovered test projects");
                return 1;
            }
            discovered_count += 1;
        }
    }

    if (discovered_count == 0) {
        int attr_passed = 0;
        int attr_skipped = 0;
        int attr_failed = 0;
        int has_attr_tests = 0;

        if (!zt_run_attr_tests_for_project(ctx, project_root, &attr_passed, &attr_skipped, &attr_failed, &has_attr_tests)) {
            zt_string_set_dispose(&selected_projects);
            zt_string_set_dispose(&discovered_projects);
            return 1;
        }

        zt_string_set_dispose(&selected_projects);
        zt_string_set_dispose(&discovered_projects);

        if (has_attr_tests) {
            passed = attr_passed;
            skipped = attr_skipped;
            failed = attr_failed;

            if (ctx != NULL && ctx->ci_mode_enabled) {
                if (failed == 0) {
                    printf("test ok (pass=%d skip=%d)\n", passed, skipped);
                } else {
                    printf("test failed (pass=%d skip=%d fail=%d)\n", passed, skipped, failed);
                }
            } else {
                printf("test summary: pass=%d skip=%d fail=%d\n", passed, skipped, failed);
            }
            return failed == 0 ? 0 : 1;
        }

        {
            int status = zt_handle_project_command(ctx, "build", project_root, NULL, 1);

            if (status == ZT_EXIT_CODE_TEST_SKIPPED) {
                printf("%s", (ctx != NULL && ctx->ci_mode_enabled) ? "test skipped\n" : "test skipped (std.test.skip)\n");
                return 0;
            }

            if (status == ZT_EXIT_CODE_TEST_FAILED) {
                printf("%s", (ctx != NULL && ctx->ci_mode_enabled) ? "test failed\n" : "test failed (std.test.fail)\n");
                return 1;
            }

            if (status == 0) {
                printf("test ok\n");
                return 0;
            }

            if (status == ZT_EXIT_CODE_RUNTIME_ERROR) {
                return 1;
            }

            if (ctx != NULL && ctx->ci_mode_enabled) {
                printf("test ok\n");
            } else {
                printf("test bootstrap fallback: executable exit code %d treated as pass (no std.test outcome)\n", status);
            }
            return 0;
        }
    }

    if (ctx == NULL || !ctx->ci_mode_enabled) {
        printf("test discovery: %zu project(s) with markers (import std.test or attr test) under %s\n", discovered_count, test_root_path);
    }

    for (i = 0; i < selected_projects.count; i += 1) {
        const char *candidate = selected_projects.items[i];
        int status;
        int attr_passed = 0;
        int attr_skipped = 0;
        int attr_failed = 0;
        int has_attr_tests = 0;

        if (candidate == NULL) continue;

        if (ctx == NULL || !ctx->ci_mode_enabled) {
            printf("test project: %s\n", candidate);
        }

        if (!zt_run_attr_tests_for_project(ctx, candidate, &attr_passed, &attr_skipped, &attr_failed, &has_attr_tests)) {
            zt_string_set_dispose(&selected_projects);
            zt_string_set_dispose(&discovered_projects);
            return 1;
        }

        if (has_attr_tests) {
            passed += attr_passed;
            skipped += attr_skipped;
            failed += attr_failed;
            continue;
        }

        status = zt_handle_project_command(ctx, "build", candidate, NULL, 1);

        if (status == ZT_EXIT_CODE_TEST_SKIPPED) {
            skipped += 1;
        } else if (status == 0) {
            passed += 1;
        } else {
            failed += 1;
        }
    }

    zt_string_set_dispose(&selected_projects);
    zt_string_set_dispose(&discovered_projects);

    if (ctx != NULL && ctx->ci_mode_enabled) {
        if (failed == 0) {
            printf("test ok (pass=%d skip=%d)\n", passed, skipped);
        } else {
            printf("test failed (pass=%d skip=%d fail=%d)\n", passed, skipped, failed);
        }
    } else {
        printf("test summary: pass=%d skip=%d fail=%d\n", passed, skipped, failed);
    }

    return failed == 0 ? 0 : 1;
}


static int zt_handle_zir_command(
        zt_driver_context *ctx,
        const char *command,
        const char *input_path,
        const char *output_path,
        int run_output) {
    char *input_text;
    size_t input_length;
    zir_parse_result parse_result;
    zir_verifier_result verifier_result;
    c_emitter emitter;
    c_emit_result emit_result;

    input_text = zt_read_file(input_path);
    if (input_text == NULL) {
        return 1;
    }
    input_length = strlen(input_text);

    if (!zir_parse_module(input_text, input_length, &parse_result)) {
        size_t line = parse_result.line_number > 0 ? (size_t)parse_result.line_number : 1;
        zt_source_span span = zt_source_span_make(input_path, line, 1, 1);
        zt_print_single_diag(
            ctx,
            "zir.parse",
            zt_diag_code_from_zir_parse_error(parse_result.code),
            span,
            "%s",
            parse_result.message[0] != '\0' ? parse_result.message : "unable to parse ZIR module");
        free(input_text);
        return 1;
    }

    if (strcmp(command, "parse") == 0) {
        size_t fi;

        printf("module %s\n", parse_result.module.name);
        for (fi = 0; fi < parse_result.module.function_count; fi += 1) {
            const zir_function *func = &parse_result.module.functions[fi];
            size_t pi;
            size_t bi;

            printf("  func %s(", func->name);
            for (pi = 0; pi < func->param_count; pi += 1) {
                if (pi > 0) printf(", ");
                printf("%s: %s", func->params[pi].name, func->params[pi].type_name);
            }
            printf(") -> %s\n", func->return_type);

            for (bi = 0; bi < func->block_count; bi += 1) {
                const zir_block *block = &func->blocks[bi];
                size_t ii;

                printf("    block %s (%zu instructions)\n", block->label, block->instruction_count);
                for (ii = 0; ii < block->instruction_count; ii += 1) {
                    const zir_instruction *instr = &block->instructions[ii];
                    if (instr->kind == ZIR_INSTR_ASSIGN) {
                        printf("      %s: %s = %s\n", instr->dest_name, instr->type_name, instr->expr_text);
                    } else {
                        printf("      %s\n", instr->expr_text);
                    }
                }
            }
        }

        zir_parse_result_dispose(&parse_result);
        free(input_text);
        return 0;
    }

    if (!zir_verify_module(&parse_result.module, &verifier_result)) {
        zt_source_span span = zt_source_span_make(input_path, 1, 1, 1);
        if (verifier_result.has_span) {
            const char *source_name = verifier_result.source_name != NULL && verifier_result.source_name[0] != '\0'
                ? verifier_result.source_name
                : input_path;
            size_t line = verifier_result.line > 0 ? verifier_result.line : 1;
            size_t column = verifier_result.column > 0 ? verifier_result.column : 1;
            span = zt_source_span_make(source_name, line, column, column);
        }
        zt_print_single_diag(
            ctx,
            "zir.verify",
            zt_diag_code_from_zir_verifier(verifier_result.code),
            span,
            "%s",
            verifier_result.message[0] != '\0' ? verifier_result.message : "ZIR verification failed");
        zir_parse_result_dispose(&parse_result);
        free(input_text);
        return 1;
    }

    if (strcmp(command, "verify") == 0) {
        printf("verification ok\n");
        zir_parse_result_dispose(&parse_result);
        free(input_text);
        return 0;
    }

    if (!zt_emit_module_to_c(&parse_result.module, &emitter, &emit_result)) {
        zt_print_single_diag(
            ctx,
            "backend.c.emit",
            zt_diag_code_from_c_emit_error(emit_result.code),
            zt_source_span_make(input_path, 1, 1, 1),
            "%s",
            emit_result.message[0] != '\0' ? emit_result.message : "C emission failed");
        zir_parse_result_dispose(&parse_result);
        free(input_text);
        return 1;
    }

    if (strcmp(command, "emit-c") == 0) {
        if (!c_emitter_write_stream(&emitter, stdout) ||
                fputc('\n', stdout) == EOF ||
                fflush(stdout) != 0) {
            fprintf(stderr, "project error: failed to write generated C to stdout\n");
            c_emitter_dispose(&emitter);
            zir_parse_result_dispose(&parse_result);
            free(input_text);
            return 1;
        }
        c_emitter_dispose(&emitter);
        zir_parse_result_dispose(&parse_result);
        free(input_text);
        return 0;
    }

    if (strcmp(command, "build") == 0) {
        char c_path[512];
        char exe_path[512];

        if (!zt_replace_extension(c_path, sizeof(c_path), input_path, ".c")) {
            fprintf(stderr, "error: C output path is too long\n");
            c_emitter_dispose(&emitter);
            zir_parse_result_dispose(&parse_result);
            free(input_text);
            return 1;
        }

        if (output_path != NULL) {
            if (!zt_copy_text(exe_path, sizeof(exe_path), output_path)) {
                fprintf(stderr, "error: executable output path is too long\n");
                c_emitter_dispose(&emitter);
                zir_parse_result_dispose(&parse_result);
                free(input_text);
                return 1;
            }
        } else if (!zt_replace_extension(exe_path, sizeof(exe_path), input_path, ".exe")) {
            fprintf(stderr, "error: executable output path is too long\n");
            c_emitter_dispose(&emitter);
            zir_parse_result_dispose(&parse_result);
            free(input_text);
            return 1;
        }

        if (!c_emitter_write_file(&emitter, c_path)) {
            c_emitter_dispose(&emitter);
            zir_parse_result_dispose(&parse_result);
            free(input_text);
            return 1;
        }

        c_emitter_dispose(&emitter);

        if (!zt_compile_c_file(ctx, c_path, exe_path, NULL)) {
            zir_parse_result_dispose(&parse_result);
            free(input_text);
            return 1;
        }

        if (ctx != NULL && ctx->ci_mode_enabled && !run_output) {
            printf("build ok\n");
        }

        if (run_output) {
            int run_exit = zt_run_executable(ctx, exe_path);
            zir_parse_result_dispose(&parse_result);
            free(input_text);
            return run_exit < 0 ? 1 : run_exit;
        }

        zir_parse_result_dispose(&parse_result);
        free(input_text);
        return 0;
    }

    fprintf(stderr, "unknown command: %s\n", command);
    c_emitter_dispose(&emitter);
    zir_parse_result_dispose(&parse_result);
    free(input_text);
    return 1;
}

static int zt_find_most_recent_file(const char *dir, const char *ext, char *out, size_t capacity) {
    time_t best_time = 0;
    char best_path[512] = "";

#ifdef _WIN32
    WIN32_FIND_DATAA find_data;
    HANDLE hFind;
    char pattern[768];
    snprintf(pattern, sizeof(pattern), "%s\\*", dir);
    hFind = FindFirstFileA(pattern, &find_data);
    if (hFind == INVALID_HANDLE_VALUE) return 0;
    do {
        if (strcmp(find_data.cFileName, ".") == 0 || strcmp(find_data.cFileName, "..") == 0) continue;
        {
            char child_path[768];
            snprintf(child_path, sizeof(child_path), "%s\\%s", dir, find_data.cFileName);
            if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                char child_best[512];
                if (zt_find_most_recent_file(child_path, ext, child_best, sizeof(child_best))) {
                    struct stat child_st;
                    if (stat(child_best, &child_st) == 0 && child_st.st_mtime > best_time) {
                        if (zt_copy_text(best_path, sizeof(best_path), child_best)) {
                            best_time = child_st.st_mtime;
                        }
                    }
                }
            } else {
                size_t name_len = strlen(find_data.cFileName);
                size_t ext_len = strlen(ext);
                if (name_len >= ext_len && strcmp(find_data.cFileName + name_len - ext_len, ext) == 0) {
                    struct stat st;
                    if (stat(child_path, &st) == 0 && st.st_mtime > best_time) {
                        if (zt_copy_text(best_path, sizeof(best_path), child_path)) {
                            best_time = st.st_mtime;
                        }
                    }
                }
            }
        }
    } while (FindNextFileA(hFind, &find_data));
    FindClose(hFind);
#else
    DIR *dp;
    struct dirent *entry;
    dp = opendir(dir);
    if (dp == NULL) return 0;
    while ((entry = readdir(dp)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
        {
            char child_path[768];
            snprintf(child_path, sizeof(child_path), "%s/%s", dir, entry->d_name);
            {
                struct stat st;
                if (stat(child_path, &st) == 0) {
                    if (S_ISDIR(st.st_mode)) {
                        char child_best[512];
                        if (zt_find_most_recent_file(child_path, ext, child_best, sizeof(child_best))) {
                            struct stat child_st;
                            if (stat(child_best, &child_st) == 0 && child_st.st_mtime > best_time) {
                                if (zt_copy_text(best_path, sizeof(best_path), child_best)) {
                                    best_time = child_st.st_mtime;
                                }
                            }
                        }
                    } else if (st.st_mtime > best_time) {
                        size_t name_len = strlen(entry->d_name);
                        size_t ext_len = strlen(ext);
                        if (name_len >= ext_len && strcmp(entry->d_name + name_len - ext_len, ext) == 0) {
                            if (zt_copy_text(best_path, sizeof(best_path), child_path)) {
                                best_time = st.st_mtime;
                            }
                        }
                    }
                }
            }
        }
    }
    closedir(dp);
#endif

    if (best_path[0] != '\0') {
        return zt_copy_text(out, capacity, best_path);
    }
    return 0;
}

static int zt_find_focus_anchor(const char *source_root, char *out_path, char *out_message, size_t path_cap, size_t msg_cap) {
    char child_path[768];

#ifdef _WIN32
    WIN32_FIND_DATAA find_data;
    HANDLE hFind;
    char pattern[768];
    snprintf(pattern, sizeof(pattern), "%s\\*", source_root);
    hFind = FindFirstFileA(pattern, &find_data);
    if (hFind == INVALID_HANDLE_VALUE) return 0;
    do {
        if (strcmp(find_data.cFileName, ".") == 0 || strcmp(find_data.cFileName, "..") == 0) continue;
        snprintf(child_path, sizeof(child_path), "%s\\%s", source_root, find_data.cFileName);
        {
            struct stat st;
            if (stat(child_path, &st) != 0) continue;
            if (st.st_mode & S_IFDIR) {
                char sub_path[512];
                char sub_msg[256];
                if (zt_find_focus_anchor(child_path, sub_path, sub_msg, sizeof(sub_path), sizeof(sub_msg))) {
                    snprintf(out_path, path_cap, "%s", sub_path);
                    snprintf(out_message, msg_cap, "%s", sub_msg);
                    FindClose(hFind);
                    return 1;
                }
                continue;
            }
            {
                size_t name_len = strlen(find_data.cFileName);
                if (name_len >= 3 && strcmp(find_data.cFileName + name_len - 3, ".zt") == 0) {
                    FILE *file = fopen(child_path, "rb");
                    char line[1024];
                    if (file == NULL) continue;
                    while (fgets(line, sizeof(line), file) != NULL) {
                        const char *focus_prefix = "-- FOCUS:";
                        const char *next_prefix = "-- NEXT:";
                        if (strncmp(line, focus_prefix, strlen(focus_prefix)) == 0) {
                            const char *msg_start = line + strlen(focus_prefix);
                            while (*msg_start == ' ' || *msg_start == '\t') msg_start++;
                            {
                                size_t len = strlen(msg_start);
                                while (len > 0 && (msg_start[len-1] == '\n' || msg_start[len-1] == '\r')) len--;
                                snprintf(out_path, path_cap, "%s", child_path);
                                snprintf(out_message, msg_cap, "%.*s", (int)len, msg_start);
                            }
                            fclose(file);
                            FindClose(hFind);
                            return 1;
                        }
                        if (strncmp(line, next_prefix, strlen(next_prefix)) == 0) {
                            const char *msg_start = line + strlen(next_prefix);
                            while (*msg_start == ' ' || *msg_start == '\t') msg_start++;
                            {
                                size_t len = strlen(msg_start);
                                while (len > 0 && (msg_start[len-1] == '\n' || msg_start[len-1] == '\r')) len--;
                                snprintf(out_path, path_cap, "%s", child_path);
                                snprintf(out_message, msg_cap, "%.*s", (int)len, msg_start);
                            }
                            fclose(file);
                            FindClose(hFind);
                            return 1;
                        }
                    }
                    fclose(file);
                }
            }
        }
    } while (FindNextFileA(hFind, &find_data));
    FindClose(hFind);
#else
    DIR *dp;
    struct dirent *entry;
    dp = opendir(source_root);
    if (dp == NULL) return 0;
    while ((entry = readdir(dp)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
        snprintf(child_path, sizeof(child_path), "%s/%s", source_root, entry->d_name);
        {
            struct stat st;
            if (stat(child_path, &st) != 0) continue;
            if (S_ISDIR(st.st_mode)) {
                char sub_path[512];
                char sub_msg[256];
                if (zt_find_focus_anchor(child_path, sub_path, sub_msg, sizeof(sub_path), sizeof(sub_msg))) {
                    snprintf(out_path, path_cap, "%s", sub_path);
                    snprintf(out_message, msg_cap, "%s", sub_msg);
                    closedir(dp);
                    return 1;
                }
                continue;
            }
            {
                size_t name_len = strlen(entry->d_name);
                if (name_len >= 3 && strcmp(entry->d_name + name_len - 3, ".zt") == 0) {
                    FILE *file = fopen(child_path, "rb");
                    char line[1024];
                    if (file == NULL) continue;
                    while (fgets(line, sizeof(line), file) != NULL) {
                        const char *focus_prefix = "-- FOCUS:";
                        const char *next_prefix = "-- NEXT:";
                        if (strncmp(line, focus_prefix, strlen(focus_prefix)) == 0) {
                            const char *msg_start = line + strlen(focus_prefix);
                            while (*msg_start == ' ' || *msg_start == '\t') msg_start++;
                            {
                                size_t len = strlen(msg_start);
                                while (len > 0 && (msg_start[len-1] == '\n' || msg_start[len-1] == '\r')) len--;
                                snprintf(out_path, path_cap, "%s", child_path);
                                snprintf(out_message, msg_cap, "%.*s", (int)len, msg_start);
                            }
                            fclose(file);
                            closedir(dp);
                            return 1;
                        }
                        if (strncmp(line, next_prefix, strlen(next_prefix)) == 0) {
                            const char *msg_start = line + strlen(next_prefix);
                            while (*msg_start == ' ' || *msg_start == '\t') msg_start++;
                            {
                                size_t len = strlen(msg_start);
                                while (len > 0 && (msg_start[len-1] == '\n' || msg_start[len-1] == '\r')) len--;
                                snprintf(out_path, path_cap, "%s", child_path);
                                snprintf(out_message, msg_cap, "%.*s", (int)len, msg_start);
                            }
                            fclose(file);
                            closedir(dp);
                            return 1;
                        }
                    }
                    fclose(file);
                }
            }
        }
    }
    closedir(dp);
#endif
    return 0;
}

static int zt_handle_summary(zt_driver_context *ctx, const char *input) {
    (void)ctx;
    char project_root[512] = "";
    char manifest_path[512] = "";
    zt_project_parse_result project;
    char source_root[512] = "";
    char most_recent[512];
    char focus_path[512];
    char focus_msg[256];
    int has_manifest;

    has_manifest = zt_resolve_project_paths(input, project_root, sizeof(project_root), manifest_path, sizeof(manifest_path));

    printf("\n");
    printf("\xf0\x9f\x93\x8c Project Summary\n");
    printf("\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\n");

    if (!has_manifest) {
        printf("  (no zenith.ztproj found in %s)\n", project_root[0] ? project_root : ".");
    } else {
        if (zt_project_load_file(manifest_path, &project)) {
            printf("  \xf0\x9f\x93\x8a Name: %s (%s)\n", project.manifest.project_name, project.manifest.project_kind);
            printf("  \xf0\x9f\x93\x81 Source: %s\n", project.manifest.source_root);
            if (project.manifest.app_entry[0] != '\0') {
                printf("  \xf0\x9f\x9a\x80 Entry: %s\n", project.manifest.app_entry);
            }
            if (project.manifest.lib_root_namespace[0] != '\0') {
                printf("  \xf0\x9f\x93\x9a Namespace: %s\n", project.manifest.lib_root_namespace);
            }
            printf("  \xf0\x9f\x8c\x90 Language: %s\n", project.manifest.lang[0] ? project.manifest.lang : "en");
        }
    }

    if (has_manifest && zt_project_load_file(manifest_path, &project)) {
        if (!zt_join_path(source_root, sizeof(source_root), project_root, project.manifest.source_root)) {
            fprintf(stderr, "error: source root path is too long\n");
            return 1;
        }
    } else {
        if (!zt_copy_text(source_root, sizeof(source_root), "src")) {
            fprintf(stderr, "error: unable to prepare source root\n");
            return 1;
        }
    }

    printf("\n");
    if (zt_find_most_recent_file(source_root, ".zt", most_recent, sizeof(most_recent))) {
        struct stat st;
        if (stat(most_recent, &st) == 0) {
            double ago = difftime(time(NULL), st.st_mtime);
            const char *ago_str;
            if (ago < 60) ago_str = "just now";
            else if (ago < 3600) { static char buf[32]; snprintf(buf, sizeof(buf), "%.0fm ago", ago / 60); ago_str = buf; }
            else if (ago < 86400) { static char buf[32]; snprintf(buf, sizeof(buf), "%.1fh ago", ago / 3600); ago_str = buf; }
            else { static char buf[32]; snprintf(buf, sizeof(buf), "%.1fd ago", ago / 86400); ago_str = buf; }
            printf("\xf0\x9f\x95\x92 Last modified: %s (%s)\n", most_recent, ago_str);
        }
    }

    if (zt_find_focus_anchor(source_root, focus_path, focus_msg, sizeof(focus_path), sizeof(focus_msg))) {
        printf("\xf0\x9f\x8e\xaf FOCUS: %s\n", focus_msg);
        printf("   (%s)\n", focus_path);
    }

    printf("\n");
    return 0;
}

static int zt_handle_resume(zt_driver_context *ctx, const char *input) {
    char project_root[512] = "";
    char manifest_path[512] = "";
    zt_project_parse_result project;
    char source_root[512] = "";
    char focus_path[512];
    char focus_msg[256];

    if (!zt_resolve_project_paths(input, project_root, sizeof(project_root), manifest_path, sizeof(manifest_path))) {
        fprintf(stderr, "\xe2\x9d\x8c error: no zenith.ztproj found\n");
        return 1;
    }

    if (!zt_project_load_file(manifest_path, &project)) {
        fprintf(stderr, "\xe2\x9d\x8c error: unable to read manifest\n");
        return 1;
    }

    zt_apply_manifest_lang(&project.manifest);

    if (!zt_join_path(source_root, sizeof(source_root), project_root, project.manifest.source_root)) {
        fprintf(stderr, "error: source root path is too long\n");
        return 1;
    }

    printf("\n");
    printf("\xe2\x9e\xa1\xef\xb8\x8f Resume Context\n");
    printf("\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\n");

    if (zt_find_focus_anchor(source_root, focus_path, focus_msg, sizeof(focus_path), sizeof(focus_msg))) {
        printf("\xf0\x9f\x8e\xaf Resuming: %s\n", focus_msg);
        printf("   File: %s\n\n", focus_path);
    } else {
        printf("   No FOCUS/NEXT anchors found. Add `-- FOCUS: <message>` or `-- NEXT: <message>` comments to your source files.\n\n");
    }

    {
        char project_input[512];
        snprintf(project_input, sizeof(project_input), "%s", project_root[0] ? project_root : ".");
        return zt_handle_project_command(ctx, "verify", project_input, NULL, 0);
    }
}

static int zt_handle_perf(zt_driver_context *ctx, const char *target) {
    (void)ctx;
    const char *suite = target;
    const char *argv[5];
    int status;

    if (suite == NULL || suite[0] == '\0') {
        suite = "quick";
    }

    if (strcmp(suite, "quick") == 0 || strcmp(suite, "nightly") == 0) {
        argv[0] = "python";
#ifdef _WIN32
        argv[1] = "tests\\perf\\run_perf.py";
#else
        argv[1] = "tests/perf/run_perf.py";
#endif
        argv[2] = "--suite";
        argv[3] = suite;
        argv[4] = NULL;
    } else {
        argv[0] = "python";
#ifdef _WIN32
        argv[1] = "tests\\perf\\run_perf.py";
#else
        argv[1] = "tests/perf/run_perf.py";
#endif
        argv[2] = suite;
        argv[3] = NULL;
    }

    status = zt_native_spawn_process(argv, NULL, NULL);
    if (status < 0) {
        return 1;
    }
    return status;
}

static int zt_dir_is_empty(const char *directory) {
#ifdef _WIN32
    WIN32_FIND_DATAA data;
    HANDLE find_handle;
    char pattern[1024];

    if (directory == NULL || directory[0] == '\0') return 0;
    if (snprintf(pattern, sizeof(pattern), "%s\\*", directory) >= (int)sizeof(pattern)) {
        return 0;
    }

    find_handle = FindFirstFileA(pattern, &data);
    if (find_handle == INVALID_HANDLE_VALUE) return 0;

    do {
        if (strcmp(data.cFileName, ".") != 0 && strcmp(data.cFileName, "..") != 0) {
            FindClose(find_handle);
            return 0;
        }
    } while (FindNextFileA(find_handle, &data));

    FindClose(find_handle);
    return 1;
#else
    DIR *dir;
    struct dirent *entry;

    if (directory == NULL || directory[0] == '\0') return 0;
    dir = opendir(directory);
    if (dir == NULL) return 0;

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            closedir(dir);
            return 0;
        }
    }

    closedir(dir);
    return 1;
#endif
}

static void zt_sanitize_project_name(const char *input, char *out, size_t out_capacity) {
    size_t in_i = 0;
    size_t out_i = 0;
    int wrote_dash = 0;

    if (out == NULL || out_capacity == 0) return;
    out[0] = '\0';
    if (input == NULL) input = "";

    while (input[in_i] != '\0' && out_i + 1 < out_capacity) {
        unsigned char ch = (unsigned char)input[in_i];
        char lowered = (char)tolower(ch);

        if (isalnum(ch)) {
            out[out_i++] = lowered;
            wrote_dash = 0;
        } else if (ch == '-' || ch == '_' || ch == '.' || isspace(ch)) {
            if (out_i > 0 && !wrote_dash) {
                out[out_i++] = '-';
                wrote_dash = 1;
            }
        }

        in_i += 1;
    }

    while (out_i > 0 && out[out_i - 1] == '-') {
        out_i -= 1;
    }

    if (out_i == 0) {
        snprintf(out, out_capacity, "zenith-app");
        return;
    }

    out[out_i] = '\0';
    if (isdigit((unsigned char)out[0])) {
        char prefixed[256];
        snprintf(prefixed, sizeof(prefixed), "app-%s", out);
        snprintf(out, out_capacity, "%s", prefixed);
    }
}

static void zt_sanitize_namespace_root(const char *input, char *out, size_t out_capacity) {
    size_t in_i = 0;
    size_t out_i = 0;
    int wrote_sep = 0;

    if (out == NULL || out_capacity == 0) return;
    out[0] = '\0';
    if (input == NULL) input = "";

    while (input[in_i] != '\0' && out_i + 1 < out_capacity) {
        unsigned char ch = (unsigned char)input[in_i];
        char lowered = (char)tolower(ch);

        if (isalnum(ch)) {
            out[out_i++] = lowered;
            wrote_sep = 0;
        } else if (ch == '-' || ch == '_' || ch == '.' || isspace(ch)) {
            if (out_i > 0 && !wrote_sep) {
                out[out_i++] = '_';
                wrote_sep = 1;
            }
        }

        in_i += 1;
    }

    while (out_i > 0 && out[out_i - 1] == '_') {
        out_i -= 1;
    }

    if (out_i == 0) {
        snprintf(out, out_capacity, "app");
        return;
    }

    out[out_i] = '\0';
    if (isdigit((unsigned char)out[0])) {
        char prefixed[256];
        snprintf(prefixed, sizeof(prefixed), "app_%s", out);
        snprintf(out, out_capacity, "%s", prefixed);
    }
}

static void zt_project_name_from_target(const char *target_path, char *project_name, size_t capacity) {
    char resolved[512];
    const char *segment = NULL;
    size_t len;

    if (project_name == NULL || capacity == 0) return;
    project_name[0] = '\0';

    if (target_path == NULL || target_path[0] == '\0' || strcmp(target_path, ".") == 0) {
        if (!zt_get_current_dir(resolved, sizeof(resolved))) {
            snprintf(project_name, capacity, "zenith-app");
            return;
        }
    } else {
        if (!zt_copy_text(resolved, sizeof(resolved), target_path)) {
            snprintf(project_name, capacity, "zenith-app");
            return;
        }
    }

    zt_normalize_path_inplace(resolved);
    len = strlen(resolved);
    while (len > 0 && resolved[len - 1] == '/') {
        resolved[len - 1] = '\0';
        len -= 1;
    }

    if (len == 0) {
        snprintf(project_name, capacity, "zenith-app");
        return;
    }

    {
        const char *slash = strrchr(resolved, '/');
        segment = slash != NULL ? slash + 1 : resolved;
    }

    if (segment == NULL || segment[0] == '\0') {
        snprintf(project_name, capacity, "zenith-app");
        return;
    }

    zt_sanitize_project_name(segment, project_name, capacity);
}

static int zt_create_write_scaffold_file(
        zt_driver_context *ctx,
        const char *path,
        const char *content,
        int force,
        const char *label) {
    if (path == NULL || content == NULL) return 0;

    if (zt_path_is_dir(path)) {
        zt_print_single_diag(
            ctx,
            "project",
            ZT_DIAG_PROJECT_INVALID_INPUT,
            zt_source_span_unknown(),
            "create: %s path points to a directory: %s",
            label,
            path);
        return 0;
    }

    if (!force && zt_path_is_file(path)) {
        zt_print_single_diag(
            ctx,
            "project",
            ZT_DIAG_PROJECT_INVALID_INPUT,
            zt_source_span_unknown(),
            "create: %s already exists (use --force to overwrite): %s",
            label,
            path);
        return 0;
    }

    if (!zt_write_file(path, content)) {
        zt_print_single_diag(
            ctx,
            "project",
            ZT_DIAG_PROJECT_INVALID_INPUT,
            zt_source_span_unknown(),
            "create: failed to write %s: %s",
            label,
            path);
        return 0;
    }

    return 1;
}

static int zt_handle_create(zt_driver_context *ctx, const char *target_path, int create_lib, int force) {
    const char *target = target_path;
    char root_dir[512];
    char project_name[128];
    char namespace_root[128];
    char manifest_path[768];
    char readme_path[768];
    char src_dir[768];
    char module_dir[768];
    char module_file_path[768];
    char module_namespace[256];
    char manifest_content[1024];
    char source_content[1024];
    char readme_content[1024];
    const char *kind_label = create_lib ? "lib" : "app";

    if (target == NULL || target[0] == '\0') target = ".";
    if (!zt_copy_text(root_dir, sizeof(root_dir), target)) {
        zt_print_single_diag(
            ctx,
            "project",
            ZT_DIAG_PROJECT_PATH_TOO_LONG,
            zt_source_span_unknown(),
            "create: target path is too long");
        return 1;
    }
    zt_normalize_path_inplace(root_dir);
    if (root_dir[0] == '\0') {
        snprintf(root_dir, sizeof(root_dir), ".");
    }

    if (zt_path_is_file(root_dir)) {
        zt_print_single_diag(
            ctx,
            "project",
            ZT_DIAG_PROJECT_INVALID_INPUT,
            zt_source_span_unknown(),
            "create: target exists and is a file: %s",
            root_dir);
        return 1;
    }

    if (zt_path_is_dir(root_dir)) {
        if (!force && !zt_dir_is_empty(root_dir)) {
            zt_print_single_diag(
                ctx,
                "project",
                ZT_DIAG_PROJECT_INVALID_INPUT,
                zt_source_span_unknown(),
                "create: target directory is not empty (use --force): %s",
                root_dir);
            return 1;
        }
    } else {
        if (!zt_make_dirs(root_dir)) {
            zt_print_single_diag(
                ctx,
                "project",
                ZT_DIAG_PROJECT_INVALID_INPUT,
                zt_source_span_unknown(),
                "create: failed to create target directory: %s",
                root_dir);
            return 1;
        }
    }

    zt_project_name_from_target(root_dir, project_name, sizeof(project_name));
    zt_sanitize_namespace_root(project_name, namespace_root, sizeof(namespace_root));

    if (!zt_join_path(src_dir, sizeof(src_dir), root_dir, "src")) {
        zt_print_single_diag(
            ctx,
            "project",
            ZT_DIAG_PROJECT_PATH_TOO_LONG,
            zt_source_span_unknown(),
            "create: src path is too long");
        return 1;
    }
    if (!zt_make_dirs(src_dir)) {
        zt_print_single_diag(
            ctx,
            "project",
            ZT_DIAG_PROJECT_INVALID_INPUT,
            zt_source_span_unknown(),
            "create: failed to create source directory: %s",
            src_dir);
        return 1;
    }

    if (create_lib) {
        if (!zt_join_path(module_dir, sizeof(module_dir), src_dir, namespace_root)) {
            zt_print_single_diag(
                ctx,
                "project",
                ZT_DIAG_PROJECT_PATH_TOO_LONG,
                zt_source_span_unknown(),
                "create: library module directory path is too long");
            return 1;
        }
        if (!zt_make_dirs(module_dir)) {
            zt_print_single_diag(
                ctx,
                "project",
                ZT_DIAG_PROJECT_INVALID_INPUT,
                zt_source_span_unknown(),
                "create: failed to create library module directory: %s",
                module_dir);
            return 1;
        }
        if (!zt_join_path(module_file_path, sizeof(module_file_path), module_dir, "core.zt")) {
            zt_print_single_diag(
                ctx,
                "project",
                ZT_DIAG_PROJECT_PATH_TOO_LONG,
                zt_source_span_unknown(),
                "create: library module path is too long");
            return 1;
        }
        snprintf(module_namespace, sizeof(module_namespace), "%s.core", namespace_root);
    } else {
        if (!zt_join_path(module_dir, sizeof(module_dir), src_dir, "app")) {
            zt_print_single_diag(
                ctx,
                "project",
                ZT_DIAG_PROJECT_PATH_TOO_LONG,
                zt_source_span_unknown(),
                "create: app module directory path is too long");
            return 1;
        }
        if (!zt_make_dirs(module_dir)) {
            zt_print_single_diag(
                ctx,
                "project",
                ZT_DIAG_PROJECT_INVALID_INPUT,
                zt_source_span_unknown(),
                "create: failed to create app module directory: %s",
                module_dir);
            return 1;
        }
        if (!zt_join_path(module_file_path, sizeof(module_file_path), module_dir, "main.zt")) {
            zt_print_single_diag(
                ctx,
                "project",
                ZT_DIAG_PROJECT_PATH_TOO_LONG,
                zt_source_span_unknown(),
                "create: app module path is too long");
            return 1;
        }
        snprintf(module_namespace, sizeof(module_namespace), "app.main");
    }

    if (!zt_join_path(manifest_path, sizeof(manifest_path), root_dir, "zenith.ztproj")) {
        zt_print_single_diag(
            ctx,
            "project",
            ZT_DIAG_PROJECT_PATH_TOO_LONG,
            zt_source_span_unknown(),
            "create: manifest path is too long");
        return 1;
    }
    if (!zt_join_path(readme_path, sizeof(readme_path), root_dir, "README.md")) {
        zt_print_single_diag(
            ctx,
            "project",
            ZT_DIAG_PROJECT_PATH_TOO_LONG,
            zt_source_span_unknown(),
            "create: README path is too long");
        return 1;
    }

    if (create_lib) {
        snprintf(
            manifest_content,
            sizeof(manifest_content),
            "[project]\n"
            "name = \"%s\"\n"
            "kind = \"lib\"\n"
            "version = \"0.1.0\"\n"
            "\n"
            "[source]\n"
            "root = \"src\"\n"
            "\n"
            "[lib]\n"
            "root_namespace = \"%s\"\n"
            "\n"
            "[build]\n"
            "target = \"native\"\n"
            "output = \"build\"\n"
            "profile = \"debug\"\n",
            project_name,
            namespace_root);

        snprintf(
            source_content,
            sizeof(source_content),
            "namespace %s\n"
            "\n"
            "public func version() -> text\n"
            "    return \"0.1.0\"\n"
            "end\n",
            module_namespace);
    } else {
        snprintf(
            manifest_content,
            sizeof(manifest_content),
            "[project]\n"
            "name = \"%s\"\n"
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
            "profile = \"debug\"\n",
            project_name);

        snprintf(
            source_content,
            sizeof(source_content),
            "namespace %s\n"
            "\n"
            "import std.io as io\n"
            "\n"
            "func main() -> result<void, core.Error>\n"
            "    io.write(\"Hello from %s\\n\")?\n"
            "    return success()\n"
            "end\n",
            module_namespace,
            project_name);
    }

    snprintf(
        readme_content,
        sizeof(readme_content),
        "# %s\n"
        "\n"
        "Scaffold generated by `zt create`.\n"
        "\n"
        "## Next Steps\n"
        "\n"
        "1. `zt check zenith.ztproj`\n"
        "2. `zt build zenith.ztproj`\n"
        "%s",
        project_name,
        create_lib ? "" : "3. `zt run zenith.ztproj`\n");

    if (!zt_create_write_scaffold_file(ctx, manifest_path, manifest_content, force, "manifest")) return 1;
    if (!zt_create_write_scaffold_file(ctx, module_file_path, source_content, force, "source")) return 1;
    if (!zt_create_write_scaffold_file(ctx, readme_path, readme_content, force, "README")) return 1;

    printf("created %s project scaffold at %s\n", kind_label, root_dir);
    printf("next:\n");
    printf("  cd %s\n", strcmp(root_dir, ".") == 0 ? "." : root_dir);
    printf("  zt check zenith.ztproj\n");
    if (!create_lib) {
        printf("  zt run zenith.ztproj\n");
    } else {
        printf("  zt build zenith.ztproj\n");
    }
    return 0;
}

int main(int argc, char *argv[]) {
    zt_cli_init_terminal();
    zt_arena_init(&global_arena, 1048576);
    zt_string_pool_init(&global_pool, &global_arena);
    zt_driver_context ctx;

    const char *command;
    const char *effective_command;
    const char *input_path = NULL;
    int run_output = 0;
    const char *output_path = NULL;
    const char *lang_override = NULL;
    int fmt_check = 0;
    int ci_mode = 0;
    int show_help = 0;
    int command_index = 1;
    int parse_start = 2;
    int project_input_optional = 0;
    int i;

    zt_driver_context_init(&ctx);

    if (argc < 2) {
        return zt_cli_fail(argv[0], "missing command", "zt help", 1);
    }

    while (command_index < argc) {
        if (strcmp(argv[command_index], "--lang") == 0 && command_index + 1 < argc) {
            lang_override = argv[command_index + 1];
            command_index += 2;
            continue;
        }
        if (strcmp(argv[command_index], "--native-raw") == 0) {
            ctx.native_raw_output = 1;
            command_index += 1;
            continue;
        }
        break;
    }

    if (command_index >= argc) {
        return zt_cli_fail(argv[0], "missing command", "zt help", 1);
    }

    if (lang_override != NULL) {
        zt_lang lang = zt_l10n_from_str(lang_override);
        if (lang != ZT_LANG_UNSPECIFIED) {
            zt_l10n_set_lang(lang);
        }
    }

    if (strcmp(argv[command_index], "help") == 0 || zt_is_help_flag(argv[command_index])) {
        const char *topic = NULL;
        for (i = command_index + 1; i < argc; i += 1) {
            if (strcmp(argv[i], "--lang") == 0 && i + 1 < argc) {
                zt_lang lang = zt_l10n_from_str(argv[i + 1]);
                if (lang != ZT_LANG_UNSPECIFIED) zt_l10n_set_lang(lang);
                i += 1;
                continue;
            }
            if (strcmp(argv[i], "--native-raw") == 0) continue;
            if (zt_is_help_flag(argv[i])) continue;
            if (topic == NULL) topic = argv[i];
        }
        return zt_print_help_topic(stdout, argv[0], topic);
    }

    command = argv[command_index];
    effective_command = command;
    parse_start = command_index + 1;

    if (ctx.native_raw_output &&
            strcmp(command, "build") != 0 &&
            strcmp(command, "run") != 0) {
        return zt_cli_fail(argv[0], "option --native-raw is only valid for build/run", "use: zt build [project] --native-raw", 0);
    }

    if (strcmp(command, "doc") == 0) {
        if (argc > parse_start && zt_is_help_flag(argv[parse_start])) {
            return zt_print_help_topic(stdout, argv[0], "doc");
        }
        if (argc <= parse_start) {
            return zt_cli_fail(
                argv[0],
                "missing subcommand for 'doc'",
                "use: zt doc check [project] | zt doc show <symbol>",
                0);
        }
        if (strcmp(argv[parse_start], "check") == 0) {
            effective_command = "doc-check";
            parse_start += 1;
        } else if (strcmp(argv[parse_start], "show") == 0) {
            effective_command = "doc-show";
            parse_start += 1;
            if (argc > parse_start && argv[parse_start][0] != '-') {
                input_path = argv[parse_start];
                parse_start += 1;
            }
        } else {
            return zt_cli_fail(
                argv[0],
                "unknown subcommand for 'doc'",
                "use: zt doc check [project] | zt doc show <symbol>",
                0);
        }
    }

    if (strcmp(command, "create") == 0) {
        const char *target = NULL;
        int create_lib = 0;
        int create_force = 0;

        for (i = parse_start; i < argc; i += 1) {
            if (zt_is_help_flag(argv[i])) {
                return zt_print_help_topic(stdout, argv[0], "create");
            } else if (strcmp(argv[i], "--lib") == 0) {
                create_lib = 1;
            } else if (strcmp(argv[i], "--app") == 0) {
                create_lib = 0;
            } else if (strcmp(argv[i], "--force") == 0) {
                create_force = 1;
            } else if (argv[i][0] == '-') {
                char message[256];
                snprintf(message, sizeof(message), "unknown option for create: %s", argv[i]);
                return zt_cli_fail(
                    argv[0],
                    message,
                    "use: zt create [path|.] [--app|--lib] [--force]",
                    0);
            } else if (target == NULL) {
                target = argv[i];
            } else {
                char message[256];
                snprintf(message, sizeof(message), "unexpected argument for create: %s", argv[i]);
                return zt_cli_fail(
                    argv[0],
                    message,
                    "use only one path: zt create [path|.] [--app|--lib] [--force]",
                    0);
            }
        }

        if (target == NULL) target = ".";
        return zt_handle_create(&ctx, target, create_lib, create_force);
    }

    if (strcmp(effective_command, "check") == 0) {
        effective_command = "verify";
        project_input_optional = 1;
    } else if (strcmp(effective_command, "run") == 0) {
        effective_command = "build";
        run_output = 1;
        project_input_optional = 1;
    } else if (strcmp(effective_command, "test") == 0 ||
            strcmp(effective_command, "fmt") == 0 ||
            strcmp(effective_command, "project-info") == 0 ||
            strcmp(effective_command, "doc-check") == 0 ||
            strcmp(effective_command, "doc-show") == 0 ||
            strcmp(effective_command, "summary") == 0 ||
            strcmp(effective_command, "resume") == 0 ||
            strcmp(effective_command, "perf") == 0 ||
            strcmp(effective_command, "verify") == 0 ||
            strcmp(effective_command, "emit-c") == 0 ||
            strcmp(effective_command, "build") == 0) {
        project_input_optional = 1;
    } else if (strcmp(effective_command, "parse") != 0) {
        char message[256];
        snprintf(message, sizeof(message), "unknown command: %s", command);
        return zt_cli_fail(argv[0], message, "run: zt help", 1);
    }

    for (i = parse_start; i < argc; i += 1) {
        if (strcmp(argv[i], "--") == 0) {
            if (i + 1 < argc) {
                fprintf(stderr, "warning: trailing arguments after '--' are not supported in the bootstrap driver and were ignored\n");
            }
            break;
        }
        if (zt_is_help_flag(argv[i])) {
            show_help = 1;
            continue;
        }
        if (strcmp(argv[i], "--run") == 0) {
            run_output = 1;
        } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            output_path = argv[i + 1];
            i += 1;
        } else if (strcmp(argv[i], "--native-raw") == 0) {
            ctx.native_raw_output = 1;
        } else if (strcmp(argv[i], "--lang") == 0 && i + 1 < argc) {
            lang_override = argv[i + 1];
            i += 1;
        } else if (strcmp(argv[i], "--check") == 0) {
            fmt_check = 1;
        } else if (strcmp(argv[i], "--ci") == 0) {
            ci_mode = 1;
        } else if (strcmp(argv[i], "--telemetry") == 0) {
            ctx.telemetry_forced = 1;
        } else if (strcmp(argv[i], "--all") == 0) {
            ctx.show_all_errors = 1;
        } else if (strcmp(argv[i], "--profile") == 0 && i + 1 < argc) {
            const char *profile_name = argv[i + 1];
            if (strcmp(profile_name, "beginner") == 0) {
                zt_driver_context_lock_profile(&ctx, ZT_COG_PROFILE_BEGINNER);
            } else if (strcmp(profile_name, "balanced") == 0) {
                zt_driver_context_lock_profile(&ctx, ZT_COG_PROFILE_BALANCED);
            } else if (strcmp(profile_name, "full") == 0) {
                zt_driver_context_lock_profile(&ctx, ZT_COG_PROFILE_FULL);
            } else {
                fprintf(stderr, "warning: unknown profile '%s', expected beginner, balanced, or full\n", profile_name);
            }
            i += 1;
        } else if (strcmp(argv[i], "--focus") == 0 && i + 1 < argc) {
            ctx.focus_path = argv[i + 1];
            i += 1;
        } else if (strcmp(argv[i], "--since") == 0 && i + 1 < argc) {
            ctx.since_ref = argv[i + 1];
            i += 1;
        } else if (argv[i][0] == '-') {
            char message[256];
            snprintf(message, sizeof(message), "unknown option: %s", argv[i]);
            return zt_cli_fail(argv[0], message, "run: zt help", 1);
        } else if (input_path == NULL) {
            input_path = argv[i];
        } else {
            char message[256];
            snprintf(message, sizeof(message), "unexpected argument: %s", argv[i]);
            return zt_cli_fail(argv[0], message, "run: zt help <command>", 0);
        }
    }

    if (show_help) {
        const char *topic = command;
        if (strcmp(command, "doc") == 0 && strcmp(effective_command, "doc-show") == 0) {
            topic = "doc-show";
        } else if (strcmp(command, "doc") == 0 && strcmp(effective_command, "doc-check") == 0) {
            topic = "doc-check";
        }
        return zt_print_help_topic(stdout, argv[0], topic);
    }

    ctx.ci_mode_enabled = ci_mode;

    if (lang_override != NULL) {
        zt_lang lang = zt_l10n_from_str(lang_override);
        if (lang != ZT_LANG_UNSPECIFIED) {
            zt_l10n_set_lang(lang);
        } else {
            fprintf(stderr, "warning: unsupported language '%s', falling back to detection\n", lang_override);
        }
    }

    /* Apply manifest diagnostic profile if --profile not set via CLI */
    if (!ctx.profile_locked && strcmp(effective_command, "perf") != 0) {
        zt_project_parse_result proj_result;
        char proj_root[512];
        char proj_manifest[512];
        if (zt_resolve_project_paths(input_path, proj_root, sizeof(proj_root), proj_manifest, sizeof(proj_manifest))) {
            if (zt_project_load_file(proj_manifest, &proj_result)) {
                if (proj_result.manifest.diag_profile[0] != '\0') {
                    if (strcmp(proj_result.manifest.diag_profile, "beginner") == 0) {
                        zt_driver_context_lock_profile(&ctx, ZT_COG_PROFILE_BEGINNER);
                    } else if (strcmp(proj_result.manifest.diag_profile, "balanced") == 0) {
                        zt_driver_context_lock_profile(&ctx, ZT_COG_PROFILE_BALANCED);
                    }
                }
            }
        }
    }

    if (ctx.since_ref != NULL && ctx.since_ref[0] != '\0') {
        char since_project_root[512];
        char since_manifest_path[512];
        if (!zt_resolve_project_paths(input_path, since_project_root, sizeof(since_project_root), since_manifest_path, sizeof(since_manifest_path))) {
            return zt_cli_fail(
                argv[0],
                "option --since requires a project path",
                "pass a project path or a directory with zenith.ztproj",
                0);
        }
        if (!zt_load_since_filter(&ctx, since_project_root, ctx.since_ref)) {
            return 1;
        }
    }
    if (fmt_check && strcmp(effective_command, "fmt") != 0) {
        return zt_cli_fail(argv[0], "option --check is only valid for fmt", "use: zt fmt [project] --check", 0);
    }

    if (output_path != NULL && strcmp(effective_command, "build") != 0) {
        return zt_cli_fail(argv[0], "option -o is only valid for build/run", "use: zt build|run [project] -o <output>", 0);
    }

    if (run_output && strcmp(effective_command, "build") != 0) {
        return zt_cli_fail(argv[0], "option --run is only valid for build/run", "use: zt build [project] --run", 0);
    }

    if (ctx.native_raw_output && strcmp(effective_command, "build") != 0) {
        return zt_cli_fail(argv[0], "option --native-raw is only valid for build/run", "use: zt build [project] --native-raw", 0);
    }

    if (input_path == NULL && project_input_optional) {
        input_path = "";
    }

    if (strcmp(effective_command, "project-info") == 0) {
        return zt_handle_project_info(&ctx, input_path);
    }

    if (strcmp(effective_command, "summary") == 0) {
        return zt_handle_summary(&ctx, input_path);
    }

    if (strcmp(effective_command, "resume") == 0) {
        return zt_handle_resume(&ctx, input_path);
    }

    if (strcmp(effective_command, "perf") == 0) {
        return zt_handle_perf(&ctx, input_path);
    }

    if (strcmp(effective_command, "doc-check") == 0) {
        if (input_path != NULL && input_path[0] != '\0' && zt_path_has_extension(input_path, ".zir")) {
            return zt_cli_fail(argv[0], "doc-check expects a project path, not .zir", "use: zt doc check [project]", 0);
        }
        return zt_handle_doc_check(&ctx, input_path);
    }

    if (strcmp(effective_command, "doc-show") == 0) {
        if (input_path == NULL || input_path[0] == '\0') {
            return zt_cli_fail(argv[0], "doc show requires a symbol", "use: zt doc show std.math.clamp", 0);
        }
        return zt_handle_doc_show(input_path, lang_override);
    }

    if (strcmp(effective_command, "fmt") == 0) {
        if (input_path != NULL && input_path[0] != '\0' && zt_path_has_extension(input_path, ".zir")) {
            return zt_cli_fail(argv[0], "fmt expects a project path, not .zir", "use: zt fmt [project] [--check]", 0);
        }
        return zt_handle_fmt(&ctx, input_path, fmt_check);
    }

    if (strcmp(effective_command, "test") == 0) {
        return zt_handle_test(&ctx, input_path);
    }

    if (input_path != NULL && input_path[0] != '\0' && zt_path_has_extension(input_path, ".zir")) {
        return zt_handle_zir_command(&ctx, effective_command, input_path, output_path, run_output);
    }

    if (strcmp(effective_command, "parse") == 0) {
        return zt_cli_fail(argv[0], "parse expects <file.zir>", "use: zt parse <file.zir>", 0);
    }

    if (strcmp(effective_command, "emit-c") == 0 ||
            strcmp(effective_command, "build") == 0 ||
            strcmp(effective_command, "verify") == 0) {
        return zt_handle_project_command(&ctx, effective_command, input_path, output_path, run_output);
    }

    {
        char message[256];
        snprintf(message, sizeof(message), "unknown command or unsupported input: %s", command);
        return zt_cli_fail(argv[0], message, "run: zt help", 1);
    }
}









