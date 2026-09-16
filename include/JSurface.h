#pragma once
#include "common.h"
#include <d3d.h>

class CD3DApplication;
class JBaseList;

// PDB type 0x14E5
class JSurface {
    friend struct LayoutCheck;

    JSurface(const JSurface&);
    JSurface();
public:
    JSurface(CD3DApplication* _app, unsigned short _resId, int _useColorKey);
    virtual ~JSurface(); // vtable 0x00
    unsigned short GetResID() const;
    IDirectDrawSurface7* GetDDSurface() const;
    int GetWidth() const;
    int GetHeight() const;
    int UseColorKey() const;
    void SetColorKey(unsigned long _colorKey);
    int Restore();
    void Invalidate();
protected:
    void _Create();
private:
    CD3DApplication* app; // 0x04
    unsigned short resId; // 0x08
    IDirectDrawSurface7* lpSurface; // 0x0C
    int width; // 0x10
    int height; // 0x14
    int useColorKey; // 0x18
    unsigned long colorKey; // 0x1C
    const JSurface& operator=(const JSurface&);
};
ASSERT_SIZE(JSurface, 0x20);

// PDB type 0x1676
class JSurfaceList {
    friend struct LayoutCheck;

    JSurfaceList(const JSurfaceList&);
public:
    JSurfaceList(CD3DApplication* _app);
    virtual ~JSurfaceList(); // vtable 0x00
    int CreateSurface(unsigned short resId, int useColorKey);
    void DestroySurface(unsigned short resId);
    void DestroyAllSurfaces();
    JSurface* GetByResId(unsigned short resId) const;
    JSurface* GetByIndex(unsigned long index) const;
    int RestoreAllSurfaces();
    void InvalidateAllSurfaces();
protected:
    int FindIndexOfResId(unsigned short resId) const;
private:
    CD3DApplication* app; // 0x04
    JBaseList* list; // 0x08
    const JSurfaceList& operator=(const JSurfaceList&);
};
ASSERT_SIZE(JSurfaceList, 0xC);

int FillBufferRect(IDirectDrawSurface7* lpSurface, RECT* lpRect, unsigned long color);
