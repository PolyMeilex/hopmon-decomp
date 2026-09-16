#include "JStringList.h"
#include "JString.h"
#include <stdio.h>

// 0x004147A0, 34 bytes.
JStringList::JStringList()
{
}

// 0x004147D0, 86 bytes.
JStringList::~JStringList()
{
    Empty();
}

// 0x00414830, 22 bytes.
unsigned long JStringList::Count() const
{
    return list.Count();
}

// 0x00414850, 22 bytes.
bool JStringList::IsEmpty() const
{
    return list.IsEmpty();
}

// 0x00414870, 141 bytes.
void JStringList::Append(const JString& str)
{
    JString* item = new JString(str);
    list.Append(item);
}

// 0x00414900, 92 bytes.
void JStringList::Remove(unsigned long index)
{
    JString* str = (JString*)list.GetData(index);
    if (str != NULL) {
        delete str;
    }
    list.Remove(index);
}

// 0x00414960, 122 bytes.
void JStringList::Empty()
{
    for (unsigned long i = 0; i < list.Count(); i++) {
        JString* str = (JString*)list.GetData(i);
        if (str != NULL) {
            delete str;
        }
    }
    list.Empty();
}

// 0x004149E0, 36 bytes.
const JString& JStringList::GetString(unsigned long index) const
{
    JString* str = (JString*)list.GetData(index);
    return *str;
}

// 0x00414A10, 394 bytes.
int GetStringListFromResource(JStringList& list, int resId)
{
    HRSRC resHandle = FindResourceA(NULL, MAKEINTRESOURCEA(resId), RT_RCDATA);
    if (resHandle == NULL) {
        return 0;
    }
    void* memHandle = LoadResource(NULL, resHandle);
    if (memHandle == NULL) {
        return 0;
    }
    void* pointer = LockResource(memHandle);
    if (pointer == NULL) {
        return 0;
    }

    list.Empty();

    JString allLines((const char*)pointer);
    unsigned currentPos = 0;
    while (currentPos < allLines.Length()) {
        int newLinePos = allLines.FindChar('\n', currentPos);
        if (newLinePos >= 0) {
            JString line = allLines.ExtractString(currentPos, newLinePos);
            line.ReplaceChar('\r', '\n');
            line.Strip(JString::TRAILING, '\n');
            list.Append(line);
            currentPos = newLinePos + 1;
        } else {
            JString line = allLines.ExtractString(currentPos, allLines.Length() - 1);
            list.Append(line);
            break;
        }
    }
    return 1;
}

// 0x00414BA0, 414 bytes.
int GetStringListFromFile(JStringList& list, const JString& file)
{
    static char buffer[1000];

    FILE* fp = fopen(file.c_str(), "r");
    if (fp == NULL) {
        return 0;
    }

    list.Empty();
    int done = 0;
    JString current("");
    while (done == 0) {
        char* ret = fgets(buffer, 1000, fp);
        if (ret != NULL) {
            current = JString(buffer);
            if (current.Length() > 0) {
                if (current.c_str()[current.Length() - 1] == '\n') {
                    current.ReplaceChar('\r', ' ');
                    current.ReplaceChar('\n', ' ');
                    list.Append(current);
                    current = "";
                }
            }
        } else {
            if (current.Length() > 0) {
                list.Append(current);
            }
            done = 1;
        }
    }
    fclose(fp);
    return 1;
}

// 0x00414D40, 132 bytes.
int PutStringListToFile(const JStringList& list, const JString& file)
{
    FILE* fp = fopen(file.c_str(), "w");
    if (fp == NULL) {
        return 0;
    }
    for (unsigned i = 0; i < list.Count(); i++) {
        fprintf(fp, "%s\n", list.GetString(i).c_str());
    }
    fclose(fp);
    return 1;
}

// 0x00414DD0, 7 bytes.
int GetStringListFromBinaryFile(JStringList& list, const JString& file)
{
    return 0;
}

// 0x00414DE0, 7 bytes.
int PutStringListToBinaryFile(const JStringList& list, const JString& file)
{
    return 0;
}
