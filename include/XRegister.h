#pragma once
#include "common.h"

// PDB type 0x1D50
struct JREGISTERINFO {
    friend struct LayoutCheck;

    char regKey[40]; // 0x00
    int registered; // 0x28
};
ASSERT_SIZE(JREGISTERINFO, 0x2C);

void LoadRegisterInfo(const char* winRegKey);
void SaveRegisterInfo(const char* winRegKey);
void DeleteRegisterInfo();
void DoRegister(HWND parent);
int IsRegistered();
HRESULT _ExecuteRgisterDialog(HWND parent, JREGISTERINFO* pRegInfo);
void _CheckRegistrationKey();
