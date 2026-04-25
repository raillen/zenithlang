/*
 * zpm.h — Zenith Package Manager (ZPM) command handlers.
 *
 * This module implements the "zpm" subcommand for the Zenith driver.
 */
#ifndef ZT_ZPM_H
#define ZT_ZPM_H

#include "compiler/driver/driver_internal.h"

/**
 * Main entry point for the "zpm" subcommand.
 * Dispatches to individual zpm command handlers (init, add, install, etc.)
 */
int zt_handle_zpm(zt_driver_context *ctx, int argc, char **argv);

/* Subcommand handlers */
int zt_handle_zpm_init(zt_driver_context *ctx, const char *path);
int zt_handle_zpm_add(zt_driver_context *ctx, const char *pkg_spec);
int zt_handle_zpm_remove(zt_driver_context *ctx, const char *pkg_name);
int zt_handle_zpm_install(zt_driver_context *ctx, const char *project_path);
int zt_handle_zpm_update(zt_driver_context *ctx, const char *pkg_name);
int zt_handle_zpm_list(zt_driver_context *ctx, const char *project_path);
int zt_handle_zpm_find(zt_driver_context *ctx, const char *query, int all);
int zt_handle_zpm_publish(zt_driver_context *ctx, const char *project_path);

#endif /* ZT_ZPM_H */
