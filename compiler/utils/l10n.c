#include "compiler/utils/l10n.h"
#include <stdlib.h>
#include <string.h>

static zt_lang current_zt_lang = ZT_LANG_UNSPECIFIED;

void zt_l10n_set_lang(zt_lang lang) {
    current_zt_lang = lang;
}

int zt_l10n_is_explicitly_set(void) {
    return current_zt_lang != ZT_LANG_UNSPECIFIED;
}

zt_lang zt_l10n_from_str(const char *str) {
    if (str == NULL) return ZT_LANG_UNSPECIFIED;
    if (strcmp(str, "en") == 0) return ZT_LANG_EN;
    if (strcmp(str, "pt") == 0 || strcmp(str, "pt_BR") == 0) return ZT_LANG_PT;
    if (strcmp(str, "es") == 0) return ZT_LANG_ES;
    if (strcmp(str, "ja") == 0 || strcmp(str, "jp") == 0) return ZT_LANG_JA;
    return ZT_LANG_UNSPECIFIED;
}

zt_lang zt_l10n_current_lang(void) {
    if (current_zt_lang != ZT_LANG_UNSPECIFIED) {
        return current_zt_lang;
    }

    const char *env = getenv("ZT_LANG");
    if (env != NULL) {
        zt_lang lang = zt_l10n_from_str(env);
        if (lang != ZT_LANG_UNSPECIFIED) return lang;
    }
    return ZT_LANG_EN;
}

const char *zt_l10n_label_where(void) {
    zt_lang lang = zt_l10n_current_lang();
    if (lang == ZT_LANG_PT) return "onde";
    if (lang == ZT_LANG_ES) return "donde";
    if (lang == ZT_LANG_JA) return "場所";
    return "where";
}

const char *zt_l10n_label_code(void) {
    zt_lang lang = zt_l10n_current_lang();
    if (lang == ZT_LANG_PT) return "código";
    if (lang == ZT_LANG_ES) return "código";
    if (lang == ZT_LANG_JA) return "コード";
    return "code";
}

const char *zt_l10n_label_note(void) {
    zt_lang lang = zt_l10n_current_lang();
    if (lang == ZT_LANG_PT) return "nota";
    if (lang == ZT_LANG_ES) return "nota";
    if (lang == ZT_LANG_JA) return "注意";
    return "note";
}

const char *zt_l10n_label_help(void) {
    zt_lang lang = zt_l10n_current_lang();
    if (lang == ZT_LANG_PT) return "ajuda";
    if (lang == ZT_LANG_ES) return "ayuda";
    if (lang == ZT_LANG_JA) return "ヘルプ";
    return "help";
}

const char *zt_l10n_severity_name(zt_diag_severity severity) {
    zt_lang lang = zt_l10n_current_lang();
    switch (severity) {
        case ZT_DIAG_SEVERITY_ERROR:
            if (lang == ZT_LANG_PT) return "erro";
            if (lang == ZT_LANG_ES) return "error";
            if (lang == ZT_LANG_JA) return "エラー";
            return "error";
        case ZT_DIAG_SEVERITY_WARNING:
            if (lang == ZT_LANG_PT) return "aviso";
            if (lang == ZT_LANG_ES) return "advertencia";
            if (lang == ZT_LANG_JA) return "警告";
            return "warning";
        case ZT_DIAG_SEVERITY_NOTE:
            if (lang == ZT_LANG_PT) return "nota";
            if (lang == ZT_LANG_ES) return "nota";
            if (lang == ZT_LANG_JA) return "注意";
            return "note";
        case ZT_DIAG_SEVERITY_HELP:
            if (lang == ZT_LANG_PT) return "ajuda";
            if (lang == ZT_LANG_ES) return "ayuda";
            if (lang == ZT_LANG_JA) return "ヘルプ";
            return "help";
        default: return "error";
    }
}

const char *zt_l10n_default_help(zt_diag_code code) {
    zt_lang lang = zt_l10n_current_lang();
    switch (code) {
        case ZT_DIAG_TYPE_MISMATCH:
            if (lang == ZT_LANG_PT) return "Verifique se os tipos são compatíveis ou use uma conversão explícita.";
            if (lang == ZT_LANG_ES) return "Asegúrate de que los tipos sean compatibles o usa una conversión explícita.";
            if (lang == ZT_LANG_JA) return "型の互換性を確認するか、明示的な型変換を使用してください。";
            return "Ensure types are compatible or use an explicit conversion.";
        case ZT_DIAG_NON_EXHAUSTIVE_MATCH:
            if (lang == ZT_LANG_PT) return "Adicione as variantes que faltam ou um caso 'default ->'.";
            if (lang == ZT_LANG_ES) return "Añade las variantes que faltan o un caso 'default ->'.";
            if (lang == ZT_LANG_JA) return "不足しているバリアントを追加するか、'default ->' ケースを追加してください。";
            return "Add missing variants or a 'default ->' case.";
        case ZT_DIAG_UNRESOLVED_NAME:
            if (lang == ZT_LANG_PT) return "Declare ou importe este nome antes de usá-lo.";
            if (lang == ZT_LANG_ES) return "Declara o importa este nombre antes de usarlo.";
            if (lang == ZT_LANG_JA) return "使用する前にこの名前を宣言またはインポートしてください。";
            return "Declare or import this name before using it.";
        case ZT_DIAG_SYNTAX_ERROR:
            if (lang == ZT_LANG_PT) return "Revise a sintaxe próximo ao erro.";
            if (lang == ZT_LANG_ES) return "Revisa la sintaxis cerca del error.";
            if (lang == ZT_LANG_JA) return "エラー周辺の構文を確認してください。";
            return "Review the syntax near the error.";
        default:
            if (lang == ZT_LANG_PT) return "Verifique o código fonte e as regras da linguagem.";
            if (lang == ZT_LANG_ES) return "Verifica el código fuente y las reglas del lenguaje.";
            if (lang == ZT_LANG_JA) return "ソースコードと言語規則を確認してください。";
            return "Check the source code and language rules.";
    }
}
