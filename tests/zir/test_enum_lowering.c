#include "compiler/utils/arena.h"
#include "compiler/utils/string_pool.h"
#include "compiler/frontend/parser/parser.h"
#include "compiler/hir/lowering/from_ast.h"
#include "compiler/zir/lowering/from_hir.h"
#include "compiler/zir/verifier.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void fail_now(const char *message) {
    fprintf(stderr, "%s\n", message);
    exit(1);
}

int main(void) {
    zt_arena arena;
    zt_string_pool pool;
    const char *src =
        "namespace app\n"
        "enum LoadUserError\n"
        "    NotFound(id: int)\n"
        "    InvalidName(message: text)\n"
        "    Timeout\n"
        "end\n"
        "func main() -> int\n"
        "    return 0\n"
        "end";
    zt_parser_result parsed;
    zt_hir_lower_result hir;
    zir_lower_result zir;
    zir_verifier_result verify;
    const zir_enum_decl *enum_decl;

    zt_arena_init(&arena, 65536);
    zt_string_pool_init(&pool, &arena);

    parsed = zt_parse(&arena, &pool, "enum_payload.zt", src, strlen(src));
    if (parsed.diagnostics.count != 0) {
        fail_now("falha: parser diagnostics inesperados em enum lowering");
    }

    hir = zt_lower_ast_to_hir(parsed.root);
    if (hir.diagnostics.count != 0) {
        zt_parser_result_dispose(&parsed);
        fail_now("falha: HIR diagnostics inesperados em enum lowering");
    }

    zir = zir_lower_hir_to_zir(hir.module);
    if (zir.diagnostics.count != 0) {
        zt_hir_lower_result_dispose(&hir);
        zt_parser_result_dispose(&parsed);
        fail_now("falha: ZIR diagnostics inesperados em enum lowering");
    }

    if (zir.module.enum_count != 1) {
        zir_lower_result_dispose(&zir);
        zt_hir_lower_result_dispose(&hir);
        zt_parser_result_dispose(&parsed);
        fail_now("falha: enum_count esperado 1");
    }

    enum_decl = &zir.module.enums[0];
    if (strcmp(enum_decl->name, "LoadUserError") != 0) {
        zir_lower_result_dispose(&zir);
        zt_hir_lower_result_dispose(&hir);
        zt_parser_result_dispose(&parsed);
        fail_now("falha: nome do enum inesperado");
    }

    if (enum_decl->variant_count != 3) {
        zir_lower_result_dispose(&zir);
        zt_hir_lower_result_dispose(&hir);
        zt_parser_result_dispose(&parsed);
        fail_now("falha: variant_count esperado 3");
    }

    if (strcmp(enum_decl->variants[0].name, "NotFound") != 0 || enum_decl->variants[0].field_count != 1) {
        zir_lower_result_dispose(&zir);
        zt_hir_lower_result_dispose(&hir);
        zt_parser_result_dispose(&parsed);
        fail_now("falha: variante NotFound inesperada");
    }

    if (strcmp(enum_decl->variants[0].fields[0].name, "id") != 0 ||
        strcmp(enum_decl->variants[0].fields[0].type_name, "int") != 0) {
        zir_lower_result_dispose(&zir);
        zt_hir_lower_result_dispose(&hir);
        zt_parser_result_dispose(&parsed);
        fail_now("falha: campo id:int inesperado");
    }

    if (strcmp(enum_decl->variants[1].name, "InvalidName") != 0 || enum_decl->variants[1].field_count != 1) {
        zir_lower_result_dispose(&zir);
        zt_hir_lower_result_dispose(&hir);
        zt_parser_result_dispose(&parsed);
        fail_now("falha: variante InvalidName inesperada");
    }

    if (strcmp(enum_decl->variants[1].fields[0].name, "message") != 0 ||
        strcmp(enum_decl->variants[1].fields[0].type_name, "text") != 0) {
        zir_lower_result_dispose(&zir);
        zt_hir_lower_result_dispose(&hir);
        zt_parser_result_dispose(&parsed);
        fail_now("falha: campo message:text inesperado");
    }

    if (strcmp(enum_decl->variants[2].name, "Timeout") != 0 || enum_decl->variants[2].field_count != 0) {
        zir_lower_result_dispose(&zir);
        zt_hir_lower_result_dispose(&hir);
        zt_parser_result_dispose(&parsed);
        fail_now("falha: variante Timeout inesperada");
    }

    if (!zir_verify_module(&zir.module, &verify)) {
        zir_lower_result_dispose(&zir);
        zt_hir_lower_result_dispose(&hir);
        zt_parser_result_dispose(&parsed);
        fail_now("falha: verifier rejeitou enum lowered");
    }

    zir_lower_result_dispose(&zir);
    zt_hir_lower_result_dispose(&hir);
    zt_parser_result_dispose(&parsed);

    puts("ZIR enum lowering tests OK");
    return 0;
}