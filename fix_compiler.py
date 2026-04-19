import os

# Fix main.c
main_path = 'compiler/driver/main.c'
with open(main_path, 'r', encoding='utf-8') as f:
    content = f.read()

target = "    if (!zt_project_discover_zt_files(source_root_path, &out->source_files)) {\n        goto fail;\n    }"
replacement = """    if (!zt_project_discover_zt_files(source_root_path, &out->source_files)) {
        goto fail;
    }

    {
        const char *stdlib_path = getenv("ZENITH_HOME");
        char default_stdlib_path[768];
        if (stdlib_path == NULL) {
            strcpy(default_stdlib_path, "stdlib");
            stdlib_path = default_stdlib_path;
        } else {
            if (!zt_join_path(default_stdlib_path, sizeof(default_stdlib_path), stdlib_path, "stdlib")) {
                strcpy(default_stdlib_path, "stdlib");
            }
            stdlib_path = default_stdlib_path;
        }
        
        if (zt_path_is_dir(stdlib_path)) {
            if (!zt_project_discover_zt_files(stdlib_path, &out->source_files)) {
                goto fail;
            }
        }
    }"""
content = content.replace(target, replacement)
with open(main_path, 'w', encoding='utf-8') as f:
    f.write(content)

# Fix checker.c
checker_path = 'compiler/semantic/types/checker.c'
with open(checker_path, 'r', encoding='utf-8') as f:
    content = f.read()

target_checker = """            }

            zt_type_dispose(result.type);
            result.type = expected_type != NULL ? zt_type_clone(expected_type) : zt_type_make(ZT_TYPE_UNKNOWN);
            zt_expr_info_dispose(&object_info);
            return result;
        }"""
replacement_checker = """            }

            {
                char combined_name[512];
                const zt_ast_node *func_decl;
                snprintf(combined_name, sizeof(combined_name), "%s.%s", alias, member);
                func_decl = zt_catalog_find_decl(&checker->catalog, combined_name);
                if (func_decl != NULL && func_decl->kind == ZT_AST_FUNC_DECL) {
                    zt_type_dispose(result.type);
                    result.type = zt_checker_resolve_type(checker, func_decl->as.func_decl.return_type, scope);
                    zt_expr_info_dispose(&object_info);
                    return result;
                }
                
                if (func_decl != NULL && func_decl->kind == ZT_AST_EXTERN_DECL) {
                    size_t fi;
                    for (fi = 0; fi < func_decl->as.extern_decl.functions.count; fi++) {
                        const zt_ast_node *ext_func = func_decl->as.extern_decl.functions.items[fi];
                        /* The external function might have been renamed or not, check base or prefixed name */
                        if (ext_func != NULL && (strcmp(ext_func->as.func_decl.name, combined_name) == 0 || strcmp(ext_func->as.func_decl.name, member) == 0)) {
                            zt_type_dispose(result.type);
                            result.type = zt_checker_resolve_type(checker, ext_func->as.func_decl.return_type, scope);
                            zt_expr_info_dispose(&object_info);
                            return result;
                        }
                    }
                }
            }

            zt_type_dispose(result.type);
            result.type = expected_type != NULL ? zt_type_clone(expected_type) : zt_type_make(ZT_TYPE_UNKNOWN);
            zt_expr_info_dispose(&object_info);
            return result;
        }"""
content = content.replace(target_checker, replacement_checker)
with open(checker_path, 'w', encoding='utf-8') as f:
    f.write(content)

print("Modifications applied.")
