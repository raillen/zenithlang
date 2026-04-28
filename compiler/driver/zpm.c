#include "compiler/driver/driver_internal.h"
#include "compiler/project/lockfile.h"

static void zt_zpm_print_help(FILE *out) {
    fprintf(out, "zt zpm <command> [options]\n");
    fprintf(out, "\n");
    fprintf(out, "commands:\n");
    fprintf(out, "  init [path|.]       initialize a new zenith project\n"
        "  add <pkg>[@version] add a dependency to zenith.ztproj\n"
        "  remove <pkg>        remove a dependency from zenith.ztproj\n"
        "  install [--locked]  install dependencies and generate or reuse zenith.lock\n"
        "  update [pkg]        update dependencies to their latest compatible versions\n"
        "  list                list installed dependencies\n"
        "  find <query>        search for a package\n"
        "  run <script>        run a script defined in zenith.ztproj\n"
        "  update-registry     sync the local package registry from remote\n"
        "  publish             validate and prepare project for publication\n"
        "  help                show this help message\n"
);
}

typedef struct zt_semver {
    int major;
    int minor;
    int patch;
} zt_semver;

typedef enum zt_semver_range_kind {
    ZT_SEMVER_EXACT = 0,
    ZT_SEMVER_CARET, /* ^1.2.3 */
    ZT_SEMVER_TILDE  /* ~1.2.3 */
} zt_semver_range_kind;

typedef struct zt_semver_range {
    zt_semver_range_kind kind;
    zt_semver version;
} zt_semver_range;

static int zt_semver_parse(const char *text, zt_semver *out) {
    char tail;
    if (text == NULL || out == NULL) return 0;
    if (text[0] == 'v') text++;
    if (sscanf(text, "%d.%d.%d%c", &out->major, &out->minor, &out->patch, &tail) != 3) return 0;
    return out->major >= 0 && out->minor >= 0 && out->patch >= 0;
}

static int zt_semver_range_parse(const char *text, zt_semver_range *out) {
    if (text[0] == '^') {
        out->kind = ZT_SEMVER_CARET;
        return zt_semver_parse(text + 1, &out->version);
    } else if (text[0] == '~') {
        out->kind = ZT_SEMVER_TILDE;
        return zt_semver_parse(text + 1, &out->version);
    } else {
        out->kind = ZT_SEMVER_EXACT;
        return zt_semver_parse(text, &out->version);
    }
}

static int zt_zpm_validate_semver_constraint(const char *version_text) {
    zt_semver_range range;
    if (version_text == NULL || version_text[0] == '\0') return 0;
    return zt_semver_range_parse(version_text, &range);
}

static void zt_zpm_print_invalid_semver(const char *name, const char *version_text) {
    fprintf(stderr, "error: dependency %s has invalid SemVer constraint: %s\n", name, version_text);
    fprintf(stderr, "help: use 1.2.3, ^1.2.3, or ~1.2.3; use an inline table for git/path dependencies\n");
}

static int zt_semver_satisfies(const zt_semver *v, const zt_semver_range *range) {
    const zt_semver *r = &range->version;
    if (range->kind == ZT_SEMVER_EXACT) {
        return v->major == r->major && v->minor == r->minor && v->patch == r->patch;
    }
    if (range->kind == ZT_SEMVER_CARET) {
        if (v->major != r->major) return 0;
        if (r->major > 0) return (v->minor > r->minor || (v->minor == r->minor && v->patch >= r->patch));
        /* For 0.x.y, it works like tilde */
        if (v->minor != r->minor) return 0;
        return v->patch >= r->patch;
    }
    if (range->kind == ZT_SEMVER_TILDE) {
        if (v->major != r->major || v->minor != r->minor) return 0;
        return v->patch >= r->patch;
    }
    return 0;
}

static int zt_semver_compare(const zt_semver *a, const zt_semver *b) {
    if (a->major != b->major) return a->major - b->major;
    if (a->minor != b->minor) return a->minor - b->minor;
    return a->patch - b->patch;
}

static int zt_zpm_resolve_remote_version(zt_driver_context *ctx, const char *url, const char *range_text, char *out_version, size_t out_cap) {
    char temp_file[512];
    FILE *f;
    char line[1024];
    zt_semver_range range;
    zt_semver best_v = { -1, -1, -1 };
    int found = 0;
    int spawn_failed = 0;

    (void)ctx;

    if (!zt_semver_range_parse(range_text, &range)) {
        /* If not a valid range, just return it as a literal ref (branch/tag) */
        zt_copy_text(out_version, out_cap, range_text);
        return 1;
    }

    if (!zt_native_make_temp_path(temp_file, sizeof(temp_file), "zpm-tags", ".log")) return 0;

    const char *argv[] = { "git", "ls-remote", "--tags", url, NULL };
    if (zt_native_spawn_process(argv, temp_file, &spawn_failed) != 0) {
        zt_native_remove_file_if_exists(temp_file);
        return 0;
    }

    f = fopen(temp_file, "r");
    if (!f) {
        zt_native_remove_file_if_exists(temp_file);
        return 0;
    }

    while (fgets(line, sizeof(line), f)) {
        /* Line format: <hash>\trefs/tags/<tag_name> */
        const char *tag = strstr(line, "refs/tags/");
        if (tag) {
            tag += 10;
            /* Strip trailing newline/junk */
            char tag_name[256];
            zt_copy_text(tag_name, sizeof(tag_name), tag);
            char *end = strchr(tag_name, '\n');
            if (end) *end = '\0';
            end = strchr(tag_name, '\r');
            if (end) *end = '\0';
            /* Strip ^{} if it's a peeled tag */
            char *peeled = strstr(tag_name, "^{}");
            if (peeled) *peeled = '\0';

            zt_semver v;
            if (zt_semver_parse(tag_name, &v)) {
                if (zt_semver_satisfies(&v, &range)) {
                    if (!found || zt_semver_compare(&v, &best_v) > 0) {
                        best_v = v;
                        found = 1;
                    }
                }
            }
        }
    }

    fclose(f);
    zt_native_remove_file_if_exists(temp_file);

    if (found) {
        snprintf(out_version, out_cap, "%d.%d.%d", best_v.major, best_v.minor, best_v.patch);
        return 1;
    }

    return 0;
}

#define ZT_ZPM_REGISTRY_URL "https://raw.githubusercontent.com/zenith-lang/registry/main/index.ztproj"
#define ZT_ZPM_BASE_PATH_CAP 1024
#define ZT_ZPM_PATH_CAP 2048
#define ZT_ZPM_LONG_PATH_CAP 4096

static int zt_zpm_sync_registry(zt_driver_context *ctx) {
    char cache_dir[ZT_ZPM_PATH_CAP];
    char cache_file[ZT_ZPM_LONG_PATH_CAP];
    char home[ZT_ZPM_BASE_PATH_CAP];
    
    if (!zt_native_get_home_dir(home, sizeof(home))) {
        zt_copy_text(home, sizeof(home), ".");
    }

    snprintf(cache_dir, sizeof(cache_dir), "%s/.zenith", home);
    snprintf(cache_file, sizeof(cache_file), "%s/registry.ztproj", cache_dir);
    
    zt_make_dirs(cache_dir);

    if (!ctx->quiet) printf("zpm: syncing registry from %s...\n", ZT_ZPM_REGISTRY_URL);

    /* Use curl if available (available on Windows 10+ and most Unix) */
    const char *argv[] = { "curl", "-s", "-L", "-o", cache_file, ZT_ZPM_REGISTRY_URL, NULL };
    int spawn_failed = 0;
    if (zt_native_spawn_process(argv, NULL, &spawn_failed) != 0) {
        if (!ctx->quiet) fprintf(stderr, "warning: registry sync failed (curl not found or network error)\n");
        return 0;
    }

    return 1;
}

static const char *zt_zpm_load_url_from_cache(const char *name) {
    static char url_buffer[512];
    char cache_file[ZT_ZPM_PATH_CAP];
    char home[ZT_ZPM_BASE_PATH_CAP];
    char *content;
    const char *line;

    if (!zt_native_get_home_dir(home, sizeof(home))) zt_copy_text(home, sizeof(home), ".");
    snprintf(cache_file, sizeof(cache_file), "%s/.zenith/registry.ztproj", home);

    content = zt_read_file(cache_file);
    if (!content) return NULL;

    /* Simple scan for 'name = "url"' in the file */
    line = strstr(content, name);
    while (line) {
        /* Verify it's at start of line or after whitespace */
        if (line == content || line[-1] == '\n' || line[-1] == ' ' || line[-1] == '\t') {
            const char *eq = strchr(line, '=');
            if (eq) {
                const char *quo = strchr(eq, '"');
                if (quo) {
                    const char *end = strchr(quo + 1, '"');
                    if (end) {
                        size_t len = (size_t)(end - (quo + 1));
                        if (len < sizeof(url_buffer)) {
                            memcpy(url_buffer, quo + 1, len);
                            url_buffer[len] = '\0';
                            free(content);
                            return url_buffer;
                        }
                    }
                }
            }
        }
        line = strstr(line + 1, name);
    }

    free(content);
    return NULL;
}

static const char *zt_zpm_get_package_url(const char *name) {
    const char *cached = zt_zpm_load_url_from_cache(name);
    if (cached) return cached;
    
    /* If name looks like a URL, return it as is */
    if (strstr(name, "://") || strstr(name, "git@")) return name;
    
    return NULL;
}

static int zt_zpm_resolve_project(zt_driver_context *ctx, const char *arg_path, char *root_out, size_t root_cap, char *manifest_out, size_t manifest_cap) {
    const char *effective_path = ctx->project_path ? ctx->project_path : arg_path;
    return zt_resolve_project_paths(effective_path, root_out, root_cap, manifest_out, manifest_cap);
}

static void zt_zpm_unquote_spec_value(const char *spec, char *out, size_t out_cap) {
    size_t len;
    size_t copy_len;

    if (out == NULL || out_cap == 0) return;
    out[0] = '\0';
    if (spec == NULL) return;

    len = strlen(spec);
    if (len >= 2 && spec[0] == '"' && spec[len - 1] == '"') {
        copy_len = len - 2;
        if (copy_len >= out_cap) copy_len = out_cap - 1;
        memcpy(out, spec + 1, copy_len);
        out[copy_len] = '\0';
        return;
    }

    zt_copy_text(out, out_cap, spec);
}

static void zt_zpm_lock_version_or_git(zt_driver_context *ctx, zt_lock_package *pkg, const char *known_url, const char *spec) {
    char raw_version[64];
    char resolved_version[64];

    if (pkg == NULL) return;
    zt_zpm_unquote_spec_value(spec, raw_version, sizeof(raw_version));

    if (known_url != NULL && known_url[0] != '\0') {
        pkg->source = ZT_LOCK_SOURCE_GIT;
        zt_copy_text(pkg->git_url, sizeof(pkg->git_url), known_url);
        if (raw_version[0] == '\0') {
            zt_copy_text(pkg->git_rev, sizeof(pkg->git_rev), "main");
        } else if (zt_zpm_resolve_remote_version(ctx, known_url, raw_version, resolved_version, sizeof(resolved_version))) {
            zt_copy_text(pkg->git_rev, sizeof(pkg->git_rev), resolved_version);
        } else {
            zt_copy_text(pkg->git_rev, sizeof(pkg->git_rev), raw_version);
        }
        return;
    }

    pkg->source = ZT_LOCK_SOURCE_VERSION;
    zt_copy_text(pkg->version, sizeof(pkg->version), raw_version);
}

static int zt_zpm_extract_inline_string(const char *spec, const char *key, char *out, size_t out_cap) {
    const char *p;
    const char *eq;
    const char *start;
    const char *end;
    size_t copy_len;

    if (spec == NULL || key == NULL || out == NULL || out_cap == 0) return 0;
    out[0] = '\0';

    p = strstr(spec, key);
    if (p == NULL) return 0;
    eq = strchr(p, '=');
    if (eq == NULL) return 0;
    start = strchr(eq, '"');
    if (start == NULL) return 0;
    start += 1;
    end = strchr(start, '"');
    if (end == NULL) return 0;

    copy_len = (size_t)(end - start);
    if (copy_len >= out_cap) copy_len = out_cap - 1;
    memcpy(out, start, copy_len);
    out[copy_len] = '\0';
    return 1;
}

static void zt_zpm_lock_inline_table(zt_lock_package *pkg, const char *spec) {
    char value[256];

    if (pkg == NULL || spec == NULL) return;

    if (zt_zpm_extract_inline_string(spec, "path", value, sizeof(value))) {
        pkg->source = ZT_LOCK_SOURCE_PATH;
        zt_copy_text(pkg->path, sizeof(pkg->path), value);
        return;
    }

    if (zt_zpm_extract_inline_string(spec, "git", value, sizeof(value))) {
        pkg->source = ZT_LOCK_SOURCE_GIT;
        zt_copy_text(pkg->git_url, sizeof(pkg->git_url), value);
        if (zt_zpm_extract_inline_string(spec, "rev", value, sizeof(value))) {
            zt_copy_text(pkg->git_rev, sizeof(pkg->git_rev), value);
        } else if (zt_zpm_extract_inline_string(spec, "tag", value, sizeof(value))) {
            zt_copy_text(pkg->git_rev, sizeof(pkg->git_rev), value);
            zt_copy_text(pkg->git_tag, sizeof(pkg->git_tag), value);
        } else if (zt_zpm_extract_inline_string(spec, "branch", value, sizeof(value))) {
            zt_copy_text(pkg->git_rev, sizeof(pkg->git_rev), value);
            zt_copy_text(pkg->git_branch, sizeof(pkg->git_branch), value);
        } else {
            zt_copy_text(pkg->git_rev, sizeof(pkg->git_rev), "main");
        }
        return;
    }

    pkg->source = ZT_LOCK_SOURCE_VERSION;
    zt_copy_text(pkg->version, sizeof(pkg->version), spec);
}

static void zt_zpm_lock_dependency(
        zt_driver_context *ctx,
        zt_lock_package *pkg,
        const zt_project_dependency_entry *dep) {
    const char *known_url;

    if (pkg == NULL || dep == NULL) return;
    memset(pkg, 0, sizeof(*pkg));
    zt_copy_text(pkg->name, sizeof(pkg->name), dep->name);

    if (dep->spec[0] == '{') {
        zt_zpm_lock_inline_table(pkg, dep->spec);
        return;
    }

    known_url = zt_zpm_get_package_url(dep->name);
    zt_zpm_lock_version_or_git(ctx, pkg, known_url, dep->spec);
}

static int zt_zpm_lock_package_matches_spec(const zt_lock_package *pkg, const zt_project_dependency_entry *dep) {
    char value[256];
    char raw_version[64];

    if (pkg == NULL || dep == NULL) return 0;
    if (strcmp(pkg->name, dep->name) != 0) return 0;

    if (dep->spec[0] == '{') {
        if (zt_zpm_extract_inline_string(dep->spec, "path", value, sizeof(value))) {
            return pkg->source == ZT_LOCK_SOURCE_PATH && strcmp(pkg->path, value) == 0;
        }
        if (zt_zpm_extract_inline_string(dep->spec, "git", value, sizeof(value))) {
            if (pkg->source != ZT_LOCK_SOURCE_GIT || strcmp(pkg->git_url, value) != 0) return 0;
            if (zt_zpm_extract_inline_string(dep->spec, "rev", value, sizeof(value))) {
                return strcmp(pkg->git_rev, value) == 0;
            }
            if (zt_zpm_extract_inline_string(dep->spec, "tag", value, sizeof(value))) {
                return strcmp(pkg->git_rev, value) == 0 || strcmp(pkg->git_tag, value) == 0;
            }
            if (zt_zpm_extract_inline_string(dep->spec, "branch", value, sizeof(value))) {
                return strcmp(pkg->git_rev, value) == 0 || strcmp(pkg->git_branch, value) == 0;
            }
            return pkg->git_rev[0] != '\0';
        }
        return 0;
    }

    zt_zpm_unquote_spec_value(dep->spec, raw_version, sizeof(raw_version));
    if (pkg->source == ZT_LOCK_SOURCE_VERSION) return strcmp(pkg->version, raw_version) == 0;
    if (pkg->source == ZT_LOCK_SOURCE_GIT) return pkg->git_rev[0] != '\0';
    return 0;
}

static int zt_zpm_add_locked_or_resolved_dependency(
        zt_driver_context *ctx,
        zt_lockfile *lock,
        const zt_lockfile *existing_lock,
        const zt_project_dependency_entry *dep,
        int locked_mode) {
    const zt_lock_package *existing_pkg;
    zt_lock_package *pkg;
    char raw_version[64];

    if (lock == NULL || dep == NULL) return 0;

    if (zt_lockfile_find_package(lock, dep->name) != NULL) return 1;

    if (dep->spec[0] != '{') {
        zt_zpm_unquote_spec_value(dep->spec, raw_version, sizeof(raw_version));
        if (!zt_zpm_validate_semver_constraint(raw_version)) {
            zt_zpm_print_invalid_semver(dep->name, raw_version);
            return 0;
        }
    }

    existing_pkg = zt_lockfile_find_package(existing_lock, dep->name);
    if (existing_pkg != NULL && zt_zpm_lock_package_matches_spec(existing_pkg, dep)) {
        if (lock->package_count >= ZT_LOCKFILE_MAX_PACKAGES) return 0;
        lock->packages[lock->package_count++] = *existing_pkg;
        return 1;
    }

    if (locked_mode) {
        if (existing_pkg == NULL) {
            fprintf(stderr, "error: zenith.lock is missing dependency %s\n", dep->name);
        } else {
            fprintf(stderr, "error: zenith.lock is out of date for dependency %s\n", dep->name);
        }
        fprintf(stderr, "help: run zpm install to refresh zenith.lock\n");
        return 0;
    }

    if (lock->package_count >= ZT_LOCKFILE_MAX_PACKAGES) return 0;
    pkg = &lock->packages[lock->package_count++];
    zt_zpm_lock_dependency(ctx, pkg, dep);
    return 1;
}

int zt_handle_zpm(zt_driver_context *ctx, int argc, char **argv) {
    if (argc < 1) {
        zt_zpm_print_help(stdout);
        return 0;
    }

    const char *command = argv[0];

    if (strcmp(command, "help") == 0 || strcmp(command, "--help") == 0 || strcmp(command, "-h") == 0) {
        zt_zpm_print_help(stdout);
        return 0;
    }

    if (strcmp(command, "init") == 0) {
        const char *path = argc > 1 ? argv[1] : ".";
        return zt_handle_zpm_init(ctx, path);
    }

    if (strcmp(command, "add") == 0) {
        if (argc < 2) {
            fprintf(stderr, "error: zpm add requires a package name (e.g., zpm add json or zpm add json@1.0.0)\n");
            return 1;
        }
        return zt_handle_zpm_add(ctx, argv[1]);
    }

    if (strcmp(command, "remove") == 0) {
        if (argc < 2) {
            fprintf(stderr, "error: zpm remove requires a package name\n");
            return 1;
        }
        return zt_handle_zpm_remove(ctx, argv[1]);
    }

    if (strcmp(command, "install") == 0) {
        const char *path = ".";
        int locked_mode = 0;
        int i;
        for (i = 1; i < argc; i += 1) {
            if (strcmp(argv[i], "--locked") == 0) {
                locked_mode = 1;
            } else {
                path = argv[i];
            }
        }
        return zt_handle_zpm_install(ctx, path, locked_mode);
    }

    if (strcmp(command, "update") == 0) {
        const char *pkg = argc > 1 ? argv[1] : NULL;
        return zt_handle_zpm_update(ctx, pkg);
    }

    if (strcmp(command, "list") == 0) {
        const char *path = argc > 1 ? argv[1] : ".";
        return zt_handle_zpm_list(ctx, path);
    }

    if (strcmp(command, "find") == 0) {
        int all = 0;
        const char *query = NULL;
        int i;
        for (i = 1; i < argc; i += 1) {
            if (strcmp(argv[i], "--all") == 0) all = 1;
            else if (query == NULL) query = argv[i];
        }
        if (query == NULL) {
            fprintf(stderr, "error: zpm find requires a search query\n");
            return 1;
        }
        return zt_handle_zpm_find(ctx, query, all);
    }

    if (strcmp(command, "run") == 0) {
        if (argc < 2) {
            fprintf(stderr, "error: zpm run requires a script name\n");
            return 1;
        }
        return zt_handle_zpm_run(ctx, argv[1]);
    }

    if (strcmp(command, "update-registry") == 0) {
        return zt_zpm_sync_registry(ctx) ? 0 : 1;
    }

    if (strcmp(command, "publish") == 0) {
        const char *path = argc > 1 ? argv[1] : ".";
        return zt_handle_zpm_publish(ctx, path);
    }

    fprintf(stderr, "error: unknown zpm command: %s\n", command);
    zt_zpm_print_help(stderr);
    return 1;
}

int zt_handle_zpm_init(zt_driver_context *ctx, const char *path) {
    char manifest_path[512];
    char project_name[128];
    char manifest_content[1024];

    if (!zt_join_path(manifest_path, sizeof(manifest_path), path, "zenith.ztproj")) {
        zt_print_single_diag(ctx, "zpm", ZT_DIAG_PROJECT_PATH_TOO_LONG, zt_source_span_unknown(), "init: manifest path too long");
        return 1;
    }

    if (zt_path_is_file(manifest_path)) {
        fprintf(stderr, "error: %s already exists\n", manifest_path);
        return 1;
    }

    /* Extract project name from directory name */
    {
        const char *last_slash = strrchr(path, '/');
        const char *last_backslash = strrchr(path, '\\');
        const char *name = path;
        if (last_slash && last_slash > name) name = last_slash + 1;
        if (last_backslash && last_backslash > name) name = last_backslash + 1;
        
        if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0 || name[0] == '\0') {
            char cwd[512];
            if (zt_get_current_dir(cwd, sizeof(cwd))) {
                const char *cwd_name = strrchr(cwd, '/');
                if (!cwd_name) cwd_name = strrchr(cwd, '\\');
                name = cwd_name ? cwd_name + 1 : "my_project";
            } else {
                name = "my_project";
            }
        }
        zt_copy_text(project_name, sizeof(project_name), name);
    }

    snprintf(manifest_content, sizeof(manifest_content),
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
        "[dependencies]\n"
        "\n"
        "[dev_dependencies]\n",
        project_name);

    if (!zt_make_dirs(path)) {
        zt_print_single_diag(ctx, "zpm", ZT_DIAG_PROJECT_INVALID_INPUT, zt_source_span_unknown(), "init: failed to create directory %s", path);
        return 1;
    }

    if (!zt_write_file(manifest_path, manifest_content)) {
        zt_print_single_diag(ctx, "zpm", ZT_DIAG_PROJECT_INVALID_INPUT, zt_source_span_unknown(), "init: failed to write %s", manifest_path);
        return 1;
    }

    printf("initialized zenith project in %s\n", manifest_path);
    return 0;
}

int zt_handle_zpm_add(zt_driver_context *ctx, const char *pkg_spec) {
    zt_project_parse_result project;
    char project_root[ZT_ZPM_BASE_PATH_CAP];
    char manifest_path[ZT_ZPM_BASE_PATH_CAP];
    char pkg_name[64];
    char pkg_version[64];
    const char *at;
    size_t i;
    if (!zt_zpm_resolve_project(ctx, NULL, project_root, sizeof(project_root), manifest_path, sizeof(manifest_path))) {
        return 1;
    }

    if (!zt_project_load_file(manifest_path, &project)) {
        zt_print_project_parse_error(ctx, manifest_path, &project);
        return 1;
    }

    /* Parse pkg_spec: name[@version] or URL */
    at = strchr(pkg_spec, '@');
    const char *url = at == NULL ? zt_zpm_get_package_url(pkg_spec) : NULL;

    /* If not found and not a URL, try syncing registry once */
    if (at == NULL && !url && !strstr(pkg_spec, "://") && !strstr(pkg_spec, "git@")) {
        char home[ZT_ZPM_BASE_PATH_CAP];
        char cache_file[ZT_ZPM_PATH_CAP];
        if (!zt_native_get_home_dir(home, sizeof(home))) zt_copy_text(home, sizeof(home), ".");
        snprintf(cache_file, sizeof(cache_file), "%s/.zenith/registry.ztproj", home);
        
        if (!zt_path_is_file(cache_file)) {
            zt_zpm_sync_registry(ctx);
            url = zt_zpm_get_package_url(pkg_spec);
        }
    }
    
    if (url) {
        /* It's a Git dependency */
        if (project.manifest.dependency_count >= ZT_PROJECT_MAX_DEPENDENCIES) {
            fprintf(stderr, "error: too many dependencies\n");
            return 1;
        }
        
        /* For URLs, extract a possible name from the last part of the path */
        const char *last_slash = strrchr(url, '/');
        const char *name = last_slash ? last_slash + 1 : url;
        char clean_name[64];
        zt_copy_text(clean_name, sizeof(clean_name), name);
        char *dot_git = strstr(clean_name, ".git");
        if (dot_git) *dot_git = '\0';
        zt_copy_text(pkg_name, sizeof(pkg_name), clean_name);
        zt_copy_text(pkg_version, sizeof(pkg_version), "git:main");
        
        zt_copy_text(project.manifest.dependencies[project.manifest.dependency_count].name, 64, clean_name);
        snprintf(project.manifest.dependencies[project.manifest.dependency_count].spec, 256, "{ git = \"%s\", rev = \"main\" }", url);
        project.manifest.dependency_count += 1;
        printf("added git dependency %s = %s\n", clean_name, url);
    } else {
        /* Treat as name[@version] */
        if (at) {
            size_t name_len = (size_t)(at - pkg_spec);
            if (name_len >= sizeof(pkg_name)) name_len = sizeof(pkg_name) - 1;
            memcpy(pkg_name, pkg_spec, name_len);
            pkg_name[name_len] = '\0';
            zt_copy_text(pkg_version, sizeof(pkg_version), at + 1);
        } else {
            zt_copy_text(pkg_name, sizeof(pkg_name), pkg_spec);
            zt_copy_text(pkg_version, sizeof(pkg_version), "^0.1.0"); 
        }

        if (!zt_zpm_validate_semver_constraint(pkg_version)) {
            zt_zpm_print_invalid_semver(pkg_name, pkg_version);
            return 1;
        }

        /* Check if already exists */
        for (i = 0; i < project.manifest.dependency_count; i += 1) {
            if (strcmp(project.manifest.dependencies[i].name, pkg_name) == 0) {
                fprintf(stderr, "warning: dependency %s already exists, updating version\n", pkg_name);
                snprintf(project.manifest.dependencies[i].spec, sizeof(project.manifest.dependencies[i].spec), "\"%s\"", pkg_version);
                goto write_back;
            }
        }

        if (project.manifest.dependency_count >= ZT_PROJECT_MAX_DEPENDENCIES) {
            fprintf(stderr, "error: too many dependencies in manifest\n");
            return 1;
        }

        zt_copy_text(project.manifest.dependencies[project.manifest.dependency_count].name, 64, pkg_name);
        snprintf(project.manifest.dependencies[project.manifest.dependency_count].spec, 256, "\"%s\"", pkg_version);
        project.manifest.dependency_count += 1;
        printf("added dependency %s = \"%s\"\n", pkg_name, pkg_version);
    }

write_back:
    {
        char buffer[16384];
        if (!zt_project_manifest_serialize(&project.manifest, buffer, sizeof(buffer))) {
            fprintf(stderr, "error: failed to serialize manifest\n");
            return 1;
        }

        if (!zt_write_file(manifest_path, buffer)) {
            fprintf(stderr, "error: failed to write %s\n", manifest_path);
            return 1;
        }
    }

    printf("added dependency %s = \"%s\" to %s\n", pkg_name, pkg_version, manifest_path);
    return 0;
}

int zt_handle_zpm_install(zt_driver_context *ctx, const char *project_path, int locked_mode) {
    zt_project_parse_result project;
    char project_root[ZT_ZPM_BASE_PATH_CAP];
    char manifest_path[ZT_ZPM_BASE_PATH_CAP];
    char lock_path[ZT_ZPM_PATH_CAP];
    zt_lockfile lock;
    zt_lockfile existing_lock;
    int has_existing_lock = 0;
    char lock_error[256];
    size_t i;

    if (!zt_zpm_resolve_project(ctx, project_path, project_root, sizeof(project_root), manifest_path, sizeof(manifest_path))) {
        return 1;
    }

    if (!zt_project_load_file(manifest_path, &project)) {
        zt_print_project_parse_error(ctx, manifest_path, &project);
        return 1;
    }

    if (!zt_join_path(lock_path, sizeof(lock_path), project_root, "zenith.lock")) {
        return 1;
    }

    zt_lockfile_init(&lock);
    zt_lockfile_init(&existing_lock);

    if (zt_path_is_file(lock_path)) {
        if (!zt_lockfile_load(lock_path, &existing_lock, lock_error, sizeof(lock_error))) {
            fprintf(stderr, "error: invalid %s: %s\n", lock_path, lock_error);
            return 1;
        }
        has_existing_lock = 1;
    } else if (locked_mode) {
        fprintf(stderr, "error: --locked requires %s\n", lock_path);
        fprintf(stderr, "help: run zpm install to create zenith.lock\n");
        return 1;
    }

    /* Direct dependencies are resolved from zenith.lock first. */
    for (i = 0; i < project.manifest.dependency_count; i += 1) {
        if (!zt_zpm_add_locked_or_resolved_dependency(
                    ctx,
                    &lock,
                    has_existing_lock ? &existing_lock : NULL,
                    &project.manifest.dependencies[i],
                    locked_mode)) {
            return 1;
        }
    }

    for (i = 0; i < project.manifest.dev_dependency_count; i += 1) {
        if (!zt_zpm_add_locked_or_resolved_dependency(
                    ctx,
                    &lock,
                    has_existing_lock ? &existing_lock : NULL,
                    &project.manifest.dev_dependencies[i],
                    locked_mode)) {
            return 1;
        }
    }

    if (locked_mode) {
        if (!ctx->quiet) printf("zenith.lock is up to date (%zu dependencies)\n", lock.package_count);
        return 0;
    }

    /* 
     * Transitive resolution loop 
     * We start with the project's own dependencies in the lockfile, 
     * and for each Git dependency, we check if it has its own zenith.ztproj 
     * and add its dependencies to the lockfile if not already there.
     */
    for (i = 0; i < lock.package_count; i += 1) {
        zt_lock_package *pkg = &lock.packages[i];
        
        if (pkg->source == ZT_LOCK_SOURCE_GIT) {
            char pkg_dir[ZT_ZPM_PATH_CAP];
            char git_dir[ZT_ZPM_PATH_CAP];
            char pkg_manifest[ZT_ZPM_LONG_PATH_CAP];
            snprintf(pkg_dir, sizeof(pkg_dir), "%s/.zenith/packages/%s", project_root, pkg->name);
            snprintf(git_dir, sizeof(git_dir), "%s/.zenith/packages/%s/.git", project_root, pkg->name);
            snprintf(pkg_manifest, sizeof(pkg_manifest), "%s/zenith.ztproj", pkg_dir);

            if (zt_path_is_dir(git_dir)) {
                if (ctx->verbose) printf("updating %s...\n", pkg->name);
                const char *fetch_argv[] = { "git", "-C", pkg_dir, "fetch", "--depth", "1", "origin", pkg->git_rev, NULL };
                int spawn_failed = 0;
                zt_native_spawn_process(fetch_argv, NULL, &spawn_failed);
                const char *checkout_argv[] = { "git", "-C", pkg_dir, "checkout", "FETCH_HEAD", NULL };
                zt_native_spawn_process(checkout_argv, NULL, &spawn_failed);
            } else {
                if (!ctx->quiet) printf("downloading %s from %s...\n", pkg->name, pkg->git_url);
                zt_make_dirs(pkg_dir);
                const char *clone_argv[] = { "git", "clone", "--depth", "1", pkg->git_url, pkg_dir, NULL };
                int spawn_failed = 0;
                if (zt_native_spawn_process(clone_argv, NULL, &spawn_failed) != 0) {
                    fprintf(stderr, "error: failed to download package %s\n", pkg->name);
                    return 1;
                }
            }

            /* Now load the package manifest to find transitive dependencies */
            zt_project_parse_result sub_project;
            if (zt_path_is_file(pkg_manifest) && zt_project_load_file(pkg_manifest, &sub_project)) {
                size_t j;
                for (j = 0; j < sub_project.manifest.dependency_count; j += 1) {
                    zt_project_dependency_entry *dep = &sub_project.manifest.dependencies[j];
                    /* Check if already in lockfile */
                    size_t k;
                    int kfound = 0;
                    for (k = 0; k < lock.package_count; k += 1) {
                        if (strcmp(lock.packages[k].name, dep->name) == 0) {
                            kfound = 1;
                            break;
                        }
                    }

                    if (!kfound && lock.package_count < ZT_LOCKFILE_MAX_PACKAGES) {
                        zt_lock_package *new_pkg = &lock.packages[lock.package_count++];
                        zt_zpm_lock_dependency(ctx, new_pkg, dep);
                    }
                }
            }
        }
    }

    if (!zt_lockfile_save(&lock, lock_path)) {
        fprintf(stderr, "error: failed to write %s\n", lock_path);
        return 1;
    }

    if (!ctx->quiet) printf("resolved %zu dependencies; locked to %s\n", lock.package_count, lock_path);
    return 0;
}

int zt_handle_zpm_remove(zt_driver_context *ctx, const char *pkg_name) {
    zt_project_parse_result project;
    char project_root[512];
    char manifest_path[512];
    size_t i;
    int found = 0;

    if (!zt_zpm_resolve_project(ctx, NULL, project_root, sizeof(project_root), manifest_path, sizeof(manifest_path))) {
        return 1;
    }

    if (!zt_project_load_file(manifest_path, &project)) {
        zt_print_project_parse_error(ctx, manifest_path, &project);
        return 1;
    }

    for (i = 0; i < project.manifest.dependency_count; i += 1) {
        if (strcmp(project.manifest.dependencies[i].name, pkg_name) == 0) {
            /* Shift remaining dependencies */
            if (i + 1 < project.manifest.dependency_count) {
                memmove(&project.manifest.dependencies[i], &project.manifest.dependencies[i+1], (project.manifest.dependency_count - i - 1) * sizeof(zt_project_dependency_entry));
            }
            project.manifest.dependency_count -= 1;
            found = 1;
            break;
        }
    }

    if (!found) {
        fprintf(stderr, "error: package %s not found in manifest\n", pkg_name);
        return 1;
    }

    /* Serialize and write back */
    {
        char buffer[16384];
        if (!zt_project_manifest_serialize(&project.manifest, buffer, sizeof(buffer))) {
            fprintf(stderr, "error: failed to serialize manifest\n");
            return 1;
        }
        if (!zt_write_file(manifest_path, buffer)) {
            fprintf(stderr, "error: failed to write %s\n", manifest_path);
            return 1;
        }
    }

    printf("removed dependency %s from %s\n", pkg_name, manifest_path);
    return 0;
}

int zt_handle_zpm_update(zt_driver_context *ctx, const char *pkg_name) {
    if (pkg_name) {
        printf("zpm: updating package %s...\n", pkg_name);
    } else {
        printf("zpm: updating all packages...\n");
    }
    /* For MVP, update is just a re-install */
    return zt_handle_zpm_install(ctx, ".", 0);
}

int zt_handle_zpm_list(zt_driver_context *ctx, const char *project_path) {
    zt_project_parse_result project;
    char project_root[512];
    char manifest_path[512];
    size_t i;

    if (!zt_zpm_resolve_project(ctx, project_path, project_root, sizeof(project_root), manifest_path, sizeof(manifest_path))) {
        return 1;
    }

    if (!zt_project_load_file(manifest_path, &project)) {
        zt_print_project_parse_error(ctx, manifest_path, &project);
        return 1;
    }

    printf("dependencies for %s:\n", project.manifest.project_name);
    if (project.manifest.dependency_count == 0) {
        printf("  (none)\n");
    } else {
        for (i = 0; i < project.manifest.dependency_count; i += 1) {
            printf("  - %s = %s\n", project.manifest.dependencies[i].name, project.manifest.dependencies[i].spec);
        }
    }

    if (project.manifest.dev_dependency_count > 0) {
        printf("\ndev_dependencies:\n");
        for (i = 0; i < project.manifest.dev_dependency_count; i += 1) {
            printf("  - %s = %s\n", project.manifest.dev_dependencies[i].name, project.manifest.dev_dependencies[i].spec);
        }
    }

    return 0;
}

int zt_handle_zpm_find(zt_driver_context *ctx, const char *query, int all) {
    (void)ctx;
    (void)all;
    printf("zpm: searching for '%s' in zenith-registry (simulated)...\n", query);
    
    /* Mock results */
    if (strstr("json", query)) {
        printf("  - json (0.1.0) - fast JSON parser and emitter\n");
    }
    if (strstr("borealis", query)) {
        printf("  - borealis (0.5.0) - 3D engine for Zenith\n");
    }
    if (strstr("http", query)) {
        printf("  - http (1.2.0) - high-level HTTP client/server\n");
    }
    
    printf("\ntip: use 'zpm add <pkg>' to install one of these.\n");
    return 0;
}

int zt_handle_zpm_publish(zt_driver_context *ctx, const char *project_path) {
    int status;
    
    printf("zpm: validating project before publication...\n");

    /* 1. Run type check (verify) */
    status = zt_handle_project_command(ctx, "verify", project_path, NULL, 0);
    if (status != 0) {
        fprintf(stderr, "error: project validation failed\n");
        return 1;
    }

    /* 2. Run doc check */
    status = zt_handle_doc_check(ctx, project_path);
    if (status != 0) {
        fprintf(stderr, "error: documentation validation failed\n");
        return 1;
    }

    printf("zpm: validation successful\n");
    printf("zpm: publishing to local registry (simulation)...\n");
    printf("zpm: package published successfully!\n");
    
    return 0;
}

int zt_handle_zpm_run(zt_driver_context *ctx, const char *script_name) {
    zt_project_parse_result project;
    char project_root[512];
    char manifest_path[512];
    size_t i;

    if (!zt_zpm_resolve_project(ctx, NULL, project_root, sizeof(project_root), manifest_path, sizeof(manifest_path))) {
        return 1;
    }

    if (!zt_project_load_file(manifest_path, &project)) {
        zt_print_project_parse_error(ctx, manifest_path, &project);
        return 1;
    }

    for (i = 0; i < project.manifest.script_count; i += 1) {
        if (strcmp(project.manifest.scripts[i].name, script_name) == 0) {
            if (!ctx->quiet) printf("running script '%s': %s\n", script_name, project.manifest.scripts[i].command);
            return system(project.manifest.scripts[i].command);
        }
    }

    fprintf(stderr, "error: script '%s' not found in [scripts] section of %s\n", script_name, manifest_path);
    return 1;
}
