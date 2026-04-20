#ifndef ZT_DRIVER_DOC_SHOW_H
#define ZT_DRIVER_DOC_SHOW_H

/**
 * Exibe a documentação zdoc para o símbolo especificado.
 *
 * @param symbol Símbolo alvo da documentação (ex: "std.math.clamp")
 * @param lang_override Idioma opcional a ser forçado
 * @return 0 se ok, não-zero se houver erro
 */
int zt_handle_doc_show(const char *symbol, const char *lang_override);

#endif
