#ifndef ZENITH_NEXT_COMPILER_UTILS_L10N_H
#define ZENITH_NEXT_COMPILER_UTILS_L10N_H

#include "compiler/utils/diagnostics.h"

typedef enum zt_lang {
    ZT_LANG_UNSPECIFIED,
    ZT_LANG_EN,
    ZT_LANG_PT,
    ZT_LANG_ES,
    ZT_LANG_JA
} zt_lang;

zt_lang zt_l10n_current_lang(void);
void zt_l10n_set_lang(zt_lang lang);
int zt_l10n_is_explicitly_set(void);
zt_lang zt_l10n_from_str(const char *str);
const char *zt_l10n_label_where(void);
const char *zt_l10n_label_code(void);
const char *zt_l10n_label_note(void);
const char *zt_l10n_label_help(void);
const char *zt_l10n_severity_name(zt_diag_severity severity);
const char *zt_l10n_default_help(zt_diag_code code);

#endif
