#pragma once
#include "common.h"

class JString;

// PDB type 0x1A40
class JWinReg {
    friend struct LayoutCheck;
public:
    JWinReg();
    virtual ~JWinReg(); // vtable 0x00
    int Create(HKEY hKey, const JString& keyName);
    int Open(HKEY hKey, const JString& keyName);
    int Close();
    int SetString(const JString& name, const JString& value);
    int GetString(const JString& name, JString& value);
    int SetBinary(const JString& name, const unsigned char* data, unsigned long size);
    int GetBinary(const JString& name, unsigned char* data, unsigned long expectedSize);
    int SetByte(const JString& name, const unsigned char& data);
    int GetByte(const JString& name, unsigned char& data);
    int SetWord(const JString& name, const unsigned short& data);
    int GetWord(const JString& name, unsigned short& data);
    int SetDWord(const JString& name, const unsigned long& data);
    int GetDWord(const JString& name, unsigned long& data);
    int DeleteValue(const JString& name);
private:
    HKEY currentKey; // 0x04
};
ASSERT_SIZE(JWinReg, 0x8);
