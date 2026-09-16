#pragma once
#include "common.h"

// PDB type 0x1621
class JString {
    friend struct LayoutCheck;
public:
    enum StripType {
        LEADING = 0,
        TRAILING = 1,
        BOTH = 2,
    };

    JString(const JString& i);
    JString(int number);
    JString(const char* cp);
    JString(char c);
    JString();
    const JString& operator=(const JString& i);
    const JString& operator+=(const JString& str);
    const JString& operator+=(char c);
    virtual ~JString(); // vtable 0x00
    const char* c_str() const;
    unsigned int Length() const;
    int FindChar(char c, unsigned int startPos) const;
    int IsEqualTo(const JString& str) const;
    JString ExtractString(unsigned int startPos, unsigned int endPos) const;
    void ReplaceChar(char ch, char newCh);
    void ToUpper();
    void ToLower();
    void Strip(StripType type, char c);
private:
    char* buffer; // 0x04
};
ASSERT_SIZE(JString, 0x8);

JString operator+(const JString& _str1, const JString& _str2);
JString ByteToHex(unsigned char b);
unsigned char HexToByte(const JString& hex);
JString EncryptString(const JString& inputStr, const JString& key);
JString DecryptString(const JString& inputStr, const JString& key);
JString GetJustPath(const JString& fileName);
JString GetCurrentPath();
JString ConvertToFullPath(const JString& fileName, int withQuotes);
void _ByteXOR(const unsigned char* input, unsigned char* output, unsigned int bufferSize, const JString& key);
