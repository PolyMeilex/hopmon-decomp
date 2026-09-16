#include "JSprite.h"
#include "JSurface.h"
#include "JStandard.h"

// 0x00412F40, 185 bytes.
JSprite::JSprite(JSurfaceList* _surfaceList, unsigned short _resId, int _numFrames)
    : surfaceList(_surfaceList),
      resId(_resId),
      posX(0.0f),
      posY(0.0f),
      width(-1),
      height(-1),
      drawWidth(-1),
      drawHeight(-1),
      currentFrame(0),
      numFrames(_numFrames)
{
    JSurface* surface = surfaceList->GetByResId(resId);
    width = surface->GetWidth();
    height = surface->GetHeight() / numFrames;
}

// 0x00413000, 20 bytes.
JSprite::~JSprite()
{
}

// 0x00413020, 17 bytes.
float JSprite::GetPosX() const
{
    return posX;
}

// 0x00413040, 17 bytes.
float JSprite::GetPosY() const
{
    return posY;
}

// 0x00413060, 31 bytes.
int JSprite::GetPosIntX() const
{
    return Round(GetPosX());
}

// 0x00413080, 31 bytes.
int JSprite::GetPosIntY() const
{
    return Round(GetPosY());
}

// 0x004130A0, 34 bytes.
int JSprite::GetCenterIntX() const
{
    return GetPosIntX() + width / 2;
}

// 0x004130D0, 34 bytes.
int JSprite::GetCenterIntY() const
{
    return GetPosIntY() + height / 2;
}

// 0x00413100, 17 bytes.
int JSprite::GetWidth() const
{
    return width;
}

// 0x00413120, 17 bytes.
int JSprite::GetHeight() const
{
    return height;
}

// 0x00413140, 17 bytes.
int JSprite::GetCurrentFrame() const
{
    return currentFrame;
}

// 0x00413160, 17 bytes.
int JSprite::GetNumFrames() const
{
    return numFrames;
}

// 0x00413180, 39 bytes.
void JSprite::SetPosition(int x, int y)
{
    posX = (float)x;
    posY = (float)y;
}

// 0x004131B0, 26 bytes.
void JSprite::SetPosX(int x)
{
    posX = (float)x;
}

// 0x004131D0, 26 bytes.
void JSprite::SetPosY(int y)
{
    posY = (float)y;
}

// 0x004131F0, 34 bytes.
void JSprite::ShiftPosX(float dx)
{
    posX += dx;
}

// 0x00413220, 34 bytes.
void JSprite::ShiftPosY(float dy)
{
    posY += dy;
}

// 0x00413250, 69 bytes.
void JSprite::SetCenterPos(int x, int y)
{
    posX = (float)(x - width / 2);
    posY = (float)(y - height / 2);
}

// 0x004132A0, 22 bytes.
void JSprite::SetCurrentFrame(int _currentFrame)
{
    currentFrame = _currentFrame;
}

// 0x004132C0, 33 bytes.
void JSprite::IncrCurrentFrame()
{
    currentFrame = (currentFrame + 1) % numFrames;
}

// 0x004132F0, 31 bytes.
void JSprite::SetDrawSize(int _drawWidth, int _drawHeight)
{
    drawWidth = _drawWidth;
    drawHeight = _drawHeight;
}

// 0x00413310, 335 bytes.
void JSprite::Draw(IDirectDrawSurface7* lpDestSurface)
{
    if (lpDestSurface == NULL) {
        return;
    }

    JSurface* surface = surfaceList->GetByResId(resId);
    IDirectDrawSurface7* lpSourceSurface = surface->GetDDSurface();
    int x = GetPosIntX();
    int y = GetPosIntY();
    int _drawWidth = (drawWidth >= 0) ? drawWidth : width;
    int _drawHeight = (drawHeight >= 0) ? drawHeight : height;

    RECT srcRect;
    srcRect.left = 0;
    srcRect.top = currentFrame * height;
    srcRect.right = width;
    srcRect.bottom = currentFrame * height + height;

    RECT destRect;
    destRect.left = x;
    destRect.top = y;
    destRect.right = x + _drawWidth;
    destRect.bottom = y + _drawHeight;

    if (surface->UseColorKey()) {
        lpDestSurface->Blt(&destRect, lpSourceSurface, &srcRect, DDBLT_WAIT | DDBLT_KEYSRC, NULL);
    } else {
        lpDestSurface->Blt(&destRect, lpSourceSurface, &srcRect, DDBLT_WAIT, NULL);
    }
}

// 0x00413460, 47 bytes.
void JSprite::DrawScale(IDirectDrawSurface7* lpDestSurface, float amount)
{
    DrawScaleXY(lpDestSurface, amount, amount);
}

// 0x00413490, 50 bytes.
void JSprite::DrawScaleX(IDirectDrawSurface7* lpDestSurface, float amountX)
{
    DrawScaleXY(lpDestSurface, amountX, 1.0f);
}

// 0x004134D0, 50 bytes.
void JSprite::DrawScaleY(IDirectDrawSurface7* lpDestSurface, float amountY)
{
    DrawScaleXY(lpDestSurface, 1.0f, amountY);
}

// 0x00413510, 349 bytes.
void JSprite::DrawScaleXY(IDirectDrawSurface7* lpDestSurface, float amountX, float amountY)
{
    if (lpDestSurface == NULL) {
        return;
    }

    JSurface* surface = surfaceList->GetByResId(resId);
    IDirectDrawSurface7* lpSourceSurface = surface->GetDDSurface();
    int destWidth = Round((float)width * amountX);
    int destX = GetCenterIntX() - destWidth / 2;
    int destHeight = Round((float)height * amountY);
    int destY = GetCenterIntY() - destHeight / 2;

    RECT destRect;
    destRect.left = destX;
    destRect.top = destY;
    destRect.right = destX + destWidth;
    destRect.bottom = destY + destHeight;

    RECT srcRect;
    srcRect.left = 0;
    srcRect.top = currentFrame * height;
    srcRect.right = width;
    srcRect.bottom = currentFrame * height + height;

    if (surface->UseColorKey()) {
        lpDestSurface->Blt(&destRect, lpSourceSurface, &srcRect, DDBLT_WAIT | DDBLT_KEYSRC, NULL);
    } else {
        lpDestSurface->Blt(&destRect, lpSourceSurface, &srcRect, DDBLT_WAIT, NULL);
    }
}
