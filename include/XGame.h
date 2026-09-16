#pragma once
#include "common.h"
#include <d3d.h>

class JNumber;
class JSprite;
class XApplication;
class XCamera;
class XCrystalList;
class XEnemyFireList;
class XEnemyList;
class XGameObject;
class XMap;
class XMapObjectList;
class XPac;
class XPacFireList;
class XParticleList;

// PDB type 0x17D8
class XStatusDisplay {
    friend struct LayoutCheck;
public:
    XStatusDisplay(XGameObject* _game);
    virtual ~XStatusDisplay(); // vtable 0x00
    void Draw(IDirectDrawSurface7* lpSurface);
private:
    XGameObject* game; // 0x04
    JSprite* textLevel; // 0x08
    JSprite* textCrystals; // 0x0C
    JSprite* textFireReady; // 0x10
    JSprite* fireLED; // 0x14
    JNumber* numLevel; // 0x18
    JNumber* numCrystals; // 0x1C
};
ASSERT_SIZE(XStatusDisplay, 0x20);

// PDB type 0x1761
class XProcessObject {
    friend struct LayoutCheck;
public:
    enum STATE {
        NON = 0,
        FADEIN = 1,
        FADEOUT = 2,
        PROCESS = 3,
    };

    XProcessObject(XApplication* _app);
    virtual ~XProcessObject(); // vtable 0x00
    virtual void ProcessFrame(unsigned long frameCount); // vtable 0x04
    virtual void Render(); // vtable 0x08
    void StartFadeIn(int _processType);
    void StartFadeOut(int _processType);
    void StartProcess(int _processType);
    void ProcessFadeIn();
    void ProcessFadeOut();
    XApplication* app; // 0x04
protected:
    void _ResetFadeIn();
    void _ResetFadeOut();
    STATE state; // 0x08
    unsigned int tick; // 0x0C
    JSprite* fadeLeft; // 0x10
    JSprite* fadeRight; // 0x14
    int processType; // 0x18
    int processFinished; // 0x1C
};
ASSERT_SIZE(XProcessObject, 0x20);

// PDB type 0x1921
class XGameObject : public XProcessObject {
    friend struct LayoutCheck;
public:
    XGameObject(XApplication* _app);
    virtual ~XGameObject();
    virtual void ProcessFrame(unsigned long frameCount);
    virtual void Render();
    void InitLevel();
    int GetCurrentLevel() const;
    unsigned char GetVolumeOfPosition(D3DVECTOR pos);
    XCamera* camera; // 0x20
    XMap* map; // 0x24
    XPac* pac; // 0x28
    XEnemyList* enemyList; // 0x2C
    XMapObjectList* mapObjList; // 0x30
    XPacFireList* pacFireList; // 0x34
    XEnemyFireList* enemyFireList; // 0x38
    XParticleList* particleList; // 0x3C
    XCrystalList* crystalList; // 0x40
    XStatusDisplay* statusDisplay; // 0x44
};
ASSERT_SIZE(XGameObject, 0x48);
