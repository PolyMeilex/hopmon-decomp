#pragma once
#include "common.h"
#include <d3d.h>

class JSurfaceList;

// PDB type 0x123F
class JSprite {
    friend struct LayoutCheck;
public:
    JSprite(JSurfaceList* _surfaceList, unsigned short _resId, int _numFrames);
    virtual ~JSprite(); // vtable 0x00
    float GetPosX() const;
    float GetPosY() const;
    int GetPosIntX() const;
    int GetPosIntY() const;
    int GetCenterIntX() const;
    int GetCenterIntY() const;
    int GetWidth() const;
    int GetHeight() const;
    int GetCurrentFrame() const;
    int GetNumFrames() const;
    void SetPosition(int x, int y);
    void SetPosX(int x);
    void SetPosY(int y);
    void ShiftPosX(float dx);
    void ShiftPosY(float dy);
    void SetCenterPos(int x, int y);
    void SetCurrentFrame(int _currentFrame);
    void IncrCurrentFrame();
    void SetDrawSize(int _drawWidth, int _drawHeight);
    void Draw(IDirectDrawSurface7* lpDestSurface);
    void DrawScale(IDirectDrawSurface7* lpDestSurface, float amount);
    void DrawScaleX(IDirectDrawSurface7* lpDestSurface, float amountX);
    void DrawScaleY(IDirectDrawSurface7* lpDestSurface, float amountY);
    void DrawScaleXY(IDirectDrawSurface7* lpDestSurface, float amountX, float amountY);
private:
    JSurfaceList* surfaceList; // 0x04
    unsigned short resId; // 0x08
    float posX; // 0x0C
    float posY; // 0x10
    int width; // 0x14
    int height; // 0x18
    int drawWidth; // 0x1C
    int drawHeight; // 0x20
    int currentFrame; // 0x24
    int numFrames; // 0x28
};
ASSERT_SIZE(JSprite, 0x2C);
