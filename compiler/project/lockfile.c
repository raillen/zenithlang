#include "compiler/project/lockfile.h"
#include "compiler/driver/driver_internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void zt_lockfile_init(zt_lockfile *lock) {
    if (lock == NULL) return;
    memset(lock, 0, sizeof(*lock));
    lock->schema_version = 1;
    strcpy(lock->generated_by, "zt-next");
}

int zt_lockfile_serialize(const zt_lockfile *lock, char *buffer, size_t capacity) {
    size_t out = 0;
    size_t i;

    if (lock == NULL || buffer == NULL || capacity == 0) return 0;

#define ZT_SLOCK(format, ...) do { \
        int _zt_res = snprintf(buffer + out, capacity - out, format, ##__VA_ARGS__); \
        if (_zt_res < 0 || (size_t)_zt_res >= capacity - out) return 0; \
        out += (size_t)_zt_res; \
    } while (0)

    ZT_SLOCK("schema = %d\n", lock->schema_version);
    ZT_SLOCK("generated_by = \"%s\"\n\n", lock->generated_by);

    for (i = 0; i < lock->package_count; i += 1) {
        const zt_lock_package *pkg = &lock->packages[i];
        ZT_SLOCK("[[package]]\n");
        ZT_SLOCK("name = \"%s\"\n", pkg->name);
        
        switch (pkg->source) {
            case ZT_LOCK_SOURCE_VERSION:
                ZT_SLOCK("source = \"version\"\n");
                ZT_SLOCK("version = \"%s\"\n", pkg->version);
                break;
            case ZT_LOCK_SOURCE_GIT:
                ZT_SLOCK("source = \"git\"\n");
                ZT_SLOCK("git = \"%s\"\n", pkg->git_url);
                ZT_SLOCK("rev = \"%s\"\n", pkg->git_rev);
                if (pkg->git_tag[0] != '\0') ZT_SLOCK("tag = \"%s\"\n", pkg->git_tag);
                if (pkg->git_branch[0] != '\0') ZT_SLOCK("branch = \"%s\"\n", pkg->git_branch);
                break;
            case ZT_LOCK_SOURCE_PATH:
                ZT_SLOCK("source = \"path\"\n");
                ZT_SLOCK("path = \"%s\"\n", pkg->path);
                break;
        }
        ZT_SLOCK("\n");
    }

#undef ZT_SLOCK
    return 1;
}

int zt_lockfile_save(const zt_lockfile *lock, const char *path) {
    char buffer[32768];
    if (!zt_lockfile_serialize(lock, buffer, sizeof(buffer))) return 0;
    return zt_write_file(path, buffer);
}

static void zt_lockfile_set_error(char *error, size_t error_cap, const char *message) {
    if (error == NULL || error_cap == 0) return;
    snprintf(error, error_cap, "%s", message != NULL ? message : "invalid lockfile");
}

static char *zt_lockfile_trim_left(char *text) {
    while (*text == ' ' || *text == '\t' || *text == '\r' || *text == '\n') text += 1;
    return text;
}

static void zt_lockfile_trim_inplace(char *text) {
    char *start;
    char *end;

    if (text == NULL) return;
    start = zt_lockfile_trim_left(text);
    if (start != text) memmove(text, start, strlen(start) + 1);

    end = text + strlen(text);
    while (end > text &&
            (end[-1] == ' ' || end[-1] == '\t' || end[-1] == '\r' || end[-1] == '\n')) {
        end -= 1;
    }
    *end = '\0';
}

static void zt_lockfile_strip_comment(char *text) {
    size_t index = 0;
    int in_string = 0;

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

static int zt_lockfile_parse_string(const char *value, char *dest, size_t dest_cap) {
    const char *cursor;
    size_t out = 0;

    if (value == NULL || dest == NULL || dest_cap == 0) return 0;
    dest[0] = '\0';

    cursor = value;
    while (*cursor == ' ' || *cursor == '\t') cursor += 1;
    if (*cursor != '"') return 0;
    cursor += 1;

    while (*cursor != '\0') {
        char ch = *cursor;
        if (ch == '"') {
            cursor += 1;
            while (*cursor == ' ' || *cursor == '\t') cursor += 1;
            if (*cursor != '\0') return 0;
            dest[out] = '\0';
            return 1;
        }
        if (out + 1 >= dest_cap) return 0;
        if (ch == '\\' && cursor[1] != '\0') {
            cursor += 1;
            ch = *cursor;
        }
        dest[out++] = ch;
        cursor += 1;
    }

    return 0;
}

static int zt_lockfile_package_valid(const zt_lock_package *pkg, char *error, size_t error_cap) {
    if (pkg == NULL || pkg->name[0] == '\0') {
        zt_lockfile_set_error(error, error_cap, "lock package is missing name");
        return 0;
    }

    switch (pkg->source) {
        case ZT_LOCK_SOURCE_VERSION:
            if (pkg->version[0] == '\0') {
                zt_lockfile_set_error(error, error_cap, "version package is missing version");
                return 0;
            }
            return 1;
        case ZT_LOCK_SOURCE_GIT:
            if (pkg->git_url[0] == '\0' || pkg->git_rev[0] == '\0') {
                zt_lockfile_set_error(error, error_cap, "git package is missing git or rev");
                return 0;
            }
            return 1;
        case ZT_LOCK_SOURCE_PATH:
            if (pkg->path[0] == '\0') {
                zt_lockfile_set_error(error, error_cap, "path package is missing path");
                return 0;
            }
            return 1;
    }

    zt_lockfile_set_error(error, error_cap, "package has unknown source");
    return 0;
}

const zt_lock_package *zt_lockfile_find_package(const zt_lockfile *lock, const char *name) {
    size_t i;
    if (lock == NULL || name == NULL) return NULL;
    for (i = 0; i < lock->package_count; i += 1) {
        if (strcmp(lock->packages[i].name, name) == 0) return &lock->packages[i];
    }
    return NULL;
}

int zt_lockfile_parse_text(const char *text, size_t length, zt_lockfile *lock, char *error, size_t error_cap) {
    char *copy;
    char *line;
    char *next;
    zt_lock_package *current = NULL;
    int schema_seen = 0;

    if (text == NULL || lock == NULL) {
        zt_lockfile_set_error(error, error_cap, "invalid lockfile input");
        return 0;
    }

    copy = (char *)malloc(length + 1);
    if (copy == NULL) {
        zt_lockfile_set_error(error, error_cap, "out of memory while reading lockfile");
        return 0;
    }
    memcpy(copy, text, length);
    copy[length] = '\0';

    zt_lockfile_init(lock);
    lock->package_count = 0;

    line = copy;
    while (line != NULL) {
        char *eq;
        char *key;
        char *value;

        next = strchr(line, '\n');
        if (next != NULL) {
            *next = '\0';
            next += 1;
        }

        zt_lockfile_strip_comment(line);
        zt_lockfile_trim_inplace(line);
        if (line[0] == '\0') {
            line = next;
            continue;
        }

        if (strcmp(line, "[[package]]") == 0) {
            if (current != NULL && !zt_lockfile_package_valid(current, error, error_cap)) {
                free(copy);
                return 0;
            }
            if (lock->package_count >= ZT_LOCKFILE_MAX_PACKAGES) {
                zt_lockfile_set_error(error, error_cap, "too many packages in lockfile");
                free(copy);
                return 0;
            }
            current = &lock->packages[lock->package_count++];
            memset(current, 0, sizeof(*current));
            current->source = ZT_LOCK_SOURCE_VERSION;
            line = next;
            continue;
        }

        eq = strchr(line, '=');
        if (eq == NULL) {
            zt_lockfile_set_error(error, error_cap, "expected key = value in lockfile");
            free(copy);
            return 0;
        }

        *eq = '\0';
        key = line;
        value = eq + 1;
        zt_lockfile_trim_inplace(key);
        zt_lockfile_trim_inplace(value);

        if (current == NULL) {
            if (strcmp(key, "schema") == 0) {
                if (strcmp(value, "1") != 0) {
                    zt_lockfile_set_error(error, error_cap, "unsupported lockfile schema");
                    free(copy);
                    return 0;
                }
                schema_seen = 1;
            } else if (strcmp(key, "generated_by") == 0) {
                if (!zt_lockfile_parse_string(value, lock->generated_by, sizeof(lock->generated_by))) {
                    zt_lockfile_set_error(error, error_cap, "invalid generated_by value");
                    free(copy);
                    return 0;
                }
            } else {
                zt_lockfile_set_error(error, error_cap, "unknown root key in lockfile");
                free(copy);
                return 0;
            }
        } else {
            if (strcmp(key, "name") == 0) {
                if (!zt_lockfile_parse_string(value, current->name, sizeof(current->name))) {
                    zt_lockfile_set_error(error, error_cap, "invalid package name");
                    free(copy);
                    return 0;
                }
                if (zt_lockfile_find_package(lock, current->name) != current) {
                    zt_lockfile_set_error(error, error_cap, "duplicate package name in lockfile");
                    free(copy);
                    return 0;
                }
            } else if (strcmp(key, "source") == 0) {
                char source[32];
                if (!zt_lockfile_parse_string(value, source, sizeof(source))) {
                    zt_lockfile_set_error(error, error_cap, "invalid package source");
                    free(copy);
                    return 0;
                }
                if (strcmp(source, "version") == 0) current->source = ZT_LOCK_SOURCE_VERSION;
                else if (strcmp(source, "git") == 0) current->source = ZT_LOCK_SOURCE_GIT;
                else if (strcmp(source, "path") == 0) current->source = ZT_LOCK_SOURCE_PATH;
                else {
                    zt_lockfile_set_error(error, error_cap, "unsupported package source");
                    free(copy);
                    return 0;
                }
            } else if (strcmp(key, "version") == 0) {
                if (!zt_lockfile_parse_string(value, current->version, sizeof(current->version))) {
                    zt_lockfile_set_error(error, error_cap, "invalid package version");
                    free(copy);
                    return 0;
                }
            } else if (strcmp(key, "git") == 0) {
                if (!zt_lockfile_parse_string(value, current->git_url, sizeof(current->git_url))) {
                    zt_lockfile_set_error(error, error_cap, "invalid git URL");
                    free(copy);
                    return 0;
                }
            } else if (strcmp(key, "rev") == 0) {
                if (!zt_lockfile_parse_string(value, current->git_rev, sizeof(current->git_rev))) {
                    zt_lockfile_set_error(error, error_cap, "invalid git rev");
                    free(copy);
                    return 0;
                }
            } else if (strcmp(key, "tag") == 0) {
                if (!zt_lockfile_parse_string(value, current->git_tag, sizeof(current->git_tag))) {
                    zt_lockfile_set_error(error, error_cap, "invalid git tag");
                    free(copy);
                    return 0;
                }
            } else if (strcmp(key, "branch") == 0) {
                if (!zt_lockfile_parse_string(value, current->git_branch, sizeof(current->git_branch))) {
                    zt_lockfile_set_error(error, error_cap, "invalid git branch");
                    free(copy);
                    return 0;
                }
            } else if (strcmp(key, "path") == 0) {
                if (!zt_lockfile_parse_string(value, current->path, sizeof(current->path))) {
                    zt_lockfile_set_error(error, error_cap, "invalid package path");
                    free(copy);
                    return 0;
                }
            } else {
                zt_lockfile_set_error(error, error_cap, "unknown package key in lockfile");
                free(copy);
                return 0;
            }
        }

        line = next;
    }

    if (!schema_seen) {
        zt_lockfile_set_error(error, error_cap, "lockfile is missing schema");
        free(copy);
        return 0;
    }
    if (current != NULL && !zt_lockfile_package_valid(current, error, error_cap)) {
        free(copy);
        return 0;
    }

    free(copy);
    return 1;
}

int zt_lockfile_load(const char *path, zt_lockfile *lock, char *error, size_t error_cap) {
    char *content;
    size_t length = 0;
    int ok;

    if (path == NULL) {
        zt_lockfile_set_error(error, error_cap, "missing lockfile path");
        return 0;
    }

    content = zt_read_file(path);
    if (content == NULL) {
        zt_lockfile_set_error(error, error_cap, "failed to read lockfile");
        return 0;
    }

    length = strlen(content);
    ok = zt_lockfile_parse_text(content, length, lock, error, error_cap);
    free(content);
    return ok;
}
