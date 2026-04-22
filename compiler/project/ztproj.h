#ifndef ZENITH_NEXT_COMPILER_PROJECT_ZTPROJ_H
#define ZENITH_NEXT_COMPILER_PROJECT_ZTPROJ_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ZT_PROJECT_MAX_DEPENDENCIES 64
#define ZT_PROJECT_DEFAULT_MONOMORPHIZATION_LIMIT 1024u

typedef enum zt_project_error_code {
    ZT_PROJECT_OK = 0,
    ZT_PROJECT_INVALID_INPUT,
    ZT_PROJECT_INVALID_SECTION,
    ZT_PROJECT_UNKNOWN_KEY,
    ZT_PROJECT_INVALID_ASSIGNMENT,
    ZT_PROJECT_INVALID_STRING,
    ZT_PROJECT_MISSING_FIELD,
    ZT_PROJECT_INVALID_KIND,
    ZT_PROJECT_INVALID_TARGET,
    ZT_PROJECT_INVALID_PROFILE,
    ZT_PROJECT_INVALID_MONOMORPHIZATION_LIMIT,
    ZT_PROJECT_PATH_TOO_LONG,
    ZT_PROJECT_TOO_MANY_DEPENDENCIES
} zt_project_error_code;

typedef enum zt_project_kind {
    ZT_PROJECT_KIND_UNKNOWN = 0,
    ZT_PROJECT_KIND_APP,
    ZT_PROJECT_KIND_LIB
} zt_project_kind;

typedef struct zt_project_dependency_entry {
    char name[64];
    char spec[256];
} zt_project_dependency_entry;

typedef struct zt_project_manifest {
    char project_name[128];
    char project_kind[16];
    char version[64];
    char lang[16];

    char source_root[256];
    char test_root[256];
    char zdoc_root[256];

    char app_entry[128];
    char lib_root_namespace[128];

    char build_target[32];
    char build_profile[32];
    char build_output[256];
    size_t build_monomorphization_limit;
    char build_linker_flags[512];

    size_t dependency_count;
    zt_project_dependency_entry dependencies[ZT_PROJECT_MAX_DEPENDENCIES];

    size_t dev_dependency_count;
    zt_project_dependency_entry dev_dependencies[ZT_PROJECT_MAX_DEPENDENCIES];

/* Internal compatibility fields used by the current bootstrap driver. */
    char entry[128];
    char output_name[128];
    char output_dir[256];
    char diag_profile[32];

    /* Accessibility and Telemetry (Cognitive Accessibility by Design) */
    char accessibility_profile[32];
    int accessibility_telemetry;
} zt_project_manifest;

typedef struct zt_project_parse_result {
    int ok;
    zt_project_error_code code;
    int line_number;
    char message[256];
    zt_project_manifest manifest;
} zt_project_parse_result;

void zt_project_manifest_init(zt_project_manifest *manifest);
void zt_project_parse_result_init(zt_project_parse_result *result);
const char *zt_project_error_code_name(zt_project_error_code code);
zt_project_kind zt_project_manifest_kind(const zt_project_manifest *manifest);
const char *zt_project_manifest_entry_namespace(const zt_project_manifest *manifest);

int zt_project_parse_text(const char *text, size_t length, zt_project_parse_result *result);
int zt_project_load_file(const char *path, zt_project_parse_result *result);

int zt_project_resolve_entry_source_path(
    const zt_project_manifest *manifest,
    const char *project_root,
    char *dest,
    size_t capacity
);

#ifdef __cplusplus
}
#endif

#endif
