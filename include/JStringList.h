#pragma once
#include "common.h"
#include "JList.h"

class JString;

// PDB type 0x198E
class JStringList {
    friend struct LayoutCheck;

    JStringList(const JStringList&);
public:
    JStringList();
    virtual ~JStringList(); // vtable 0x00
    unsigned long Count() const;
    bool IsEmpty() const;
    void Append(const JString& str);
    void Remove(unsigned long index);
    void Empty();
    const JString& GetString(unsigned long index) const;
private:
    JBaseList list; // 0x04
    const JStringList& operator=(const JStringList&);
};
ASSERT_SIZE(JStringList, 0x10);

int GetStringListFromResource(JStringList& list, int resId);
int GetStringListFromFile(JStringList& list, const JString& file);
int PutStringListToFile(const JStringList& list, const JString& file);
int GetStringListFromBinaryFile(JStringList& list, const JString& file);
int PutStringListToBinaryFile(const JStringList& list, const JString& file);
