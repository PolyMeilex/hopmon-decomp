#include "JWinReg.h"
#include "JString.h"

// 0x004156C0, 33 bytes.
JWinReg::JWinReg()
    : currentKey(NULL)
{
}

// 0x004156F0, 75 bytes.
JWinReg::~JWinReg()
{
    Close();
}

// 0x00415740, 92 bytes.
int JWinReg::Create(HKEY hKey, const JString& keyName)
{
    Close();
    long res = RegCreateKeyExA(hKey, keyName.c_str(), 0, NULL, 0, KEY_ALL_ACCESS, NULL, &currentKey, NULL);
    return res == 0;
}

// 0x004157A0, 84 bytes.
int JWinReg::Open(HKEY hKey, const JString& keyName)
{
    Close();
    long res = RegOpenKeyExA(hKey, keyName.c_str(), 0, KEY_ALL_ACCESS, &currentKey);
    return res == 0;
}

// 0x00415800, 77 bytes.
int JWinReg::Close()
{
    if (currentKey != NULL) {
        long res = RegCloseKey(currentKey);
        currentKey = NULL;
        return res == 0;
    }
    return 0;
}

// 0x00415850, 103 bytes.
int JWinReg::SetString(const JString& name, const JString& value)
{
    if (currentKey == NULL) {
        return 0;
    }
    long res = RegSetValueExA(currentKey, name.c_str(), 0, REG_SZ, (const BYTE*)value.c_str(), value.Length() + 1);
    return res == 0;
}

// 0x004158C0, 302 bytes.
int JWinReg::GetString(const JString& name, JString& value)
{
    if (currentKey == NULL) {
        return 0;
    }
    static char buffer[256];
    memset(buffer, 0, sizeof(buffer));
    DWORD dwSize = 0;
    DWORD dwType = 0;
    long res = RegQueryValueExA(currentKey, name.c_str(), NULL, &dwType, NULL, &dwSize);
    if (res != 0) {
        return 0;
    }
    if (dwType != REG_SZ) {
        return 0;
    }
    if (dwSize >= sizeof(buffer)) {
        return 0;
    }
    res = RegQueryValueExA(currentKey, name.c_str(), NULL, NULL, (LPBYTE)buffer, &dwSize);
    value = JString(buffer);
    return res == 0;
}

// 0x004159F0, 90 bytes.
int JWinReg::SetBinary(const JString& name, const unsigned char* data, unsigned long size)
{
    if (currentKey == NULL) {
        return 0;
    }
    long res = RegSetValueExA(currentKey, name.c_str(), 0, REG_BINARY, data, size);
    return res == 0;
}

// 0x00415A50, 178 bytes.
int JWinReg::GetBinary(const JString& name, unsigned char* data, unsigned long expectedSize)
{
    if (currentKey == NULL) {
        return 0;
    }
    DWORD dwSize = 0;
    DWORD dwType = 0;
    long res = RegQueryValueExA(currentKey, name.c_str(), NULL, &dwType, NULL, &dwSize);
    if (res != 0) {
        return 0;
    }
    if (dwType != REG_BINARY) {
        return 0;
    }
    if (dwSize != expectedSize) {
        return 0;
    }
    res = RegQueryValueExA(currentKey, name.c_str(), NULL, &dwType, data, &dwSize);
    return res == 0;
}

// 0x00415B10, 88 bytes.
int JWinReg::SetByte(const JString& name, const unsigned char& data)
{
    if (currentKey == NULL) {
        return 0;
    }
    long res = RegSetValueExA(currentKey, name.c_str(), 0, REG_BINARY, &data, 1);
    return res == 0;
}

// 0x00415B70, 174 bytes.
int JWinReg::GetByte(const JString& name, unsigned char& data)
{
    if (currentKey == NULL) {
        return 0;
    }
    DWORD dwSize = 0;
    DWORD dwType = 0;
    long res = RegQueryValueExA(currentKey, name.c_str(), NULL, &dwType, NULL, &dwSize);
    if (res != 0) {
        return 0;
    }
    if (dwType != REG_BINARY) {
        return 0;
    }
    if (dwSize != 1) {
        return 0;
    }
    res = RegQueryValueExA(currentKey, name.c_str(), NULL, NULL, &data, &dwSize);
    return res == 0;
}

// 0x00415C20, 40 bytes.
int JWinReg::SetWord(const JString& name, const unsigned short& data)
{
    unsigned long _data = data;
    return SetDWord(name, _data);
}

// 0x00415C50, 61 bytes.
int JWinReg::GetWord(const JString& name, unsigned short& data)
{
    unsigned long _data = 0;
    if (GetDWord(name, _data)) {
        data = (unsigned short)_data;
        return 1;
    }
    return 0;
}

// 0x00415C90, 88 bytes.
int JWinReg::SetDWord(const JString& name, const unsigned long& data)
{
    if (currentKey == NULL) {
        return 0;
    }
    long res = RegSetValueExA(currentKey, name.c_str(), 0, REG_DWORD, (const BYTE*)&data, 4);
    return res == 0;
}

// 0x00415CF0, 174 bytes.
int JWinReg::GetDWord(const JString& name, unsigned long& data)
{
    if (currentKey == NULL) {
        return 0;
    }
    DWORD dwSize = 0;
    DWORD dwType = 0;
    long res = RegQueryValueExA(currentKey, name.c_str(), NULL, &dwType, NULL, &dwSize);
    if (res != 0) {
        return 0;
    }
    if (dwType != REG_DWORD) {
        return 0;
    }
    if (dwSize != 4) {
        return 0;
    }
    res = RegQueryValueExA(currentKey, name.c_str(), NULL, NULL, (LPBYTE)&data, &dwSize);
    return res == 0;
}

// 0x00415DA0, 78 bytes.
int JWinReg::DeleteValue(const JString& name)
{
    if (currentKey == NULL) {
        return 0;
    }
    long res = RegDeleteValueA(currentKey, name.c_str());
    return res == 0;
}
