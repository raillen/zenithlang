/*
 * driver_internal.h — Shared internal declarations for the compiler driver.
 *
 * This header defines types, globals, and function prototypes shared
 * across the driver modules (main.c, paths.c, project.c).
 *
 * This is an INTERNAL header. Do not include from outside compiler/driver/.
 */

#ifndef ZT_DRIVER_INTERNAL_H
#define ZT_DRIVER_INTERNAL_H

#include "compiler/utils/arena.h"
#include "compiler/utils/string_pool.h"
#include "compiler/frontend/parser/parser.h"
#include "compiler/frontend/lexer/lexer.h"
#include "compiler/hir/lowering/from_ast.h"
#include "compiler/project/ztproj.h"
#include "compiler/project/zdoc.h"
#include "compiler/semantic/binder/binder.h"
#include "compiler/semantic/types/checker.h"
#include "compiler/targets/c/emitter.h"
#include "compiler/zir/lowering/from_hir.h"
#include "compiler/zir/parser.h"
#include "compiler/zir/verifier.h"
#include "compiler/semantic/diagnostics/diagnostics.h"
#include "compiler/tooling/formatter.h"
#include "compiler/utils/l10n.h"
#include "compiler/driver/doc_show.h"
#include "runtime/c/zenith_rt.h"

#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#include <windows.h>
#define ZT_MKDIR(path) _mkdir(path)
#define ZT_GETCWD(buffer, size) _getcwd((buffer), (int)(size))
#define ZT_POPEN(command, mode) _popen((command), (mode))
#define ZT_PCLOSE(handle) _pclose((handle))
#else
#include <dirent.h>
#include <sys/wait.h>
#include <unistd.h>
#define ZT_MKDIR(path) mkdir((path), 0755)
#define ZT_GETCWD(buffer, size) getcwd((buffer), (size))
#define ZT_POPEN(command, mode) popen((command), (mode))
#define ZT_PCLOSE(handle) pclose((handle))
#endif

/* ── Shared global state ─────────────────────────────────────────── */

extern zt_arena global_arena;
extern zt_string_pool global_pool;

extern int zt_ci_mode_enabled;
extern zt_cog_profile zt_active_profile;
extern int zt_show_all_errors;
extern const char *zt_focus_path;
extern const char *zt_since_ref;
extern int zt_use_action_first;
extern const zt_project_manifest *zt_active_manifest;
extern int zt_telemetry_enabled;
extern int zt_native_raw_output;
extern char zt_project_root_abs[512];

/* ── Path filter list ────────────────────────────────────────────── */

typedef struct zt_path_filter_list {
    char **items;
    size_t count;
    size_t capacity;
} zt_path_filter_list;

extern zt_path_filter_list zt_since_filter;

/* ── Source file management ──────────────────────────────────────── */

typedef struct zt_project_source_file {
    char path[512];
    char *source_text;
    zt_parser_result parsed;
    int parsed_ready;
} zt_project_source_file;

typedef struct zt_project_source_file_list {
    zt_project_source_file *items;
    size_t count;
    size_t capacity;
} zt_project_source_file_list;

/* ── paths.c — File I/O and path manipulation ────────────────────── */

char *zt_read_file(const char *path);
int zt_write_file(const char *path, const char *content);
int zt_copy_text(char *dest, size_t capacity, const char *text);
char *zt_heap_strdup(const char *text);
int zt_path_has_extension(const char *path, const char *extension);
int zt_path_is_dir(const char *path);
int zt_path_is_file(const char *path);
int zt_join_path(char *dest, size_t capacity, const char *left, const char *right);
int zt_dirname(char *dest, size_t capacity, const char *path);
int zt_get_current_dir(char *dest, size_t capacity);
int zt_find_project_root_from_cwd(char *project_root, size_t capacity);
int zt_replace_extension(char *dest, size_t capacity, const char *path, const char *extension);
void zt_normalize_path_separators(char *path);
int zt_git_ref_is_safe(const char *ref);
int zt_make_dirs(const char *path);
void zt_normalize_path_inplace(char *path);
int zt_path_char_equal(char left, char right);
int zt_path_suffix_matches(const char *path, const char *suffix);
void zt_normalize_system_path(const char *input, char *output, size_t output_size);

/* ── project.c — Project source management ───────────────────────── */

void zt_project_source_file_list_init(zt_project_source_file_list *list);
void zt_project_source_file_list_dispose(zt_project_source_file_list *list);
int zt_project_source_file_list_push(zt_project_source_file_list *list, const char *path);
int zt_project_discover_zt_files(const char *directory, zt_project_source_file_list *files);
int zt_namespace_to_relative_path(const char *namespace_name, char *dest, size_t capacity);
const char *zt_last_namespace_segment(const char *namespace_name);
const zt_ast_node *zt_find_entry_root(const zt_project_source_file_list *files, const char *entry_namespace);
int zt_namespace_exists(const zt_project_source_file_list *files, const char *namespace_name);
int zt_namespace_is_std(const char *namespace_name);
size_t zt_find_namespace_index(const zt_project_source_file_list *files, const char *namespace_name);
int zt_detect_import_cycles(const zt_project_source_file_list *files);
int zt_validate_source_namespaces(
        const zt_project_source_file_list *files,
        const zt_project_manifest *manifest,
        zt_diag_list *diagnostics);
int zt_parse_project_sources(zt_project_source_file_list *files);
zt_ast_node *zt_build_combined_project_ast(
        zt_project_source_file_list *files,
        const zt_project_manifest *manifest);
int zt_load_since_filter(const char *project_root, const char *since_ref);
void zt_path_filter_list_dispose(zt_path_filter_list *list);
int zt_path_filter_list_push(zt_path_filter_list *list, const char *entry);

/* ── main.c — Diagnostics and CLI ────────────────────────────────── */

void zt_apply_manifest_lang(const zt_project_manifest *manifest);
void zt_print_diagnostics(const char *stage, const zt_diag_list *diagnostics);
void zt_print_single_diag(
        const char *stage,
        zt_diag_code code,
        zt_source_span span,
        const char *format,
        ...);
int zt_resolve_project_paths(
        const char *input_path,
        char *project_root,
        size_t project_root_capacity,
        char *manifest_path,
        size_t manifest_path_capacity);
void zt_print_project_parse_error(const char *manifest_path, const zt_project_parse_result *project);
zt_diag_code zt_diag_code_from_zir_verifier(zir_verifier_code code);
zt_diag_code zt_diag_code_from_zir_parse_error(zir_parse_error_code code);
zt_diag_code zt_diag_code_from_c_emit_error(c_emit_error_code code);
int zt_collect_project_sources(
        const char *input_path,
        zt_project_manifest *manifest,
        char *project_root,
        size_t project_root_capacity,
        zt_project_source_file_list *source_files);

/* ── pipeline.c — Compilation pipeline ───────────────────────────── */

typedef struct zt_project_compile_result {
    zt_project_manifest manifest;
    char project_root[512];
    char manifest_path[512];
    char entry_path[512];
    zt_project_source_file_list source_files;
    zir_lower_result zir;
    int has_zir;
} zt_project_compile_result;

void zt_project_compile_result_init(zt_project_compile_result *result);
void zt_project_compile_result_dispose(zt_project_compile_result *result);
int zt_compile_project(const char *input_path, zt_project_compile_result *out);
int zt_emit_module_to_c(
        const zir_module *module_decl,
        c_emitter *emitter,
        c_emit_result *out_error);
int zt_compile_c_file(const char *c_path, const char *exe_path, const zt_project_manifest *manifest);
int zt_run_executable(const char *exe_path);

#endif /* ZT_DRIVER_INTERNAL_H */
