#include "XRegister.h"

static JREGISTERINFO* g_pRegInfo;

int CALLBACK _ChangeRegisterProc(HWND hDlg, UINT uiMsg, WPARAM wParam, LPARAM lParam);

// 0x00428700, 96 bytes.
void LoadRegisterInfo(const char* winRegKey)
{
    if (g_pRegInfo != NULL) {
        delete g_pRegInfo;
        g_pRegInfo = NULL;
    }
    g_pRegInfo = new JREGISTERINFO;
    memset(g_pRegInfo, 0, sizeof(JREGISTERINFO));
    _CheckRegistrationKey();
}

// 0x00428760, 5 bytes.
void SaveRegisterInfo(const char* winRegKey)
{
}

// 0x00428770, 49 bytes.
void DeleteRegisterInfo()
{
    if (g_pRegInfo != NULL) {
        delete g_pRegInfo;
        g_pRegInfo = NULL;
    }
}

// 0x004287B0, 74 bytes.
void DoRegister(HWND parent)
{
    HRESULT result = _ExecuteRgisterDialog(parent, g_pRegInfo);
    if (result >= 0) {
        _CheckRegistrationKey();
        if (g_pRegInfo->registered != 0) {
            MessageBoxA(parent, "Thank you! You have successfully registered Hopmon.", "Hopmon", MB_ICONINFORMATION);
        }
    }
}

// 0x00428800, 10 bytes.
int IsRegistered()
{
    return 1;
}

// 0x00428810, 318 bytes.
int CALLBACK _ChangeRegisterProc(HWND hDlg, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    static JREGISTERINFO* pRegInfo;

    if (uiMsg == WM_INITDIALOG) {
        pRegInfo = (JREGISTERINFO*)lParam;
        if (pRegInfo == NULL) {
            return 0;
        }
        HWND hwndReg = GetDlgItem(hDlg, 0x400);
        HWND hwndOK = GetDlgItem(hDlg, IDOK);
        SendMessageA(hwndReg, EM_LIMITTEXT, 0x1e, 0);
        SetWindowTextA(hwndReg, pRegInfo->regKey);
        EnableWindow(hwndReg, pRegInfo->registered == 0);
        EnableWindow(hwndOK, pRegInfo->registered == 0);
        return 1;
    }
    else if (uiMsg == WM_COMMAND) {
        if (LOWORD(wParam) == IDOK) {
            HWND hwndReg = GetDlgItem(hDlg, 0x400);
            GetWindowTextA(hwndReg, pRegInfo->regKey, 0x27);
            EndDialog(hDlg, IDOK);
        }
        else if (LOWORD(wParam) == IDCANCEL) {
            EndDialog(hDlg, IDCANCEL);
        }
        return 1;
    }
    return 0;
}

// 0x00428960, 59 bytes.
HRESULT _ExecuteRgisterDialog(HWND parent, JREGISTERINFO* pRegInfo)
{
    int res = DialogBoxParamA(GetModuleHandleA(NULL), MAKEINTRESOURCE(0xf9), parent, _ChangeRegisterProc, (LPARAM)pRegInfo);
    if (res == 1) {
        return S_OK;
    }
    return E_FAIL;
}

// 0x004289A0, 26 bytes.
void _CheckRegistrationKey()
{
    if (g_pRegInfo != NULL) {
        g_pRegInfo->registered = 1;
    }
}
