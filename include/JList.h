#pragma once
#include "common.h"

// PDB type 0x1131
struct ListItem {
    friend struct LayoutCheck;

    void* data; // 0x00
    ListItem* nextItem; // 0x04
};
ASSERT_SIZE(ListItem, 0x8);

// PDB type 0x14AE
class JBaseList {
    friend struct LayoutCheck;

    JBaseList(const JBaseList&);
public:
    JBaseList();
    virtual ~JBaseList(); // vtable 0x00
    void* GetData(unsigned long index) const;
    unsigned long Count() const;
    bool IsEmpty() const;
    void Append(void* data);
    void InsertBefore(unsigned long index, void* data);
    void Remove(unsigned long index);
    void Empty();
    ListItem* GetFirstItem() const;
protected:
    ListItem* GetLastItem() const;
    ListItem* GetItemAt(unsigned long index) const;
private:
    unsigned long count; // 0x04
    ListItem* firstItem; // 0x08
    const JBaseList& operator=(const JBaseList&);
};
ASSERT_SIZE(JBaseList, 0xC);
