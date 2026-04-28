#ifndef ZENITH_NEXT_COMPILER_PROJECT_LOCKFILE_H
#define ZENITH_NEXT_COMPILER_PROJECT_LOCKFILE_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ZT_LOCKFILE_MAX_PACKAGES 128

typedef enum zt_lock_source_kind {
    ZT_LOCK_SOURCE_VERSION = 0,
    ZT_LOCK_SOURCE_GIT,
    ZT_LOCK_SOURCE_PATH
} zt_lock_source_kind;

typedef struct zt_lock_package {
    char name[64];
    zt_lock_source_kind source;
    char version[64];
    char git_url[256];
    char git_rev[64];
    char git_tag[64];
    char git_branch[64];
    char path[256];
} zt_lock_package;

typedef struct zt_lockfile {
    int schema_version;
    char generated_by[64];
    size_t package_count;
    zt_lock_package packages[ZT_LOCKFILE_MAX_PACKAGES];
} zt_lockfile;

void zt_lockfile_init(zt_lockfile *lock);
int zt_lockfile_serialize(const zt_lockfile *lock, char *buffer, size_t capacity);
int zt_lockfile_save(const zt_lockfile *lock, const char *path);
int zt_lockfile_parse_text(const char *text, size_t length, zt_lockfile *lock, char *error, size_t error_cap);
int zt_lockfile_load(const char *path, zt_lockfile *lock, char *error, size_t error_cap);
const zt_lock_package *zt_lockfile_find_package(const zt_lockfile *lock, const char *name);

#ifdef __cplusplus
}
#endif

#endif
