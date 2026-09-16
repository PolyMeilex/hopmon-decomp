#include "JList.h"

// 0x0040EB90, 43 bytes.
JBaseList::JBaseList()
    : count(0), firstItem(NULL)
{
}

// 0x0040EBC0, 75 bytes.
JBaseList::~JBaseList()
{
    Empty();
}

// 0x0040EC10, 35 bytes.
void* JBaseList::GetData(unsigned long index) const
{
    ListItem* item = GetItemAt(index);
    return item->data;
}

// 0x0040EC40, 17 bytes.
unsigned long JBaseList::Count() const
{
    return count;
}

// 0x0040EC60, 41 bytes.
bool JBaseList::IsEmpty() const
{
    return count == 0;
}

// 0x0040EC90, 107 bytes.
void JBaseList::Append(void* data)
{
    ListItem* item = new ListItem;
    item->data = data;
    item->nextItem = NULL;
    if (count == 0) {
        firstItem = item;
    } else {
        ListItem* last = GetLastItem();
        last->nextItem = item;
    }
    count++;
}

// 0x0040ED00, 119 bytes.
void JBaseList::InsertBefore(unsigned long index, void* data)
{
    ListItem* item = new ListItem;
    item->data = data;
    item->nextItem = GetItemAt(index);
    if (index == 0) {
        firstItem = item;
    } else {
        ListItem* prev = GetItemAt(index - 1);
        prev->nextItem = item;
    }
    count++;
}

// 0x0040ED80, 118 bytes.
void JBaseList::Remove(unsigned long index)
{
    ListItem* item = GetItemAt(index);
    ListItem* next = item->nextItem;
    if (index == 0) {
        firstItem = next;
    } else {
        ListItem* prev = GetItemAt(index - 1);
        prev->nextItem = next;
    }
    delete item;
    count--;
}

// 0x0040EE00, 85 bytes.
void JBaseList::Empty()
{
    ListItem* item = firstItem;
    while (item != NULL) {
        ListItem* next = item->nextItem;
        delete item;
        item = next;
    }
    firstItem = NULL;
    count = 0;
}

// 0x0040EE60, 17 bytes.
ListItem* JBaseList::GetFirstItem() const
{
    return firstItem;
}

// 0x0040EE80, 58 bytes.
ListItem* JBaseList::GetLastItem() const
{
    if (count == 0) {
        return NULL;
    }
    ListItem* item = firstItem;
    while (item->nextItem != NULL) {
        item = item->nextItem;
    }
    return item;
}

// 0x0040EEC0, 62 bytes.
ListItem* JBaseList::GetItemAt(unsigned long index) const
{
    unsigned long i = 0;
    ListItem* item = firstItem;
    while (i != index) {
        item = item->nextItem;
        i++;
    }
    return item;
}
