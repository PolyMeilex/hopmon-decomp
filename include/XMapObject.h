#pragma once
#include "common.h"
#include "JObject3D.h"

class JObject3D_Shadow;
class XGameObject;

// PDB type 0x19A2
class XMapObjectBase : public JObject3DX {
    friend struct LayoutCheck;
public:
    enum STATE {
        NON = 0,
        PROCESS = 1,
    };

    XMapObjectBase(XGameObject* _game, unsigned short resId);
    virtual ~XMapObjectBase();
    virtual void Render();
    STATE GetState() const;
    void SetDirection(int dir, int faceToDir);
    virtual void ProcessRotate(unsigned long frameCount); // vtable 0x20
    virtual void ProcessFrame(); // vtable 0x24
protected:
    XGameObject* game; // 0x5C
    STATE state; // 0x60
    unsigned int localTime; // 0x64
    int direction; // 0x68
    JObject3D_Shadow* shadow; // 0x6C
};
ASSERT_SIZE(XMapObjectBase, 0x70);

// PDB type 0x19DC
class XVolcano : public XMapObjectBase {
    friend struct LayoutCheck;
public:
    XVolcano(XGameObject* _game);
    virtual ~XVolcano();
    virtual void ProcessFrame();
};
ASSERT_SIZE(XVolcano, 0x70);

// PDB type 0x1A04
class XCannonUp : public XMapObjectBase {
    friend struct LayoutCheck;
public:
    XCannonUp(XGameObject* _game);
    virtual ~XCannonUp();
    virtual void ProcessFrame();
};
ASSERT_SIZE(XCannonUp, 0x70);

// PDB type 0x19E6
class XCannon : public XMapObjectBase {
    friend struct LayoutCheck;
public:
    XCannon(XGameObject* _game, int initDir);
    virtual ~XCannon();
    virtual void ProcessFrame();
};
ASSERT_SIZE(XCannon, 0x70);

// PDB type 0x19F0
class XPole : public XMapObjectBase {
    friend struct LayoutCheck;
public:
    XPole(XGameObject* _game);
    virtual ~XPole();
};
ASSERT_SIZE(XPole, 0x70);

// PDB type 0x19FA
class XShortPole : public XMapObjectBase {
    friend struct LayoutCheck;
public:
    XShortPole(XGameObject* _game);
    virtual ~XShortPole();
};
ASSERT_SIZE(XShortPole, 0x70);

// PDB type 0x19C8
class XTree : public XMapObjectBase {
    friend struct LayoutCheck;
public:
    XTree(XGameObject* _game);
    virtual ~XTree();
};
ASSERT_SIZE(XTree, 0x70);

// PDB type 0x19D2
class XSteam : public XMapObjectBase {
    friend struct LayoutCheck;
public:
    XSteam(XGameObject* _game);
    virtual ~XSteam();
    virtual void ProcessFrame();
};
ASSERT_SIZE(XSteam, 0x70);

// PDB type 0x197C
class XMapObjectList {
    friend struct LayoutCheck;
public:
    XMapObjectList(XGameObject* _game);
    virtual ~XMapObjectList(); // vtable 0x00
    XMapObjectBase* CreateMapObject(unsigned char type);
    void Empty();
    void ProcessRotate(unsigned long frameCount);
    void ProcessFrame();
    void Render();
protected:
    int GetNullIndex() const;
private:
    XGameObject* game; // 0x04
    XMapObjectBase* objArray[25]; // 0x08
};
ASSERT_SIZE(XMapObjectList, 0x6C);
