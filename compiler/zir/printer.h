#ifndef ZENITH_NEXT_COMPILER_ZIR_PRINTER_H
#define ZENITH_NEXT_COMPILER_ZIR_PRINTER_H

#include "compiler/zir/model.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct zir_string_buffer {
    char *data;
    size_t length;
    size_t capacity;
} zir_string_buffer;

typedef struct zir_printer {
    zir_string_buffer buffer;
    const char *newline;
} zir_printer;

void zir_printer_init(zir_printer *printer);
void zir_printer_reset(zir_printer *printer);
void zir_printer_dispose(zir_printer *printer);
const char *zir_printer_text(const zir_printer *printer);
int zir_printer_print_module(zir_printer *printer, const zir_module *module_decl);

#ifdef __cplusplus
}
#endif

#endif

