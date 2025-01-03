// FFstartup.cpp - Startup entry manipulator for Windows
// License: MIT

// Detect memory leaks (for Debug and MSVC)
#if defined(_MSC_VER) && !defined(NDEBUG) && !defined(_CRTDBG_MAP_ALLOC)
    #define _CRTDBG_MAP_ALLOC
    #include <crtdbg.h>
#endif

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <shlwapi.h>
#include <tchar.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <string>

inline WORD get_lang_id(void)
{
    return PRIMARYLANGID(LANGIDFROMLCID(GetThreadLocale()));
}

// localization
LPCTSTR get_text(INT id)
{
#ifdef JAPAN
    if (get_lang_id() == LANG_JAPANESE) // Japone for Japone
    {
        switch (id)
        {
        case -1: return TEXT("FFstartup バージョン 0.3 by 片山博文MZ");
        case 0:
            return TEXT("使用方法 #1: FFstartup -add エントリ名 コマンドライン...\n")
                   TEXT("使用方法 #2: FFstartup -remove エントリ名\n")
                   TEXT("使用方法 #3: FFstartup -help\n")
                   TEXT("使用方法 #4: FFstartup -version");
        case 1: return TEXT("エラー: アクションが未指定です。\n");
        case 2: return TEXT("エラー: エントリ名が未指定です。\n");
        case 3: return TEXT("エラー: レジストリキーを開くのに失敗しました。\n");
        case 4: return TEXT("エラー: レジストリの処理に失敗しました (エラー: %ld)\n");
        }
    }
    else // The others are Let's la English
#endif
    {
        switch (id)
        {
        case -1: return TEXT("FFstartup version 0.3 by katahiromz");
        case 0:
            return TEXT("Usage #1: FFstartup -add ENTRY_NAME CMDLINE...\n")
                   TEXT("Usage #2: FFstartup -remove ENTRY_NAME\n")
                   TEXT("Usage #3: FFstartup -help\n")
                   TEXT("Usage #4: FFstartup -version");
        case 1: return TEXT("ERROR: No action specified.\n");
        case 2: return TEXT("ERROR: No entry name specified.\n");
        case 3: return TEXT("ERROR: Failed to open registry key\n");
        case 4: return TEXT("ERROR: Registry operation is failed (error: %ld)\n");
        }
    }

    assert(0);
    return nullptr;
}

void version(void)
{
    _putts(get_text(-1));
}

void usage(void)
{
    _putws(get_text(0));
}

struct FFSTARTUP
{
    bool m_add;
    bool m_remove;
    std::wstring m_entry_name;
    std::wstring m_cmdline;

    FFSTARTUP()
    {
        m_add = false;
        m_remove = false;
    }

    INT parse_cmdline(INT argc, LPWSTR *argv);
    INT run(HINSTANCE hInstance, INT nCmdShow);
    INT set_startup(LPCTSTR regkey, LPCTSTR entry_name);
};

INT FFSTARTUP::parse_cmdline(INT argc, LPWSTR *argv)
{
    if (argc <= 1)
    {
        usage();
        return 1;
    }

    std::wstring action = argv[1];

    if (lstrcmpi(action.c_str(), TEXT("-help")) == 0 || lstrcmpi(action.c_str(), TEXT("--help")) == 0)
    {
        usage();
        return 1;
    }

    if (lstrcmpi(action.c_str(), TEXT("-version")) == 0 || lstrcmpi(action.c_str(), TEXT("--version")) == 0)
    {
        version();
        return 1;
    }

    if (lstrcmpi(action.c_str(), TEXT("-add")) == 0 || lstrcmpi(action.c_str(), TEXT("--add")) == 0)
    {
        m_add = true;
    }
    else if (lstrcmpi(action.c_str(), TEXT("-remove")) == 0 || lstrcmpi(action.c_str(), TEXT("--remove")) == 0)
    {
        m_remove = true;
    }

    if (!m_add && !m_remove)
    {
        _ftprintf(stderr, get_text(1));
        return 1;
    }

    if (argc <= 2)
    {
        usage();
        return 1;
    }

    m_entry_name = argv[2];
    if (m_entry_name.empty())
    {
        _ftprintf(stderr, get_text(2));
        return 1;
    }

    std::wstring cmdline;
    for (INT iarg = 3; iarg < argc; ++iarg)
    {
        if (iarg > 3)
            cmdline += L' ';
        std::wstring arg = argv[iarg];
        if (arg.find(' ') == arg.npos && arg.find('\t') == arg.npos)
        {
            cmdline += L"\"";
            cmdline += arg;
            cmdline += L"\"";
        }
        else
        {
            cmdline += arg;
        }
    }

    m_cmdline = cmdline;
    return 0;
}

INT FFSTARTUP::set_startup(LPCTSTR regkey, LPCTSTR entry_name)
{
    HKEY hKey;
    LONG error;
    if (m_add)
    {
        RegCreateKeyEx(HKEY_CURRENT_USER, regkey, 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL);
        if (!hKey)
        {
            _ftprintf(stderr, get_text(3));
            return 1;
        }

        SIZE_T cbData = (m_cmdline.size() + 1) * sizeof(TCHAR);
        error = RegSetValueEx(hKey, entry_name, 0, REG_SZ, (BYTE*)m_cmdline.c_str(), cbData);
    }
    else
    {
        RegOpenKeyEx(HKEY_CURRENT_USER, regkey, 0, KEY_WRITE, &hKey);
        if (!hKey)
        {
            _ftprintf(stderr, get_text(3));
            return 1;
        }

        error = RegDeleteValue(hKey, entry_name);
    }

    RegCloseKey(hKey);

    if (error)
        _ftprintf(stderr, get_text(4), error);

    return error ? 1 : 0;
}

#define REG_KEY_RUN TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Run")

INT FFSTARTUP::run(HINSTANCE hInstance, INT nCmdShow)
{
    if (set_startup(REG_KEY_RUN, m_entry_name.c_str()) == 0)
    {
        return 1;
    }
    return 0;
}

INT FFstartup_main(
    HINSTANCE hInstance,
    INT argc,
    LPWSTR *argv,
    INT nCmdShow)
{
    FFSTARTUP ffstartup;
    INT ret = ffstartup.parse_cmdline(argc, argv);
    if (ret)
        return ret;

    return ffstartup.run(hInstance, nCmdShow);
}

#include <clocale>

int main(void)
{
    // Unicode console output support
    std::setlocale(LC_ALL, "");

    INT argc;
    LPWSTR *argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    INT ret = FFstartup_main(GetModuleHandle(NULL), argc, argv, SW_SHOWNORMAL);
    LocalFree(argv);

    // Detect handle leaks (for Debug)
#if (_WIN32_WINNT >= 0x0500) && !defined(NDEBUG)
    TCHAR szText[MAX_PATH];
    wnsprintf(szText, _countof(szText), TEXT("GDI Objects: %ld, User Objects: %ld\n"),
              GetGuiResources(GetCurrentProcess(), GR_GDIOBJECTS),
              GetGuiResources(GetCurrentProcess(), GR_USEROBJECTS));
    OutputDebugString(szText);
#endif

    // Detect memory leaks (for Debug and MSVC)
#if defined(_MSC_VER) && !defined(NDEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    return ret;
}
