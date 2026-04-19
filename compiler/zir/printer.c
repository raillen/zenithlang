#include "compiler/zir/printer.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *zir_safe_text(const char *text) {
    return text != NULL ? text : "";
}

static int zir_buffer_reserve(zir_string_buffer *buffer, size_t additional) {
    size_t needed = buffer->length + additional + 1;
    if (needed <= buffer->capacity) {
        return 1;
    }

    size_t new_capacity = buffer->capacity > 0 ? buffer->capacity : 128;
    while (new_capacity < needed) {
        new_capacity *= 2;
    }

    char *new_data = (char *)realloc(buffer->data, new_capacity);
    if (new_data == NULL) {
        return 0;
    }

    buffer->data = new_data;
    buffer->capacity = new_capacity;
    return 1;
}

static int zir_buffer_append_n(zir_string_buffer *buffer, const char *text, size_t length) {
    if (!zir_buffer_reserve(buffer, length)) {
        return 0;
    }

    memcpy(buffer->data + buffer->length, text, length);
    buffer->length += length;
    buffer->data[buffer->length] = '\0';
    return 1;
}

static int zir_buffer_append(zir_string_buffer *buffer, const char *text) {
    return zir_buffer_append_n(buffer, zir_safe_text(text), strlen(zir_safe_text(text)));
}

static int zir_buffer_append_vformat(zir_string_buffer *buffer, const char *format, va_list args) {
    va_list copy;
    va_copy(copy, args);
    int needed = vsnprintf(NULL, 0, format, copy);
    va_end(copy);

    if (needed < 0) {
        return 0;
    }

    if (!zir_buffer_reserve(buffer, (size_t)needed)) {
        return 0;
    }

    vsnprintf(buffer->data + buffer->length, buffer->capacity - buffer->length, format, args);
    buffer->length += (size_t)needed;
    return 1;
}

static int zir_buffer_append_format(zir_string_buffer *buffer, const char *format, ...) {
    va_list args;
    va_start(args, format);
    int ok = zir_buffer_append_vformat(buffer, format, args);
    va_end(args);
    return ok;
}

static int zir_printer_begin_line(zir_printer *printer) {
    if (printer->buffer.length > 0) {
        return zir_buffer_append(&printer->buffer, printer->newline);
    }
    return 1;
}

static int zir_printer_emit_line(zir_printer *printer, const char *text) {
    return zir_printer_begin_line(printer) && zir_buffer_append(&printer->buffer, zir_safe_text(text));
}

static int zir_printer_emit_span_line(zir_printer *printer, zir_span span, const char *indent) {
    if (!zir_span_is_known(span)) {
        return 1;
    }

    if (!zir_printer_begin_line(printer)) {
        return 0;
    }

    return zir_buffer_append_format(
        &printer->buffer,
        "%s@span %s:%zu:%zu",
        zir_safe_text(indent),
        zir_safe_text(span.source_name),
        span.line,
        span.column
    );
}

static int zir_printer_emit_expr_text(zir_printer *printer, const zir_expr *expr, const char *fallback) {
    char *rendered = NULL;
    int ok;
    if (expr != NULL) rendered = zir_expr_render_alloc(expr);
    ok = zir_buffer_append(&printer->buffer, rendered != NULL ? rendered : zir_safe_text(fallback));
    free(rendered);
    return ok;
}

static int zir_printer_emit_params(zir_printer *printer, const zir_function *function_decl) {
    size_t index;
    for (index = 0; index < function_decl->param_count; ++index) {
        const zir_param *param = &function_decl->params[index];
        if (index > 0 && !zir_buffer_append(&printer->buffer, ", ")) {
            return 0;
        }
        if (!zir_buffer_append_format(&printer->buffer, "%s: %s", zir_safe_text(param->name), zir_safe_text(param->type_name))) {
            return 0;
        }
    }
    return 1;
}

static int zir_printer_emit_instruction(zir_printer *printer, const zir_instruction *instruction) {
    if (!zir_printer_emit_span_line(printer, instruction->span, "  ")) {
        return 0;
    }

    if (!zir_printer_begin_line(printer) || !zir_buffer_append(&printer->buffer, "  ")) {
        return 0;
    }

    if (instruction->kind == ZIR_INSTR_ASSIGN) {
        if (!zir_buffer_append_format(
                &printer->buffer,
                "%s: %s = ",
                zir_safe_text(instruction->dest_name),
                zir_safe_text(instruction->type_name))) {
            return 0;
        }
        return zir_printer_emit_expr_text(printer, instruction->expr, instruction->expr_text);
    }

    if (instruction->kind == ZIR_INSTR_CHECK_CONTRACT) {
        if (!zir_buffer_append(&printer->buffer, "check_contract ")) {
            return 0;
        }
        return zir_printer_emit_expr_text(printer, instruction->expr, instruction->expr_text);
    }

    return zir_printer_emit_expr_text(printer, instruction->expr, instruction->expr_text);
}

static int zir_printer_emit_terminator(zir_printer *printer, const zir_terminator *terminator) {
    if (!zir_printer_emit_span_line(printer, terminator->span, "  ")) {
        return 0;
    }

    if (!zir_printer_begin_line(printer) || !zir_buffer_append(&printer->buffer, "  ")) {
        return 0;
    }

    switch (terminator->kind) {
        case ZIR_TERM_RETURN:
            if (terminator->value == NULL && zir_safe_text(terminator->value_text)[0] == '\0') {
                return zir_buffer_append(&printer->buffer, "return");
            }
            return zir_buffer_append(&printer->buffer, "return ") &&
                   zir_printer_emit_expr_text(printer, terminator->value, terminator->value_text);
        case ZIR_TERM_JUMP:
            return zir_buffer_append_format(&printer->buffer, "jump %s", zir_safe_text(terminator->target_label));
        case ZIR_TERM_BRANCH_IF:
            return zir_buffer_append(&printer->buffer, "branch_if ") &&
                   zir_printer_emit_expr_text(printer, terminator->condition, terminator->condition_text) &&
                   zir_buffer_append_format(
                       &printer->buffer,
                       ", %s, %s",
                       zir_safe_text(terminator->then_label),
                       zir_safe_text(terminator->else_label)
                   );
        case ZIR_TERM_PANIC:
            if (terminator->message == NULL && zir_safe_text(terminator->message_text)[0] == '\0') {
                return zir_buffer_append(&printer->buffer, "panic");
            }
            return zir_buffer_append(&printer->buffer, "panic ") &&
                   zir_printer_emit_expr_text(printer, terminator->message, terminator->message_text);
        case ZIR_TERM_UNREACHABLE:
        default:
            return zir_buffer_append(&printer->buffer, "unreachable");
    }
}

static int zir_printer_emit_block(zir_printer *printer, const zir_block *block) {
    if (!zir_printer_emit_line(printer, "")) {
        return 0;
    }
    if (!zir_printer_emit_span_line(printer, block->span, "")) {
        return 0;
    }
    if (!zir_printer_begin_line(printer)) {
        return 0;
    }
    if (!zir_buffer_append_format(&printer->buffer, "block %s:", zir_safe_text(block->label))) {
        return 0;
    }

    for (size_t index = 0; index < block->instruction_count; ++index) {
        if (!zir_printer_emit_instruction(printer, &block->instructions[index])) {
            return 0;
        }
    }

    return zir_printer_emit_terminator(printer, &block->terminator);
}

static int zir_printer_emit_struct(zir_printer *printer, const zir_struct_decl *struct_decl) {
    size_t index;
    if (!zir_printer_emit_line(printer, "")) {
        return 0;
    }
    if (!zir_printer_emit_span_line(printer, struct_decl->span, "")) {
        return 0;
    }
    if (!zir_printer_begin_line(printer)) {
        return 0;
    }
    if (!zir_buffer_append_format(&printer->buffer, "struct %s", zir_safe_text(struct_decl->name))) {
        return 0;
    }

    for (index = 0; index < struct_decl->field_count; ++index) {
        const zir_field_decl *field = &struct_decl->fields[index];
        if (!zir_printer_emit_span_line(printer, field->span, "  ")) {
            return 0;
        }
        if (!zir_printer_begin_line(printer) || !zir_buffer_append(&printer->buffer, "  ")) {
            return 0;
        }
        if (!zir_buffer_append_format(&printer->buffer, "%s: %s", zir_safe_text(field->name), zir_safe_text(field->type_name))) {
            return 0;
        }
    }

    return 1;
}
static int zir_printer_emit_enum(zir_printer *printer, const zir_enum_decl *enum_decl) {
    size_t variant_index;
    if (!zir_printer_emit_line(printer, "")) {
        return 0;
    }
    if (!zir_printer_emit_span_line(printer, enum_decl->span, "")) {
        return 0;
    }
    if (!zir_printer_begin_line(printer)) {
        return 0;
    }
    if (!zir_buffer_append_format(&printer->buffer, "enum %s", zir_safe_text(enum_decl->name))) {
        return 0;
    }

    for (variant_index = 0; variant_index < enum_decl->variant_count; ++variant_index) {
        const zir_enum_variant_decl *variant = &enum_decl->variants[variant_index];
        size_t field_index;

        if (!zir_printer_emit_span_line(printer, variant->span, "  ")) {
            return 0;
        }
        if (!zir_printer_begin_line(printer) || !zir_buffer_append(&printer->buffer, "  ")) {
            return 0;
        }
        if (!zir_buffer_append(&printer->buffer, zir_safe_text(variant->name))) {
            return 0;
        }

        if (variant->field_count > 0) {
            if (!zir_buffer_append(&printer->buffer, "(")) {
                return 0;
            }
            for (field_index = 0; field_index < variant->field_count; ++field_index) {
                const zir_enum_variant_field_decl *field = &variant->fields[field_index];
                if (field_index > 0 && !zir_buffer_append(&printer->buffer, ", ")) {
                    return 0;
                }
                if (!zir_buffer_append_format(&printer->buffer, "%s: %s", zir_safe_text(field->name), zir_safe_text(field->type_name))) {
                    return 0;
                }
            }
            if (!zir_buffer_append(&printer->buffer, ")")) {
                return 0;
            }
        }
    }

    return 1;
}

static int zir_printer_emit_function(zir_printer *printer, const zir_function *function_decl) {
    if (!zir_printer_emit_line(printer, "")) {
        return 0;
    }
    if (!zir_printer_emit_span_line(printer, function_decl->span, "")) {
        return 0;
    }
    if (!zir_printer_begin_line(printer)) {
        return 0;
    }
    if (!zir_buffer_append_format(&printer->buffer, "func %s(", zir_safe_text(function_decl->name))) {
        return 0;
    }
    if (!zir_printer_emit_params(printer, function_decl)) {
        return 0;
    }
    if (!zir_buffer_append_format(&printer->buffer, ") -> %s", zir_safe_text(function_decl->return_type))) {
        return 0;
    }

    for (size_t index = 0; index < function_decl->block_count; ++index) {
        if (!zir_printer_emit_block(printer, &function_decl->blocks[index])) {
            return 0;
        }
    }

    return 1;
}

void zir_printer_init(zir_printer *printer) {
    printer->buffer.data = NULL;
    printer->buffer.length = 0;
    printer->buffer.capacity = 0;
    printer->newline = "\n";
}

void zir_printer_reset(zir_printer *printer) {
    printer->buffer.length = 0;
    if (printer->buffer.data != NULL) {
        printer->buffer.data[0] = '\0';
    }
}

void zir_printer_dispose(zir_printer *printer) {
    free(printer->buffer.data);
    printer->buffer.data = NULL;
    printer->buffer.length = 0;
    printer->buffer.capacity = 0;
}

const char *zir_printer_text(const zir_printer *printer) {
    return printer->buffer.data != NULL ? printer->buffer.data : "";
}

int zir_printer_print_module(zir_printer *printer, const zir_module *module_decl) {
    zir_printer_reset(printer);

    if (!zir_printer_emit_line(printer, "")) {
        return 0;
    }
    printer->buffer.length = 0;
    if (printer->buffer.data != NULL) {
        printer->buffer.data[0] = '\0';
    }

    if (!zir_printer_begin_line(printer)) {
        return 0;
    }
    if (!zir_buffer_append_format(&printer->buffer, "module %s", zir_safe_text(module_decl->name))) {
        return 0;
    }

    for (size_t index = 0; index < module_decl->struct_count; ++index) {
        if (!zir_printer_emit_struct(printer, &module_decl->structs[index])) {
            return 0;
        }
    }

    for (size_t index = 0; index < module_decl->enum_count; ++index) {
        if (!zir_printer_emit_enum(printer, &module_decl->enums[index])) {
            return 0;
        }
    }

    for (size_t index = 0; index < module_decl->function_count; ++index) {
        if (!zir_printer_emit_function(printer, &module_decl->functions[index])) {
            return 0;
        }
    }

    return 1;
}

