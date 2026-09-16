#include "JSurface.h"
#include "JList.h"
#include "D3DApp.h"
#include "ddutil.h"

// 0x00414E90, 102 bytes.
JSurface::JSurface(CD3DApplication* _app, unsigned short _resId, int _useColorKey)
    : app(_app), resId(_resId), lpSurface(NULL), width(-1), height(-1),
      useColorKey(_useColorKey), colorKey(0xFFFFFFFF)
{
    _Create();
}

// 0x00414F00, 106 bytes.
JSurface::~JSurface()
{
    if (lpSurface != NULL) {
        lpSurface->Release();
        lpSurface = NULL;
    }
}

// 0x00414F70, 18 bytes.
unsigned short JSurface::GetResID() const
{
    return resId;
}

// 0x00414F90, 17 bytes.
IDirectDrawSurface7* JSurface::GetDDSurface() const
{
    return lpSurface;
}

// 0x00414FB0, 17 bytes.
int JSurface::GetWidth() const
{
    return width;
}

// 0x00414FD0, 17 bytes.
int JSurface::GetHeight() const
{
    return height;
}

// 0x00414FF0, 17 bytes.
int JSurface::UseColorKey() const
{
    return useColorKey;
}

// 0x00415010, 53 bytes.
void JSurface::SetColorKey(unsigned long _colorKey)
{
    colorKey = _colorKey;
    if (useColorKey) {
        DDSetColorKey(lpSurface, colorKey);
    }
}

// 0x00415050, 88 bytes.
int JSurface::Restore()
{
    if (lpSurface != NULL) {
        lpSurface->Release();
        lpSurface = NULL;
    }
    _Create();
    int res;
    if (lpSurface != NULL) {
        res = 1;
    } else {
        res = 0;
    }
    return res;
}

// 0x004150B0, 50 bytes.
void JSurface::Invalidate()
{
    if (lpSurface != NULL) {
        lpSurface->Release();
        lpSurface = NULL;
    }
}

// 0x004150F0, 90 bytes.
void JSurface::_Create()
{
    lpSurface = DDLoadBitmap(app->GetDDraw(), (LPCSTR)(DWORD)resId, width, height);
    if (useColorKey) {
        DDSetColorKey(lpSurface, colorKey);
    }
}

// 0x00415150, 146 bytes.
JSurfaceList::JSurfaceList(CD3DApplication* _app)
    : app(_app), list(new JBaseList())
{
}

// 0x004151F0, 137 bytes.
JSurfaceList::~JSurfaceList()
{
    DestroyAllSurfaces();
    if (list != NULL) {
        delete list;
        list = NULL;
    }
}

// 0x00415280, 190 bytes.
int JSurfaceList::CreateSurface(unsigned short resId, int useColorKey)
{
    int index = FindIndexOfResId(resId);
    JSurface* surface = new JSurface(app, resId, useColorKey);
    if (surface->GetDDSurface() == NULL) {
        return 0;
    }
    list->Append(surface);
    return 1;
}

// 0x00415340, 121 bytes.
void JSurfaceList::DestroySurface(unsigned short resId)
{
    int index = FindIndexOfResId(resId);
    if (index >= 0) {
        JSurface* surface = (JSurface*)list->GetData(index);
        if (surface != NULL) {
            delete surface;
            surface = NULL;
        }
        list->Remove(index);
    }
}

// 0x004153C0, 129 bytes.
void JSurfaceList::DestroyAllSurfaces()
{
    for (unsigned long i = 0; i < list->Count(); i++) {
        JSurface* surface = (JSurface*)list->GetData(i);
        if (surface != NULL) {
            delete surface;
            surface = NULL;
        }
    }
    list->Empty();
}

// 0x00415450, 80 bytes.
JSurface* JSurfaceList::GetByResId(unsigned short resId) const
{
    ListItem* item = list->GetFirstItem();
    while (item != NULL) {
        JSurface* surface = (JSurface*)item->data;
        if (surface->GetResID() == resId) {
            return surface;
        }
        item = item->nextItem;
    }
    return NULL;
}

// 0x004154A0, 48 bytes.
JSurface* JSurfaceList::GetByIndex(unsigned long index) const
{
    if (index >= list->Count()) {
        return NULL;
    }
    return (JSurface*)list->GetData(index);
}

// 0x004154D0, 98 bytes.
int JSurfaceList::RestoreAllSurfaces()
{
    int result = 1;
    for (unsigned long i = 0; i < list->Count(); i++) {
        JSurface* surface = (JSurface*)list->GetData(i);
        if (!surface->Restore()) {
            result = 0;
            break;
        }
    }
    return result;
}

// 0x00415540, 75 bytes.
void JSurfaceList::InvalidateAllSurfaces()
{
    for (unsigned long i = 0; i < list->Count(); i++) {
        JSurface* surface = (JSurface*)list->GetData(i);
        surface->Invalidate();
    }
}

// 0x00415590, 97 bytes.
int JSurfaceList::FindIndexOfResId(unsigned short resId) const
{
    ListItem* item = list->GetFirstItem();
    int index = 0;
    while (item != NULL) {
        JSurface* surface = (JSurface*)item->data;
        if (surface->GetResID() == resId) {
            return index;
        }
        index++;
        item = item->nextItem;
    }
    return -1;
}

// 0x00415600, 95 bytes.
int FillBufferRect(IDirectDrawSurface7* lpSurface, RECT* lpRect, unsigned long color)
{
    int res;
    HRESULT hr;
    DDBLTFX ddbltfx;
    ddbltfx.dwSize = sizeof(DDBLTFX);
    ddbltfx.dwFillColor = DDColorMatch(lpSurface, color);
    hr = lpSurface->Blt(lpRect, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx);
    if (hr == DD_OK) {
        res = 1;
    } else {
        res = 0;
    }
    return res;
}

