#include <stdio.h>
#include <stdlib.h>
#include "runtime/c/zenith_rt.h"

void stress_test_memory_allocation() {
    printf("Iniciando teste de estresse de alocação de memória...\n");

    for (size_t i = 0; i < 1000000; i++) {
        zt_text *text = zt_text_pool_alloc();
        text->data = malloc(256);
        snprintf(text->data, 256, "Texto de teste %zu", i);
        text->len = strlen(text->data);
        zt_text_pool_free(text);
    }

    printf("Teste de estresse de alocação de memória concluído.\n");
}

void stress_test_map_operations() {
    printf("Iniciando teste de estresse de operações em mapas...\n");

    zt_map_text_text *map = malloc(sizeof(zt_map_text_text));
    map->keys = calloc(16, sizeof(zt_text *));
    map->values = calloc(16, sizeof(zt_text *));
    map->len = 0;
    map->capacity = 16;

    for (size_t i = 0; i < 100000; i++) {
        zt_text *key = zt_text_pool_alloc();
        zt_text *value = zt_text_pool_alloc();
        key->data = malloc(32);
        value->data = malloc(32);
        snprintf(key->data, 32, "Chave %zu", i);
        snprintf(value->data, 32, "Valor %zu", i);
        key->len = strlen(key->data);
        value->len = strlen(value->data);
        zt_map_text_text_set(map, key, value);
    }

    printf("Teste de estresse de operações em mapas concluído.\n");
}

int main() {
    stress_test_memory_allocation();
    stress_test_map_operations();
    return 0;
}