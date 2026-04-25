#include "compiler/project/lockfile.h"
#include "compiler/driver/driver_internal.h"
#include <stdio.h>
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
