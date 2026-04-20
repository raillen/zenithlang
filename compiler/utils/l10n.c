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
    if (lang == ZT_LANG_JA) return "å ´æ‰€";
    return "where";
}

const char *zt_l10n_label_code(void) {
    zt_lang lang = zt_l10n_current_lang();
    if (lang == ZT_LANG_PT) return "cÃ³digo";
    if (lang == ZT_LANG_ES) return "cÃ³digo";
    if (lang == ZT_LANG_JA) return "ã‚³ãƒ¼ãƒ‰";
    return "code";
}

const char *zt_l10n_label_note(void) {
    zt_lang lang = zt_l10n_current_lang();
    if (lang == ZT_LANG_PT) return "nota";
    if (lang == ZT_LANG_ES) return "nota";
    if (lang == ZT_LANG_JA) return "æ³¨æ„";
    return "note";
}

const char *zt_l10n_label_help(void) {
    zt_lang lang = zt_l10n_current_lang();
    if (lang == ZT_LANG_PT) return "ajuda";
    if (lang == ZT_LANG_ES) return "ayuda";
    if (lang == ZT_LANG_JA) return "ãƒ˜ãƒ«ãƒ—";
    return "help";
}

const char *zt_l10n_severity_name(zt_diag_severity severity) {
    zt_lang lang = zt_l10n_current_lang();
    switch (severity) {
        case ZT_DIAG_SEVERITY_ERROR:
            if (lang == ZT_LANG_PT) return "erro";
            if (lang == ZT_LANG_ES) return "error";
            if (lang == ZT_LANG_JA) return "ã‚¨ãƒ©ãƒ¼";
            return "error";
        case ZT_DIAG_SEVERITY_WARNING:
            if (lang == ZT_LANG_PT) return "aviso";
            if (lang == ZT_LANG_ES) return "advertencia";
            if (lang == ZT_LANG_JA) return "è­¦å‘Š";
            return "warning";
        case ZT_DIAG_SEVERITY_NOTE:
            if (lang == ZT_LANG_PT) return "nota";
            if (lang == ZT_LANG_ES) return "nota";
            if (lang == ZT_LANG_JA) return "æ³¨æ„";
            return "note";
        case ZT_DIAG_SEVERITY_HELP:
            if (lang == ZT_LANG_PT) return "ajuda";
            if (lang == ZT_LANG_ES) return "ayuda";
            if (lang == ZT_LANG_JA) return "ãƒ˜ãƒ«ãƒ—";
            return "help";
        default: return "error";
    }
}

const char *zt_l10n_default_help(zt_diag_code code) {
    zt_lang lang = zt_l10n_current_lang();
    switch (code) {
        case ZT_DIAG_TYPE_MISMATCH:
            if (lang == ZT_LANG_PT) return "Verifique se os tipos sÃ£o compatÃ­veis ou use uma conversÃ£o explÃ­cita.";
            if (lang == ZT_LANG_ES) return "AsegÃºrate de que los tipos sean compatibles o usa una conversiÃ³n explÃ­cita.";
            if (lang == ZT_LANG_JA) return "åž‹ã®äº’æ›æ€§ã‚’ç¢ºèªã™ã‚‹ã‹ã€æ˜Žç¤ºçš„ãªåž‹å¤‰æ›ã‚’ä½¿ç”¨ã—ã¦ãã ã•ã„ã€‚";
            return "Ensure types are compatible or use an explicit conversion.";
        case ZT_DIAG_NON_EXHAUSTIVE_MATCH:
            if (lang == ZT_LANG_PT) return "Adicione as variantes que faltam ou um caso 'default ->'.";
            if (lang == ZT_LANG_ES) return "AÃ±ade las variantes que faltan o un caso 'default ->'.";
            if (lang == ZT_LANG_JA) return "ä¸è¶³ã—ã¦ã„ã‚‹ãƒãƒªã‚¢ãƒ³ãƒˆã‚’è¿½åŠ ã™ã‚‹ã‹ã€'default ->' ã‚±ãƒ¼ã‚¹ã‚’è¿½åŠ ã—ã¦ãã ã•ã„ã€‚";
            return "Add missing variants or a 'default ->' case.";
        case ZT_DIAG_UNRESOLVED_NAME:
            if (lang == ZT_LANG_PT) return "Declare ou importe este nome antes de usÃ¡-lo.";
            if (lang == ZT_LANG_ES) return "Declara o importa este nombre antes de usarlo.";
            if (lang == ZT_LANG_JA) return "ä½¿ç”¨ã™ã‚‹å‰ã«ã“ã®åå‰ã‚’å®£è¨€ã¾ãŸã¯ã‚¤ãƒ³ãƒãƒ¼ãƒˆã—ã¦ãã ã•ã„ã€‚";
            return "Declare or import this name before using it.";
        case ZT_DIAG_CONFUSING_NAME:
            if (lang == ZT_LANG_PT) return "Renomeie identificadores que misturam caracteres confundÃ­veis como l/I/1 ou O/0.";
            if (lang == ZT_LANG_ES) return "Renombra identificadores que mezclan caracteres confundibles como l/I/1 u O/0.";
            if (lang == ZT_LANG_JA) return "l/I/1 や O/0 のような紛らわしい文字を混在させる名前は避けてください。";
            return "Rename identifiers that mix confusable characters like l/I/1 or O/0.";
        case ZT_DIAG_SYNTAX_ERROR:
            if (lang == ZT_LANG_PT) return "Revise a sintaxe prÃ³ximo ao erro.";
            if (lang == ZT_LANG_ES) return "Revisa la sintaxis cerca del error.";
            if (lang == ZT_LANG_JA) return "ã‚¨ãƒ©ãƒ¼å‘¨è¾ºã®æ§‹æ–‡ã‚’ç¢ºèªã—ã¦ãã ã•ã„ã€‚";
            return "Review the syntax near the error.";
        default:
            if (lang == ZT_LANG_PT) return "Verifique o cÃ³digo fonte e as regras da linguagem.";
            if (lang == ZT_LANG_ES) return "Verifica el cÃ³digo fuente y las reglas del lenguaje.";
            if (lang == ZT_LANG_JA) return "ã‚½ãƒ¼ã‚¹ã‚³ãƒ¼ãƒ‰ã¨è¨€èªžè¦å‰‡ã‚’ç¢ºèªã—ã¦ãã ã•ã„ã€‚";
            return "Check the source code and language rules.";
    }
}


