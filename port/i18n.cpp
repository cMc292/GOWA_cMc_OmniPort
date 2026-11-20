#include "i18n.h"

#include <windows.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <unordered_map>

namespace {

std::unordered_map<std::wstring, std::wstring> g_text;
std::wstring g_language = L"en";

std::wstring exe_dir()
{
    wchar_t p[MAX_PATH];
    const DWORD n = GetModuleFileNameW(nullptr, p, MAX_PATH);
    if (!n || n >= MAX_PATH) return L".";
    wchar_t* s = wcsrchr(p, L'\\');
    if (s) *s = 0;
    return p;
}

std::wstring trim(const std::wstring& s)
{
    size_t a = 0, b = s.size();
    while (a < b && (s[a] == L' ' || s[a] == L'\t' || s[a] == L'\r')) ++a;
    while (b > a && (s[b - 1] == L' ' || s[b - 1] == L'\t' || s[b - 1] == L'\r')) --b;
    return s.substr(a, b - a);
}

std::wstring wide_utf8(const std::string& s)
{
    if (s.empty()) return {};
    const int n = MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0);
    if (n <= 0) return {};
    std::wstring w((size_t)n, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), w.data(), n);
    return w;
}

void load_file(const std::wstring& path, bool overwrite)
{
    FILE* f = _wfopen(path.c_str(), L"rb");
    if (!f) return;
    std::string line;
    char buf[4096];
    while (fgets(buf, sizeof buf, f)) {
        line.assign(buf);
        if (line.size() >= 3 && (unsigned char)line[0] == 0xEF &&
            (unsigned char)line[1] == 0xBB && (unsigned char)line[2] == 0xBF)
            line.erase(0, 3);
        while (!line.empty() && (line.back() == '\n' || line.back() == '\r')) line.pop_back();
        if (line.empty() || line[0] == '#') continue;
        const size_t eq = line.find('=');
        if (eq == std::string::npos) continue;
        const std::wstring key = trim(wide_utf8(line.substr(0, eq)));
        const std::wstring val = trim(wide_utf8(line.substr(eq + 1)));
        if (key.empty()) continue;
        if (overwrite || !g_text.count(key)) g_text[key] = val;
    }
    fclose(f);
}

std::wstring automatic_language()
{
    const LANGID id = GetUserDefaultUILanguage();
    switch (PRIMARYLANGID(id)) {
    case LANG_SPANISH: return L"es";
    case LANG_PORTUGUESE: return L"pt-BR";
    case LANG_FRENCH: return L"fr";
    case LANG_GERMAN: return L"de";
    case LANG_ITALIAN: return L"it";
    default: return L"en";
    }
}

}  // namespace

void i18n_init(const char* selected)
{
    g_text.clear();
    std::wstring lang = L"en";

    const char* env = getenv("GOWA_LAUNCHER_LANGUAGE");
    const char* choice = (env && *env) ? env : selected;
    if (choice && *choice && strcmp(choice, "auto")) {
        lang = wide_utf8(choice);
    } else {
        lang = automatic_language();
    }
    g_language = lang;

    const std::wstring dir = exe_dir() + L"\\locale\\";
    // English is the fallback for every missing key; the chosen file overrides it.
    load_file(dir + L"en.lang", true);
    if (_wcsicmp(lang.c_str(), L"en") && _wcsicmp(lang.c_str(), L"en-gb"))
        load_file(dir + lang + L".lang", true);
}

const wchar_t* i18n_tr(const wchar_t* key, const wchar_t* fallback)
{
    const auto it = g_text.find(key);
    return it == g_text.end() || it->second.empty() ? fallback : it->second.c_str();
}

const wchar_t* i18n_language(void)
{
    return g_language.c_str();
}
