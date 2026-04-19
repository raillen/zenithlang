#include "compiler/project/ztproj.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum zt_project_section {
    ZT_PROJECT_SECTION_NONE = 0,
    ZT_PROJECT_SECTION_PROJECT,
    ZT_PROJECT_SECTION_SOURCE,
    ZT_PROJECT_SECTION_APP,
    ZT_PROJECT_SECTION_LIB,
    ZT_PROJECT_SECTION_BUILD,
    ZT_PROJECT_SECTION_TEST,
    ZT_PROJECT_SECTION_ZDOC,
    ZT_PROJECT_SECTION_DEPENDENCIES,
    ZT_PROJECT_SECTION_DEV_DEPENDENCIES
} zt_project_section;

static const char *zt_project_safe_text(const char *text) {
    return text != NULL ? text : "";
}

static const char *zt_project_section_name(zt_project_section section) {
    switch (section) {
        case ZT_PROJECT_SECTION_PROJECT: return "project";
        case ZT_PROJECT_SECTION_SOURCE: return "source";
        case ZT_PROJECT_SECTION_APP: return "app";
        case ZT_PROJECT_SECTION_LIB: return "lib";
        case ZT_PROJECT_SECTION_BUILD: return "build";
        case ZT_PROJECT_SECTION_TEST: return "test";
        case ZT_PROJECT_SECTION_ZDOC: return "zdoc";
        case ZT_PROJECT_SECTION_DEPENDENCIES: return "dependencies";
        case ZT_PROJECT_SECTION_DEV_DEPENDENCIES: return "dev_dependencies";
        default: return "<none>";
    }
}

static void zt_project_set_error(
        zt_project_parse_result *result,
        zt_project_error_code code,
        int line_number,
        const char *message) {
    if (result == NULL) return;
    result->ok = 0;
    result->code = code;
    result->line_number = line_number;
    snprintf(result->message, sizeof(result->message), "%s", zt_project_safe_text(message));
}

static void zt_project_set_unknown_key(
        zt_project_parse_result *result,
        zt_project_section section,
        const char *key,
        int line_number) {
    char message[192];
    snprintf(message, sizeof(message), "unknown key %s.%s", zt_project_section_name(section), zt_project_safe_text(key));
    zt_project_set_error(result, ZT_PROJECT_UNKNOWN_KEY, line_number, message);
}

static char *zt_project_read_file_text(const char *path, size_t *out_length) {
    FILE *file;
    long file_size;
    char *buffer;

    if (out_length != NULL) *out_length = 0;
    if (path == NULL) return NULL;

    file = fopen(path, "rb");
    if (file == NULL) return NULL;

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return NULL;
    }

    file_size = ftell(file);
    if (file_size < 0) {
        fclose(file);
        return NULL;
    }

    if (fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return NULL;
    }

    buffer = (char *)malloc((size_t)file_size + 1);
    if (buffer == NULL) {
        fclose(file);
        return NULL;
    }

    if (file_size > 0 && fread(buffer, 1, (size_t)file_size, file) != (size_t)file_size) {
        free(buffer);
        fclose(file);
        return NULL;
    }

    buffer[file_size] = '\0';
    fclose(file);

    if (out_length != NULL) *out_length = (size_t)file_size;
    return buffer;
}

static const char *zt_project_trim_left(const char *text) {
    while (*text == ' ' || *text == '\t' || *text == '\r' || *text == '\n') text += 1;
    return text;
}

static void zt_project_trim_inplace(char *text) {
    char *start;
    char *end;
    size_t length;

    if (text == NULL) return;

    start = (char *)zt_project_trim_left(text);
    if (start != text) {
        memmove(text, start, strlen(start) + 1);
    }

    length = strlen(text);
    end = text + length;
    while (end > text &&
            (end[-1] == ' ' || end[-1] == '\t' || end[-1] == '\r' || end[-1] == '\n')) {
        end -= 1;
    }
    *end = '\0';
}

static void zt_project_strip_comment(char *text) {
    size_t index = 0;
    int in_string = 0;

    if (text == NULL) return;

    while (text[index] != '\0') {
        if (text[index] == '"' && (index == 0 || text[index - 1] != '\\')) {
            in_string = !in_string;
        } else if (!in_string && text[index] == '#') {
            text[index] = '\0';
            return;
        }
        index += 1;
    }
}

static int zt_project_copy_checked(char *dest, size_t capacity, const char *value) {
    size_t length;

    if (dest == NULL || capacity == 0 || value == NULL) return 0;
    length = strlen(value);
    if (length + 1 > capacity) return 0;
    memcpy(dest, value, length + 1);
    return 1;
}

static int zt_project_parse_string_value(
        const char *value_text,
        char *dest,
        size_t capacity,
        zt_project_parse_result *result,
        int line_number) {
    const char *cursor;
    size_t out = 0;

    if (dest == NULL || capacity == 0) {
        zt_project_set_error(result, ZT_PROJECT_INVALID_INPUT, line_number, "destination string buffer is invalid");
        return 0;
    }

    dest[0] = '\0';
    cursor = zt_project_trim_left(value_text);
    if (*cursor != '"') {
        zt_project_set_error(result, ZT_PROJECT_INVALID_STRING, line_number, "expected string value");
        return 0;
    }
    cursor += 1;

    while (*cursor != '\0') {
        char ch = *cursor;

        if (ch == '"') {
            cursor += 1;
            cursor = zt_project_trim_left(cursor);
            if (*cursor != '\0') {
                zt_project_set_error(result, ZT_PROJECT_INVALID_STRING, line_number, "unexpected characters after string value");
                return 0;
            }
            dest[out] = '\0';
            return 1;
        }

        if (out + 1 >= capacity) {
            zt_project_set_error(result, ZT_PROJECT_PATH_TOO_LONG, line_number, "string value is too long");
            return 0;
        }

        if (ch == '\\' && cursor[1] != '\0') {
            cursor += 1;
            switch (*cursor) {
                case 'n': ch = '\n'; break;
                case 'r': ch = '\r'; break;
                case 't': ch = '\t'; break;
                case '"': ch = '"'; break;
                case '\\': ch = '\\'; break;
                default: ch = *cursor; break;
            }
        }

        dest[out++] = ch;
        cursor += 1;
    }

    zt_project_set_error(result, ZT_PROJECT_INVALID_STRING, line_number, "unterminated string value");
    return 0;
}

static zt_project_section zt_project_section_from_name(const char *name) {
    if (strcmp(name, "project") == 0) return ZT_PROJECT_SECTION_PROJECT;
    if (strcmp(name, "source") == 0) return ZT_PROJECT_SECTION_SOURCE;
    if (strcmp(name, "app") == 0) return ZT_PROJECT_SECTION_APP;
    if (strcmp(name, "lib") == 0) return ZT_PROJECT_SECTION_LIB;
    if (strcmp(name, "build") == 0) return ZT_PROJECT_SECTION_BUILD;
    if (strcmp(name, "test") == 0) return ZT_PROJECT_SECTION_TEST;
    if (strcmp(name, "zdoc") == 0) return ZT_PROJECT_SECTION_ZDOC;
    if (strcmp(name, "dependencies") == 0) return ZT_PROJECT_SECTION_DEPENDENCIES;
    if (strcmp(name, "dev_dependencies") == 0) return ZT_PROJECT_SECTION_DEV_DEPENDENCIES;
    return ZT_PROJECT_SECTION_NONE;
}

static int zt_project_parse_section(
        const char *line,
        zt_project_section *section,
        zt_project_parse_result *result,
        int line_number) {
    char section_name[64];
    char message[128];
    size_t length;

    if (line[0] != '[') return 0;

    length = strlen(line);
    if (length < 3 || line[length - 1] != ']') {
        zt_project_set_error(result, ZT_PROJECT_INVALID_SECTION, line_number, "invalid section header");
        return -1;
    }

    if (length - 2 >= sizeof(section_name)) {
        zt_project_set_error(result, ZT_PROJECT_INVALID_SECTION, line_number, "section name is too long");
        return -1;
    }

    memcpy(section_name, line + 1, length - 2);
    section_name[length - 2] = '\0';
    zt_project_trim_inplace(section_name);

    *section = zt_project_section_from_name(section_name);
    if (*section == ZT_PROJECT_SECTION_NONE) {
        snprintf(message, sizeof(message), "unknown section [%s]", section_name);
        zt_project_set_error(result, ZT_PROJECT_INVALID_SECTION, line_number, message);
        return -1;
    }

    return 1;
}

static int zt_project_find_assignment(const char *line) {
    size_t index = 0;
    int in_string = 0;
    int brace_depth = 0;

    while (line[index] != '\0') {
        if (line[index] == '"' && (index == 0 || line[index - 1] != '\\')) {
            in_string = !in_string;
        } else if (!in_string) {
            if (line[index] == '{') brace_depth += 1;
            else if (line[index] == '}' && brace_depth > 0) brace_depth -= 1;
            else if (line[index] == '=' && brace_depth == 0) return (int)index;
        }
        index += 1;
    }

    return -1;
}

static int zt_project_parse_dependency_spec(
        const char *value_text,
        char *dest,
        size_t capacity,
        zt_project_parse_result *result,
        int line_number) {
    char trimmed[256];

    if (value_text == NULL || dest == NULL || capacity == 0) {
        zt_project_set_error(result, ZT_PROJECT_INVALID_INPUT, line_number, "invalid dependency spec destination");
        return 0;
    }

    if (!zt_project_copy_checked(trimmed, sizeof(trimmed), value_text)) {
        zt_project_set_error(result, ZT_PROJECT_PATH_TOO_LONG, line_number, "dependency spec is too long");
        return 0;
    }

    zt_project_trim_inplace(trimmed);

    if (trimmed[0] == '"') {
        if (!zt_project_parse_string_value(trimmed, dest, capacity, result, line_number)) {
            return 0;
        }
        return 1;
    }

    if (trimmed[0] == '{') {
        size_t length = strlen(trimmed);
        if (length < 2 || trimmed[length - 1] != '}') {
            zt_project_set_error(result, ZT_PROJECT_INVALID_ASSIGNMENT, line_number, "dependency table must end with '}'");
            return 0;
        }

        if (!zt_project_copy_checked(dest, capacity, trimmed)) {
            zt_project_set_error(result, ZT_PROJECT_PATH_TOO_LONG, line_number, "dependency table is too long");
            return 0;
        }

        return 1;
    }

    zt_project_set_error(result, ZT_PROJECT_INVALID_ASSIGNMENT, line_number, "dependency spec must be a string or inline table");
    return 0;
}

static int zt_project_assign_dependency(
        zt_project_manifest *manifest,
        zt_project_section section,
        const char *key,
        const char *value,
        zt_project_parse_result *result,
        int line_number) {
    zt_project_dependency_entry *entry;
    size_t index;
    size_t *count;
    zt_project_dependency_entry *entries;

    if (manifest == NULL || key == NULL || key[0] == '\0') {
        zt_project_set_error(result, ZT_PROJECT_INVALID_ASSIGNMENT, line_number, "dependency name cannot be empty");
        return 0;
    }

    if (section == ZT_PROJECT_SECTION_DEPENDENCIES) {
        count = &manifest->dependency_count;
        entries = manifest->dependencies;
    } else {
        count = &manifest->dev_dependency_count;
        entries = manifest->dev_dependencies;
    }

    for (index = 0; index < *count; index += 1) {
        if (strcmp(entries[index].name, key) == 0) {
            entry = &entries[index];
            return zt_project_parse_dependency_spec(value, entry->spec, sizeof(entry->spec), result, line_number);
        }
    }

    if (*count >= ZT_PROJECT_MAX_DEPENDENCIES) {
        zt_project_set_error(result, ZT_PROJECT_TOO_MANY_DEPENDENCIES, line_number, "too many dependencies in manifest");
        return 0;
    }

    entry = &entries[*count];
    memset(entry, 0, sizeof(*entry));

    if (!zt_project_copy_checked(entry->name, sizeof(entry->name), key)) {
        zt_project_set_error(result, ZT_PROJECT_PATH_TOO_LONG, line_number, "dependency name is too long");
        return 0;
    }

    if (!zt_project_parse_dependency_spec(value, entry->spec, sizeof(entry->spec), result, line_number)) {
        return 0;
    }

    *count += 1;
    return 1;
}

static int zt_project_assign_value(
        zt_project_manifest *manifest,
        zt_project_section section,
        const char *key,
        const char *value,
        zt_project_parse_result *result,
        int line_number) {
    if (section == ZT_PROJECT_SECTION_NONE) {
        zt_project_set_error(result, ZT_PROJECT_INVALID_ASSIGNMENT, line_number, "assignment outside a section");
        return 0;
    }

    if (section == ZT_PROJECT_SECTION_DEPENDENCIES ||
            section == ZT_PROJECT_SECTION_DEV_DEPENDENCIES) {
        return zt_project_assign_dependency(manifest, section, key, value, result, line_number);
    }

    if (section == ZT_PROJECT_SECTION_PROJECT) {
        if (strcmp(key, "name") == 0) return zt_project_parse_string_value(value, manifest->project_name, sizeof(manifest->project_name), result, line_number);
        if (strcmp(key, "kind") == 0) return zt_project_parse_string_value(value, manifest->project_kind, sizeof(manifest->project_kind), result, line_number);
        if (strcmp(key, "version") == 0) return zt_project_parse_string_value(value, manifest->version, sizeof(manifest->version), result, line_number);
        zt_project_set_unknown_key(result, section, key, line_number);
        return 0;
    }

    if (section == ZT_PROJECT_SECTION_SOURCE) {
        if (strcmp(key, "root") == 0) return zt_project_parse_string_value(value, manifest->source_root, sizeof(manifest->source_root), result, line_number);
        zt_project_set_unknown_key(result, section, key, line_number);
        return 0;
    }

    if (section == ZT_PROJECT_SECTION_APP) {
        if (strcmp(key, "entry") == 0) return zt_project_parse_string_value(value, manifest->app_entry, sizeof(manifest->app_entry), result, line_number);
        zt_project_set_unknown_key(result, section, key, line_number);
        return 0;
    }

    if (section == ZT_PROJECT_SECTION_LIB) {
        if (strcmp(key, "root_namespace") == 0) return zt_project_parse_string_value(value, manifest->lib_root_namespace, sizeof(manifest->lib_root_namespace), result, line_number);
        zt_project_set_unknown_key(result, section, key, line_number);
        return 0;
    }

    if (section == ZT_PROJECT_SECTION_BUILD) {
        if (strcmp(key, "target") == 0) return zt_project_parse_string_value(value, manifest->build_target, sizeof(manifest->build_target), result, line_number);
        if (strcmp(key, "output") == 0) return zt_project_parse_string_value(value, manifest->build_output, sizeof(manifest->build_output), result, line_number);
        if (strcmp(key, "profile") == 0) return zt_project_parse_string_value(value, manifest->build_profile, sizeof(manifest->build_profile), result, line_number);
        zt_project_set_unknown_key(result, section, key, line_number);
        return 0;
    }

    if (section == ZT_PROJECT_SECTION_TEST) {
        if (strcmp(key, "root") == 0) return zt_project_parse_string_value(value, manifest->test_root, sizeof(manifest->test_root), result, line_number);
        zt_project_set_unknown_key(result, section, key, line_number);
        return 0;
    }

    if (section == ZT_PROJECT_SECTION_ZDOC) {
        if (strcmp(key, "root") == 0) return zt_project_parse_string_value(value, manifest->zdoc_root, sizeof(manifest->zdoc_root), result, line_number);
        zt_project_set_unknown_key(result, section, key, line_number);
        return 0;
    }

    zt_project_set_error(result, ZT_PROJECT_INVALID_ASSIGNMENT, line_number, "assignment outside a known section");
    return 0;
}

static int zt_project_require_field(
        zt_project_parse_result *result,
        const char *value,
        const char *field_name) {
    char message[192];

    if (value != NULL && value[0] != '\0') {
        return 1;
    }

    snprintf(message, sizeof(message), "missing required field '%s'", field_name);
    zt_project_set_error(result, ZT_PROJECT_MISSING_FIELD, 0, message);
    return 0;
}

static int zt_project_copy_default(
        zt_project_parse_result *result,
        char *dest,
        size_t capacity,
        const char *value,
        const char *field_name) {
    char message[192];

    if (zt_project_copy_checked(dest, capacity, value)) {
        return 1;
    }

    snprintf(message, sizeof(message), "%s default is too long", field_name);
    zt_project_set_error(result, ZT_PROJECT_PATH_TOO_LONG, 0, message);
    return 0;
}

zt_project_kind zt_project_manifest_kind(const zt_project_manifest *manifest) {
    if (manifest == NULL) return ZT_PROJECT_KIND_UNKNOWN;
    if (strcmp(manifest->project_kind, "app") == 0) return ZT_PROJECT_KIND_APP;
    if (strcmp(manifest->project_kind, "lib") == 0) return ZT_PROJECT_KIND_LIB;
    return ZT_PROJECT_KIND_UNKNOWN;
}

const char *zt_project_manifest_entry_namespace(const zt_project_manifest *manifest) {
    if (manifest == NULL) return "";
    if (zt_project_manifest_kind(manifest) == ZT_PROJECT_KIND_APP) return manifest->app_entry;
    if (zt_project_manifest_kind(manifest) == ZT_PROJECT_KIND_LIB) return manifest->lib_root_namespace;
    return "";
}

static int zt_project_validate(zt_project_parse_result *result) {
    zt_project_manifest *manifest;
    zt_project_kind kind;
    char message[192];

    if (result == NULL) return 0;
    manifest = &result->manifest;

    if (!zt_project_require_field(result, manifest->project_name, "project.name")) return 0;
    if (!zt_project_require_field(result, manifest->project_kind, "project.kind")) return 0;
    if (!zt_project_require_field(result, manifest->version, "project.version")) return 0;
    if (!zt_project_require_field(result, manifest->source_root, "source.root")) return 0;

    kind = zt_project_manifest_kind(manifest);
    if (kind == ZT_PROJECT_KIND_UNKNOWN) {
        snprintf(message, sizeof(message), "unknown project kind '%s'; expected app or lib", manifest->project_kind);
        zt_project_set_error(result, ZT_PROJECT_INVALID_KIND, 0, message);
        return 0;
    }

    if (kind == ZT_PROJECT_KIND_APP) {
        if (!zt_project_require_field(result, manifest->app_entry, "app.entry")) return 0;
        if (!zt_project_copy_default(result, manifest->entry, sizeof(manifest->entry), manifest->app_entry, "app.entry")) return 0;
    } else if (kind == ZT_PROJECT_KIND_LIB) {
        if (!zt_project_require_field(result, manifest->lib_root_namespace, "lib.root_namespace")) return 0;
        if (!zt_project_copy_default(result, manifest->entry, sizeof(manifest->entry), manifest->lib_root_namespace, "lib.root_namespace")) return 0;
    }

    if (manifest->build_target[0] == '\0') {
        if (!zt_project_copy_default(result, manifest->build_target, sizeof(manifest->build_target), "native", "build.target")) return 0;
    }
    if (strcmp(manifest->build_target, "native") != 0) {
        snprintf(message, sizeof(message), "unknown build target '%s'; expected native", manifest->build_target);
        zt_project_set_error(result, ZT_PROJECT_INVALID_TARGET, 0, message);
        return 0;
    }

    if (manifest->build_output[0] == '\0') {
        if (!zt_project_copy_default(result, manifest->build_output, sizeof(manifest->build_output), "build", "build.output")) return 0;
    }
    if (!zt_project_copy_default(result, manifest->output_dir, sizeof(manifest->output_dir), manifest->build_output, "build.output")) return 0;

    if (manifest->build_profile[0] == '\0') {
        if (!zt_project_copy_default(result, manifest->build_profile, sizeof(manifest->build_profile), "debug", "build.profile")) return 0;
    }
    if (strcmp(manifest->build_profile, "debug") != 0 && strcmp(manifest->build_profile, "release") != 0) {
        snprintf(message, sizeof(message), "unknown build profile '%s'; expected debug or release", manifest->build_profile);
        zt_project_set_error(result, ZT_PROJECT_INVALID_PROFILE, 0, message);
        return 0;
    }

    if (manifest->test_root[0] == '\0') {
        if (!zt_project_copy_default(result, manifest->test_root, sizeof(manifest->test_root), "tests", "test.root")) return 0;
    }

    if (manifest->zdoc_root[0] == '\0') {
        if (!zt_project_copy_default(result, manifest->zdoc_root, sizeof(manifest->zdoc_root), "zdoc", "zdoc.root")) return 0;
    }

    if (!zt_project_copy_default(result, manifest->output_name, sizeof(manifest->output_name), manifest->project_name, "project.name")) return 0;

    result->ok = 1;
    result->code = ZT_PROJECT_OK;
    result->message[0] = '\0';
    return 1;
}

void zt_project_manifest_init(zt_project_manifest *manifest) {
    if (manifest == NULL) return;
    memset(manifest, 0, sizeof(*manifest));
}

void zt_project_parse_result_init(zt_project_parse_result *result) {
    if (result == NULL) return;
    memset(result, 0, sizeof(*result));
    result->ok = 1;
    result->code = ZT_PROJECT_OK;
    zt_project_manifest_init(&result->manifest);
}

const char *zt_project_error_code_name(zt_project_error_code code) {
    switch (code) {
        case ZT_PROJECT_OK: return "project.ok";
        case ZT_PROJECT_INVALID_INPUT: return "project.invalid_input";
        case ZT_PROJECT_INVALID_SECTION: return "project.invalid_section";
        case ZT_PROJECT_UNKNOWN_KEY: return "project.unknown_key";
        case ZT_PROJECT_INVALID_ASSIGNMENT: return "project.invalid_assignment";
        case ZT_PROJECT_INVALID_STRING: return "project.invalid_string";
        case ZT_PROJECT_MISSING_FIELD: return "project.missing_field";
        case ZT_PROJECT_INVALID_KIND: return "project.invalid_kind";
        case ZT_PROJECT_INVALID_TARGET: return "project.invalid_target";
        case ZT_PROJECT_INVALID_PROFILE: return "project.invalid_profile";
        case ZT_PROJECT_PATH_TOO_LONG: return "project.path_too_long";
        case ZT_PROJECT_TOO_MANY_DEPENDENCIES: return "project.too_many_dependencies";
        default: return "project.unknown_error";
    }
}

int zt_project_parse_text(const char *text, size_t length, zt_project_parse_result *result) {
    size_t pos = 0;
    int line_number = 0;
    zt_project_section section = ZT_PROJECT_SECTION_NONE;

    zt_project_parse_result_init(result);

    if (text == NULL) {
        zt_project_set_error(result, ZT_PROJECT_INVALID_INPUT, 0, "manifest text cannot be null");
        return 0;
    }

    while (pos < length) {
        char line[2048];
        size_t start = pos;
        size_t line_length;
        int section_status;

        while (pos < length && text[pos] != '\n') pos += 1;
        line_length = pos - start;
        if (pos < length && text[pos] == '\n') pos += 1;
        line_number += 1;

        if (line_length >= sizeof(line)) {
            zt_project_set_error(result, ZT_PROJECT_INVALID_INPUT, line_number, "manifest line is too long");
            return 0;
        }

        memcpy(line, text + start, line_length);
        line[line_length] = '\0';
        zt_project_strip_comment(line);
        zt_project_trim_inplace(line);

        if (line[0] == '\0') {
            continue;
        }

        section_status = zt_project_parse_section(line, &section, result, line_number);
        if (section_status < 0) return 0;
        if (section_status > 0) continue;

        {
            int equals_pos = zt_project_find_assignment(line);
            char key[128];
            const char *value;

            if (equals_pos < 0) {
                zt_project_set_error(result, ZT_PROJECT_INVALID_ASSIGNMENT, line_number, "expected key = value");
                return 0;
            }

            if ((size_t)equals_pos >= sizeof(key)) {
                zt_project_set_error(result, ZT_PROJECT_INVALID_ASSIGNMENT, line_number, "manifest key is too long");
                return 0;
            }

            memcpy(key, line, (size_t)equals_pos);
            key[equals_pos] = '\0';
            zt_project_trim_inplace(key);

            if (key[0] == '\0') {
                zt_project_set_error(result, ZT_PROJECT_INVALID_ASSIGNMENT, line_number, "manifest key cannot be empty");
                return 0;
            }

            value = line + equals_pos + 1;
            if (!zt_project_assign_value(&result->manifest, section, key, value, result, line_number)) {
                return 0;
            }
        }
    }

    return zt_project_validate(result);
}

int zt_project_load_file(const char *path, zt_project_parse_result *result) {
    char *text;
    size_t length = 0;
    int ok;

    zt_project_parse_result_init(result);

    text = zt_project_read_file_text(path, &length);
    if (text == NULL) {
        zt_project_set_error(result, ZT_PROJECT_INVALID_INPUT, 0, "unable to read project manifest");
        return 0;
    }

    ok = zt_project_parse_text(text, length, result);
    free(text);
    return ok;
}

int zt_project_resolve_entry_source_path(
        const zt_project_manifest *manifest,
        const char *project_root,
        char *dest,
        size_t capacity) {
    const char *entry_namespace;
    size_t out = 0;
    size_t i;

    if (manifest == NULL || project_root == NULL || dest == NULL || capacity == 0) {
        return 0;
    }

    entry_namespace = zt_project_manifest_entry_namespace(manifest);
    if (entry_namespace == NULL || entry_namespace[0] == '\0') {
        return 0;
    }

    dest[0] = '\0';

#define ZT_APPEND_CHAR(ch) do { if (out + 1 >= capacity) return 0; dest[out++] = (ch); dest[out] = '\0'; } while (0)
#define ZT_APPEND_TEXT(txt) do { const char *_zt_p = (txt); while (_zt_p != NULL && *_zt_p != '\0') { ZT_APPEND_CHAR(*_zt_p); _zt_p += 1; } } while (0)

    ZT_APPEND_TEXT(project_root);
    if (out > 0 && dest[out - 1] != '/' && dest[out - 1] != '\\') {
        ZT_APPEND_CHAR('/');
    }

    ZT_APPEND_TEXT(manifest->source_root);
    if (out > 0 && dest[out - 1] != '/' && dest[out - 1] != '\\') {
        ZT_APPEND_CHAR('/');
    }

    for (i = 0; entry_namespace[i] != '\0'; i += 1) {
        ZT_APPEND_CHAR(entry_namespace[i] == '.' ? '/' : entry_namespace[i]);
    }

    ZT_APPEND_TEXT(".zt");

#undef ZT_APPEND_TEXT
#undef ZT_APPEND_CHAR

    return 1;
}
