#pragma once
#include "common.h"
#include "XGame.h"

class JDInput;
class JNumber;
class JObject3DX;
class JObject3D_Rect;
class JSprite;
class XApplication;
class XCamera;
class XPreviewMap;

// PDB type 0x1700
class XTitleBackground {
    friend struct LayoutCheck;
public:
    XTitleBackground(XApplication* _app);
    virtual ~XTitleBackground(); // vtable 0x00
    void ProcessFrame(unsigned long frameCount);
    void Render();
private:
    XApplication* app; // 0x04
    JObject3D_Rect* backUpper; // 0x08
    JObject3D_Rect* backLower; // 0x0C
    float textureOffset; // 0x10
};
ASSERT_SIZE(XTitleBackground, 0x14);

// PDB type 0x16F7
class XLogoObject : public XProcessObject {
    friend struct LayoutCheck;
public:
    XLogoObject(XApplication* _app);
    virtual ~XLogoObject();
    virtual void ProcessFrame(unsigned long frameCount);
    virtual void Render();
private:
    XCamera* camera; // 0x20
    JSprite* logo; // 0x24
    JObject3D_Rect* backWhite; // 0x28
};
ASSERT_SIZE(XLogoObject, 0x2C);

// PDB type 0x14F1
class XRegisterInfoObject : public XProcessObject {
    friend struct LayoutCheck;
public:
    XRegisterInfoObject(XApplication* _app);
    virtual ~XRegisterInfoObject();
    virtual void ProcessFrame(unsigned long frameCount);
    virtual void Render();
private:
    XCamera* camera; // 0x20
    XTitleBackground* background; // 0x24
    JSprite* cursorLeft; // 0x28
    JSprite* cursorRight; // 0x2C
    JSprite* menuBack; // 0x30
    JSprite* screenShot1; // 0x34
    JSprite* screenShot2; // 0x38
    JSprite* regInfo; // 0x3C
};
ASSERT_SIZE(XRegisterInfoObject, 0x40);

// PDB type 0x14D0
class XSelectLevelObject : public XProcessObject {
    friend struct LayoutCheck;
public:
    XSelectLevelObject(XApplication* _app);
    virtual ~XSelectLevelObject();
    virtual void ProcessFrame(unsigned long frameCount);
    virtual void Render();
protected:
    void CreatePreviewMap();
private:
    XCamera* camera; // 0x20
    XTitleBackground* background; // 0x24
    XPreviewMap* map; // 0x28
    JSprite* cursorLeft; // 0x2C
    JSprite* cursorRight; // 0x30
    JSprite* cursor2Left; // 0x34
    JSprite* cursor2Right; // 0x38
    JSprite* menuItems[3]; // 0x3C
    JSprite* textUnreg; // 0x48
    JSprite* textLevel; // 0x4C
    JSprite* levelLED; // 0x50
    JNumber* numLevel; // 0x54
    int cursorPos; // 0x58
    int inSelectLevel; // 0x5C
};
ASSERT_SIZE(XSelectLevelObject, 0x60);

// PDB type 0x15D0
class XTitleObject : public XProcessObject {
    friend struct LayoutCheck;
public:
    XTitleObject(XApplication* _app);
    virtual ~XTitleObject();
    virtual void ProcessFrame(unsigned long frameCount);
    virtual void Render();
private:
    XCamera* camera; // 0x20
    JObject3DX* pac; // 0x24
    XTitleBackground* background; // 0x28
    JSprite* cursorLeft; // 0x2C
    JSprite* cursorRight; // 0x30
    JSprite* menuItems[2]; // 0x34
    JSprite* textHopmon; // 0x3C
    JSprite* textCopyright; // 0x40
    JSprite* textUnreg; // 0x44
    int cursorPos; // 0x48
};
ASSERT_SIZE(XTitleObject, 0x4C);

// PDB type 0x164B
class XEndingObject : public XProcessObject {
    friend struct LayoutCheck;
public:
    XEndingObject(XApplication* _app);
    virtual ~XEndingObject();
    virtual void ProcessFrame(unsigned long frameCount);
    virtual void Render();
private:
    XCamera* camera; // 0x20
    JObject3DX* chara[10]; // 0x24
    XTitleBackground* background; // 0x4C
    JSprite* textCongra; // 0x50
    JSprite* textStaff; // 0x54
    JSprite* textCopyright; // 0x58
    int charaIndex; // 0x5C
    float charaMaxScale[10]; // 0x60
};
ASSERT_SIZE(XEndingObject, 0x88);

int CheckButtonPressed(JDInput* input);
