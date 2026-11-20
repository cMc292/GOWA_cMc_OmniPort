#pragma once

/* Launcher UI text, kept apart from the game's language (system.language).
 * Locale files live in <exe>\locale\<code>.lang and are UTF-8 key=value. */
void i18n_init(const char* selected);
const wchar_t* i18n_tr(const wchar_t* key, const wchar_t* fallback);
const wchar_t* i18n_language(void);
