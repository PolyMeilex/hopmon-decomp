#pragma once
#include "common.h"
#include <d3d.h>
#include "JObject3D.h"

class JObject3D_Shadow;
class XGameObject;

// PDB type 0x18C4
class XPac : public JObject3DX {
    friend struct LayoutCheck;
public:
    enum STATE {
        NON = 0,
        WAIT = 1,
        MOVE = 2,
        ON_ARROW = 3,
        LEVEL_COMPLETED = 4,
        DYING = 5,
    };

    XPac(XGameObject* _game);
    virtual ~XPac();
    virtual void Render();
    void Reset();
    STATE GetState() const;
    int GetFaceDirection() const;
    void SetMapPos(int x, int z);
    int GetMapPosX() const;
    int GetMapPosZ() const;
    int GetFireEnergy() const;
    void ResetFireEnergy();
    void SetFaceDirection(int dir);
    void StartMoving(int dir);
    void StartArrow(int dir);
    void StartDying();
    void ProcessRotate(unsigned long frameCount);
    void ProcessFrame();
private:
    XGameObject* game; // 0x5C
    STATE state; // 0x60
    unsigned int localTime; // 0x64
    int mapPosX; // 0x68
    int mapPosZ; // 0x6C
    int faceDirection; // 0x70
    int moveDirection; // 0x74
    D3DVECTOR refrencePos; // 0x78
    JObject3D_Shadow* shadow; // 0x84
    unsigned int pacMoveFrame; // 0x88
    int fireEnergy; // 0x8C
};
ASSERT_SIZE(XPac, 0x90);

// PDB type 0x17C9
class XPacFire : public JObject3D_Particle {
    friend struct LayoutCheck;
public:
    enum STATE {
        NON = 0,
        MOVE = 1,
    };

    XPacFire(XGameObject* _game, int dir);
    virtual ~XPacFire();
    virtual void Render2(const D3DMATRIX& _viewMatrix);
    STATE GetState() const;
    void SetState(STATE _state);
    int GetDirection() const;
    void ProcessFrame();
protected:
    void _SetTextureRandomly();
private:
    XGameObject* game; // 0x88
    STATE state; // 0x8C
    unsigned int localTime; // 0x90
    int direction; // 0x94
};
ASSERT_SIZE(XPacFire, 0x98);

// PDB type 0x190A
class XPacFireList {
    friend struct LayoutCheck;
public:
    XPacFireList(XGameObject* _game);
    virtual ~XPacFireList(); // vtable 0x00
    void CreateFire();
    void Empty();
    void CheckCollisionWithEnemy();
    void ProcessFrame();
    void Render(const D3DMATRIX& _viewMatrix);
protected:
    int GetNullIndex() const;
private:
    XGameObject* game; // 0x04
    XPacFire* fireArray[3]; // 0x08
};
ASSERT_SIZE(XPacFireList, 0x14);

D3DVECTOR GetVectorFromDir(int dir);
