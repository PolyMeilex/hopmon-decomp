#pragma once
#include "common.h"
#include <d3d.h>
#include "JObject3D.h"

class JObject3D_Shadow;
class XEnemyBase;
class XGameObject;

// PDB type 0x18D7
class XEnemyList {
    friend struct LayoutCheck;
public:
    XEnemyList(XGameObject* _game);
    virtual ~XEnemyList(); // vtable 0x00
    XEnemyBase* CreateEnemy(unsigned char type);
    XEnemyBase* GetAt(unsigned long index) const;
    void Empty();
    void CheckCollisionWithPac();
    void ProcessRotate(unsigned long frameCount);
    void ProcessFrame();
    void Render();
protected:
    int GetNullIndex() const;
private:
    XGameObject* game; // 0x04
    XEnemyBase* enemyArray[30]; // 0x08
};
ASSERT_SIZE(XEnemyList, 0x80);

// PDB type 0x187C
class XEnemyBase : public JObject3DX {
    friend struct LayoutCheck;
public:
    enum STATE {
        NON = 0,
        WAIT = 1,
        MOVE = 2,
        FIRE = 3,
        DYING = 4,
    };

    XEnemyBase(XGameObject* _game, unsigned short resId);
    virtual ~XEnemyBase();
    virtual void Render();
    STATE GetState() const;
    int Breakable() const;
    void SetMapPos(int x, int z);
    void SetDirection(int dir);
    void StartMoving(int dir);
    void StartDying(int dir);
    void StartFiring();
    void DecreaseShield(int dir);
    void ProcessRotate(unsigned long frameCount);
    virtual void SubProcessDyingRotate(unsigned long frameCount); // vtable 0x20
    virtual void SubProcessMoveRotate(unsigned long frameCount); // vtable 0x24
    virtual void SubProcessFireRotate(unsigned long frameCount); // vtable 0x28
    void ProcessFrame();
    virtual void SubProcessDying(); // vtable 0x2C
    virtual void SubProcessMove(); // vtable 0x30
    virtual void SubProcessWait(); // vtable 0x34
    virtual void SubProcessFire(); // vtable 0x38
protected:
    int IsBlocked(int dir) const;
    int FindDirectionToPac() const;
    void DefaultProcessWait();
    XGameObject* game; // 0x5C
    STATE state; // 0x60
    unsigned int localTime; // 0x64
    int mapPosX; // 0x68
    int mapPosZ; // 0x6C
    D3DVECTOR refrencePos; // 0x70
    int direction; // 0x7C
    JObject3D_Shadow* shadow; // 0x80
    int faceToDir; // 0x84
    int breakable; // 0x88
    int shield; // 0x8C
};
ASSERT_SIZE(XEnemyBase, 0x90);

// PDB type 0x1B2C
class XEnemy_Ghost : public XEnemyBase {
    friend struct LayoutCheck;
public:
    XEnemy_Ghost(XGameObject* _game);
    virtual ~XEnemy_Ghost();
    virtual void SubProcessMove();
    virtual void SubProcessWait();
};
ASSERT_SIZE(XEnemy_Ghost, 0x90);

// PDB type 0x1AC9
class XEnemy_Cube : public XEnemyBase {
    friend struct LayoutCheck;
public:
    XEnemy_Cube(XGameObject* _game);
    virtual ~XEnemy_Cube();
    virtual void SubProcessMoveRotate(unsigned long frameCount);
    virtual void SubProcessMove();
    virtual void SubProcessWait();
};
ASSERT_SIZE(XEnemy_Cube, 0x90);

// PDB type 0x1B67
class XEnemy_Kurage : public XEnemyBase {
    friend struct LayoutCheck;
public:
    XEnemy_Kurage(XGameObject* _game);
    virtual ~XEnemy_Kurage();
    virtual void SubProcessMove();
    virtual void SubProcessWait();
};
ASSERT_SIZE(XEnemy_Kurage, 0x90);

// PDB type 0x1B3F
class XEnemy_UFO : public XEnemyBase {
    friend struct LayoutCheck;
public:
    XEnemy_UFO(XGameObject* _game);
    virtual ~XEnemy_UFO();
    virtual void SubProcessMoveRotate(unsigned long frameCount);
    virtual void SubProcessFireRotate(unsigned long frameCount);
    virtual void SubProcessMove();
    virtual void SubProcessWait();
    virtual void SubProcessFire();
private:
    void StartMoving_UFO(int dir);
    int IsBlocked_UFO(int dir) const;
    int moveCount; // 0x90
    int countDownToFire; // 0x94
};
ASSERT_SIZE(XEnemy_UFO, 0x98);

// PDB type 0x1ADD
class XEnemy_Sesame : public XEnemyBase {
    friend struct LayoutCheck;
public:
    XEnemy_Sesame(XGameObject* _game);
    virtual ~XEnemy_Sesame();
    virtual void SubProcessMove();
    virtual void SubProcessWait();
private:
    int moveCountDown; // 0x90
    int movePhase; // 0x94
};
ASSERT_SIZE(XEnemy_Sesame, 0x98);

// PDB type 0x1B5D
class XNeedleBall : public XEnemyBase {
    friend struct LayoutCheck;
public:
    XNeedleBall(XGameObject* _game, int initDir);
    virtual ~XNeedleBall();
    virtual void SubProcessMoveRotate(unsigned long frameCount);
    virtual void SubProcessMove();
    virtual void SubProcessWait();
};
ASSERT_SIZE(XNeedleBall, 0x90);

// PDB type 0x1AD3
class XNeedleBallFast : public XEnemyBase {
    friend struct LayoutCheck;
public:
    XNeedleBallFast(XGameObject* _game, int initDir);
    virtual ~XNeedleBallFast();
    virtual void SubProcessMoveRotate(unsigned long frameCount);
    virtual void SubProcessMove();
    virtual void SubProcessWait();
};
ASSERT_SIZE(XNeedleBallFast, 0x90);

// PDB type 0x1B36
class XNeedleBox : public XEnemyBase {
    friend struct LayoutCheck;
public:
    XNeedleBox(XGameObject* _game);
    virtual ~XNeedleBox();
    virtual void SubProcessMove();
    virtual void SubProcessWait();
};
ASSERT_SIZE(XNeedleBox, 0x90);

// PDB type 0x1B53
class XNeedleBoxDrop : public XEnemyBase {
    friend struct LayoutCheck;
public:
    XNeedleBoxDrop(XGameObject* _game);
    virtual ~XNeedleBoxDrop();
    virtual void SubProcessMove();
    virtual void SubProcessWait();
};
ASSERT_SIZE(XNeedleBoxDrop, 0x90);

// PDB type 0x1B49
class XGate : public XEnemyBase {
    friend struct LayoutCheck;
public:
    XGate(XGameObject* _game, int initDir);
    virtual ~XGate();
};
ASSERT_SIZE(XGate, 0x90);
