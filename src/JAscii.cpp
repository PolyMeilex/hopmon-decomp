#include "JAscii.h"
#include "JSprite.h"
#include <stdlib.h>
#include <string.h>

// 0x0040D540, 200 bytes.
JNumber::JNumber(JSurfaceList* surfaceList, unsigned short resId, int numFrames)
    : numberSprite(new JSprite(surfaceList, resId, numFrames)),
      posX(0),
      posY(0),
      displayZero(1),
      numDigits(0),
      number(0)
{
}

// 0x0040D610, 82 bytes.
JNumber::~JNumber()
{
    if (numberSprite != NULL) {
        delete numberSprite;
        numberSprite = NULL;
    }
}

// 0x0040D670, 31 bytes.
void JNumber::SetPosition(int x, int y)
{
    posX = x;
    posY = y;
}

// 0x0040D690, 22 bytes.
void JNumber::SetDisplayZero(int display)
{
    displayZero = display;
}

// 0x0040D6B0, 22 bytes.
void JNumber::SetNumDigits(int _numDigits)
{
    numDigits = _numDigits;
}

// 0x0040D6D0, 22 bytes.
void JNumber::SetNumber(int _number)
{
    number = _number;
}

// 0x0040D6F0, 319 bytes.
void JNumber::Draw(IDirectDrawSurface7* lpSurface)
{
    static char buffer[40];

    _ltoa(number, buffer, 10);
    int length = strlen(buffer);
    int nx = posX;
    if (numDigits > 0 && length < numDigits) {
        int zeros = numDigits - length;
        for (int i = 0; i < zeros; i++) {
            numberSprite->SetCurrentFrame(0);
            numberSprite->SetPosition(nx, posY);
            if (displayZero) {
                numberSprite->Draw(lpSurface);
            }
            nx += GetDigitWidth();
        }
    }
    for (int i = 0; i < length; i++) {
        if (length - i <= numDigits) {
            int numIndex = buffer[i] - '0';
            numberSprite->SetCurrentFrame(numIndex);
            numberSprite->SetPosition(nx, posY);
            numberSprite->Draw(lpSurface);
            nx += GetDigitWidth();
        }
    }
}

// 0x0040D830, 17 bytes.
int JNumber::GetPosX() const
{
    return posX;
}

// 0x0040D850, 17 bytes.
int JNumber::GetPosY() const
{
    return posY;
}

// 0x0040D870, 30 bytes.
int JNumber::GetDigitWidth() const
{
    return numberSprite->GetWidth() / 3;
}

// 0x0040D890, 26 bytes.
int JNumber::GetWidth() const
{
    return GetDigitWidth() * numDigits;
}

// 0x0040D8B0, 22 bytes.
int JNumber::GetHeight() const
{
    return numberSprite->GetHeight();
}
