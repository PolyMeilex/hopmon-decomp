#include "JString.h"
#include <stdlib.h>
#include <string.h>

char HEX_DIGITS[] = "0123456789ABCDEF";

// 0x004138B0, 75 bytes.
JString::JString() : buffer(NULL)
{
    buffer = new char[1];
    buffer[0] = '\0';
}

// 0x00413900, 97 bytes.
JString::JString(char c) : buffer(NULL)
{
    buffer = new char[2];
    buffer[0] = c;
    buffer[1] = '\0';
}

// 0x00413970, 140 bytes.
JString::JString(const char* cp) : buffer(NULL)
{
    if (cp != NULL) {
        buffer = new char[strlen(cp) + 1];
        strcpy(buffer, cp);
    } else {
        buffer = new char[1];
        buffer[0] = '\0';
    }
}

// 0x00413A00, 113 bytes.
JString::JString(int number) : buffer(NULL)
{
    static char tempBuffer[40];
    _ltoa(number, tempBuffer, 10);
    buffer = new char[strlen(tempBuffer) + 1];
    strcpy(buffer, tempBuffer);
}

// 0x00413A80, 47 bytes.
JString::JString(const JString& i) : buffer(NULL)
{
    *this = i;
}

// 0x00413AB0, 128 bytes.
const JString& JString::operator=(const JString& i)
{
    if (buffer != NULL) {
        delete[] buffer;
    }
    buffer = NULL;
    if (i.buffer != NULL) {
        buffer = new char[strlen(i.buffer) + 1];
        strcpy(buffer, i.buffer);
    }
    return *this;
}

// 0x00413B30, 158 bytes.
const JString& JString::operator+=(char c)
{
    unsigned int newLength = strlen(buffer) + 1;
    char* newBuffer = new char[newLength + 1];
    strncpy(newBuffer, buffer, strlen(buffer));
    newBuffer[newLength - 1] = c;
    newBuffer[newLength] = '\0';
    if (buffer != NULL) {
        delete[] buffer;
    }
    buffer = newBuffer;
    return *this;
}

// 0x00413BD0, 214 bytes.
const JString& JString::operator+=(const JString& str)
{
    unsigned int newLength = strlen(buffer) + strlen(str.buffer);
    char* newBuffer = new char[newLength + 1];
    strncpy(newBuffer, buffer, strlen(buffer));
    strncpy(newBuffer + strlen(buffer), str.buffer, strlen(str.buffer));
    newBuffer[newLength] = '\0';
    if (buffer != NULL) {
        delete[] buffer;
    }
    buffer = newBuffer;
    return *this;
}

// 0x00413CB0, 62 bytes.
JString::~JString()
{
    if (buffer != NULL) {
        delete[] buffer;
    }
    buffer = NULL;
}

// 0x00413CF0, 17 bytes.
const char* JString::c_str() const
{
    return buffer;
}

// 0x00413D10, 26 bytes.
unsigned int JString::Length() const
{
    return strlen(buffer);
}

// 0x00413D30, 86 bytes.
int JString::FindChar(char c, unsigned int startPos) const
{
    if (startPos >= strlen(buffer)) {
        return -1;
    }
    char* ptr = strchr(buffer + startPos, c);
    if (ptr == NULL) {
        return -1;
    }
    return ptr - buffer;
}

// 0x00413D90, 60 bytes.
int JString::IsEqualTo(const JString& str) const
{
    return strcmp(buffer, str.buffer) == 0;
}

// 0x00413DD0, 191 bytes.
JString JString::ExtractString(unsigned int startPos, unsigned int endPos) const
{
    JString result;
    unsigned int length = strlen(buffer);
    unsigned int pos = startPos;
    while (pos < length && pos <= endPos) {
        result += buffer[pos];
        pos++;
    }
    return result;
}

// 0x00413E90, 67 bytes.
void JString::ReplaceChar(char ch, char newCh)
{
    while (1) {
        char* ptr = strchr(buffer, ch);
        if (ptr != NULL) {
            *ptr = newCh;
        } else {
            break;
        }
    }
}

// 0x00413EE0, 26 bytes.
void JString::ToUpper()
{
    _strupr(buffer);
}

// 0x00413F00, 26 bytes.
void JString::ToLower()
{
    _strlwr(buffer);
}

// 0x00413F20, 491 bytes.
void JString::Strip(StripType type, char c)
{
    unsigned int length = strlen(buffer);
    if (length > 0 && (type == LEADING || type == BOTH)) {
        unsigned long currentPos = 0;
        while (currentPos < length && buffer[currentPos] == c) {
            currentPos++;
        }
        if (currentPos > 0 && currentPos < length) {
            JString newStr = ExtractString(currentPos, length - 1);
            *this = newStr;
        } else if (currentPos >= length) {
            JString empty("");
            *this = empty;
        }
    }
    length = strlen(buffer);
    if (length > 0 && (type == TRAILING || type == BOTH)) {
        int currentPos = length - 1;
        while (currentPos >= 0 && buffer[currentPos] == c) {
            currentPos--;
        }
        if (currentPos >= 0 && currentPos < (int)length - 1) {
            JString newStr = ExtractString(0, currentPos);
            *this = newStr;
        } else if (currentPos < 0) {
            JString empty("");
            *this = empty;
        }
    }
}

// 0x00414110, 129 bytes.
JString operator+(const JString& _str1, const JString& _str2)
{
    JString str1 = _str1;
    str1 += _str2;
    return str1;
}

// 0x004141A0, 186 bytes.
JString ByteToHex(unsigned char b)
{
    int firstDigit = b / 16;
    int secondDigit = b % 16;
    JString hex;
    hex += HEX_DIGITS[firstDigit];
    hex += HEX_DIGITS[secondDigit];
    return hex;
}

// 0x00414260, 210 bytes.
unsigned char HexToByte(const JString& hex)
{
    JString hexDigits(HEX_DIGITS);
    int firstDigit = hexDigits.FindChar(hex.c_str()[0], 0);
    int secondDigit = hexDigits.FindChar(hex.c_str()[1], 0);
    if (firstDigit < 0 || secondDigit < 0) {
        return '?';
    }
    return (firstDigit << 4) + secondDigit;
}

// 0x00414340, 39 bytes.
JString EncryptString(const JString& inputStr, const JString& key)
{
    return JString(inputStr);
}

// 0x00414370, 39 bytes.
JString DecryptString(const JString& inputStr, const JString& key)
{
    return JString(inputStr);
}

// 0x004143A0, 209 bytes.
JString GetJustPath(const JString& fileName)
{
    int index = fileName.Length() - 1;
    while (index > 0) {
        if (fileName.c_str()[index] == '\\') {
            break;
        }
        index--;
    }
    if (index > 0) {
        JString path = fileName.ExtractString(0, index);
        return path;
    }
    return JString("\\");
}

// 0x00414480, 195 bytes.
JString GetCurrentPath()
{
    static char buffer[1001];
    if (GetModuleFileNameA(NULL, buffer, 1000) != 0) {
        JString path = GetJustPath(buffer);
        return path;
    }
    return JString("\\");
}

// 0x00414550, 409 bytes.
JString ConvertToFullPath(const JString& fileName, int withQuotes)
{
    JString currentPath = GetCurrentPath();
    JString fullPath;
    if (withQuotes != 0) {
        fullPath = '"' + currentPath + fileName + '"';
    } else {
        fullPath = currentPath + fileName;
    }
    return fullPath;
}

// 0x004146F0, 94 bytes.
void _ByteXOR(const unsigned char* input, unsigned char* output, unsigned int bufferSize, const JString& key)
{
    for (unsigned int i = 0; i < bufferSize; i++) {
        int keyIndex = i % key.Length();
        output[i] = input[i] ^ key.c_str()[keyIndex];
    }
}
