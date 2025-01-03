// FFstartup.cpp
// License: MIT
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <tchar.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>

void version(void)
{
    std::puts("FFstartup version 0.0 by katahiromz");
}

void usage(void)
{
    std::puts(
        "Usage #1: FFstartup -add ENTRY_NAME CMDLINE...\n"
        "Usage #2: FFstartup -remove ENTRY_NAME\n"
    );
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
    if (argc <= 2)
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
        fprintf(stderr, "ERROR: No action specified.\n");
        return 1;
    }

    m_entry_name = argv[2];
    if (m_entry_name.empty())
    {
        fprintf(stderr, "ERROR: No entry name specified.\n");
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
            fprintf(stderr, "ERROR: Failed to open registry key\n");
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
            fprintf(stderr, "ERROR: Failed to open registry key\n");
            return 1;
        }

        error = RegDeleteValue(hKey, entry_name);
    }

    RegCloseKey(hKey);
    return error ? 1 : 0;
}

#define REG_KEY_RUN TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Run")

INT FFSTARTUP::run(HINSTANCE hInstance, INT nCmdShow)
{
    if (LONG error = set_startup(REG_KEY_RUN, m_entry_name.c_str()))
    {
        fprintf(stderr, "ERROR: Registry operation is failed (error: %ld)\n", error);
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

int main(void)
{
    INT argc;
    LPWSTR *argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    INT ret = FFstartup_main(GetModuleHandle(NULL), argc, argv, SW_SHOWNORMAL);
    LocalFree(argv);
    return ret;
}
