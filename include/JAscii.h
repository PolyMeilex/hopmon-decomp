#pragma once
#include "common.h"
#include <d3d.h>

class JSprite;
class JSurfaceList;

// PDB type 0x14BE
class JNumber {
    friend struct LayoutCheck;

    JNumber(const JNumber&);
public:
    JNumber(JSurfaceList* surfaceList, unsigned short resId, int numFrames);
    virtual ~JNumber(); // vtable 0x00
    void SetPosition(int x, int y);
    void SetDisplayZero(int display);
    void SetNumDigits(int _numDigits);
    void SetNumber(int _number);
    void Draw(IDirectDrawSurface7* lpSurface);
    int GetPosX() const;
    int GetPosY() const;
    int GetDigitWidth() const;
    int GetWidth() const;
    int GetHeight() const;
private:
    JSprite* numberSprite; // 0x04
    int posX; // 0x08
    int posY; // 0x0C
    int displayZero; // 0x10
    int numDigits; // 0x14
    int number; // 0x18
    const JNumber& operator=(const JNumber&);
};
ASSERT_SIZE(JNumber, 0x1C);
