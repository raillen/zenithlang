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

#ifdef __cplusplus
}
#endif

#endif
