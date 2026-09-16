#pragma once
#include "common.h"
#include <d3d.h>
#include "JObject3D.h"

class JObject3D_Shadow;
class XCrystal;
class XEnemyFireBase;
class XGameObject;
class XParticleBase;

// PDB type 0x1934
class XCrystalList {
    friend struct LayoutCheck;
public:
    XCrystalList(XGameObject* _game);
    virtual ~XCrystalList(); // vtable 0x00
    XCrystal* CreateCrystal();
    void Empty();
    void CheckCollisionWithPac();
    void DoVanishCrystal();
    void StartGameOver();
    int GetTakenCount() const;
    int GetCrystalLeft() const;
    void ProcessRotate(unsigned long frameCount);
    void ProcessFrame();
    void Render();
protected:
    int GetNullIndex() const;
private:
    XGameObject* game; // 0x04
    XCrystal* crystalArray[30]; // 0x08
    int takenCount; // 0x80
    int crystalLeft; // 0x84
};
ASSERT_SIZE(XCrystalList, 0x88);

// PDB type 0x1812
class XCrystal : public JObject3DX {
    friend struct LayoutCheck;
public:
    enum STATE {
        NON = 0,
        PROCESS = 1,
        TAKEN = 2,
        VANISHING = 3,
        GAME_OVER = 4,
    };

    XCrystal(XGameObject* _game);
    virtual ~XCrystal();
    virtual void Render();
    STATE GetState() const;
    void SetState(STATE _state);
    int GetTakenPosition() const;
    void StartTaken(int takenPos);
    void StartVanishing();
    void StartGameOver();
    void ProcessRotate(unsigned long frameCount);
    void ProcessFrame();
protected:
    XGameObject* game; // 0x5C
    STATE state; // 0x60
    unsigned int localTime; // 0x64
    JObject3D_Shadow* shadow; // 0x68
    int takenPosition; // 0x6C
    D3DVECTOR dirVector; // 0x70
    D3DVECTOR referencePos; // 0x7C
};
ASSERT_SIZE(XCrystal, 0x88);

// PDB type 0x1946
class XParticleList {
    friend struct LayoutCheck;
public:
    XParticleList(XGameObject* _game);
    virtual ~XParticleList(); // vtable 0x00
    void Empty();
    void ProcessFrame();
    void Render(const D3DMATRIX& _viewMatrix);
    void CreateEnemyExplode(D3DVECTOR pos);
    void CreateEnemyFireSmokePurple(D3DVECTOR pos);
    void CreateEnemyFireSmokeGreen(D3DVECTOR pos);
    void CreatePacFireHit(D3DVECTOR pos);
    void CreatePacVanish(D3DVECTOR pos);
protected:
    int GetNullIndex();
private:
    XGameObject* game; // 0x04
    unsigned short _arrayPointer; // 0x08
    XParticleBase* particleArray[150]; // 0x0C
};
ASSERT_SIZE(XParticleList, 0x264);

// PDB type 0x18BB
class XParticleBase : public JObject3D_Particle {
    friend struct LayoutCheck;
public:
    enum STATE {
        NON = 0,
        PROCESS = 1,
    };

    XParticleBase(XGameObject* _game, D3DVECTOR pos, float size);
    virtual ~XParticleBase();
    virtual void Render2(const D3DMATRIX& _viewMatrix);
    STATE GetState() const;
    virtual void ProcessFrame() = 0; // vtable 0x28
protected:
    XGameObject* game; // 0x88
    STATE state; // 0x8C
    unsigned int localTime; // 0x90
    D3DVECTOR refrencePos; // 0x94
};
ASSERT_SIZE(XParticleBase, 0xA0);

// PDB type 0x2C7B
class XParticle_EnemyExplode : public XParticleBase {
    friend struct LayoutCheck;
public:
    XParticle_EnemyExplode(XGameObject* _game, D3DVECTOR pos);
    virtual ~XParticle_EnemyExplode();
    void SetDirVector(D3DVECTOR _dirVector);
    virtual void ProcessFrame();
private:
    D3DVECTOR dirVector; // 0xA0
};
ASSERT_SIZE(XParticle_EnemyExplode, 0xAC);

// PDB type 0x2C67
class XParticle_EnemyFireSmokePurple : public XParticleBase {
    friend struct LayoutCheck;
public:
    XParticle_EnemyFireSmokePurple(XGameObject* _game, D3DVECTOR pos);
    virtual ~XParticle_EnemyFireSmokePurple();
    virtual void ProcessFrame();
};
ASSERT_SIZE(XParticle_EnemyFireSmokePurple, 0xA0);

// PDB type 0x2C8F
class XParticle_EnemyFireSmokeGreen : public XParticleBase {
    friend struct LayoutCheck;
public:
    XParticle_EnemyFireSmokeGreen(XGameObject* _game, D3DVECTOR pos);
    virtual ~XParticle_EnemyFireSmokeGreen();
    virtual void ProcessFrame();
};
ASSERT_SIZE(XParticle_EnemyFireSmokeGreen, 0xA0);

// PDB type 0x2CAD
class XParticle_PacFireHit : public XParticleBase {
    friend struct LayoutCheck;
public:
    XParticle_PacFireHit(XGameObject* _game, D3DVECTOR pos, float size, unsigned int _processFrame);
    virtual ~XParticle_PacFireHit();
    virtual void Render2(const D3DMATRIX& _viewMatrix);
    virtual void ProcessFrame();
private:
    void _SetTextureRandomly();
    unsigned int processFrame; // 0xA0
    unsigned int renderCount; // 0xA4
};
ASSERT_SIZE(XParticle_PacFireHit, 0xA8);

// PDB type 0x1829
class XEnemyFireList {
    friend struct LayoutCheck;
public:
    XEnemyFireList(XGameObject* _game);
    virtual ~XEnemyFireList(); // vtable 0x00
    void CheckCollisionWithPac();
    void Empty();
    void ProcessFrame();
    void Render(const D3DMATRIX& _viewMatrix);
    void CreateCannonUpFire(D3DVECTOR pos);
    void CreateVolcanoFire(D3DVECTOR pos);
    void CreateCannonFire(D3DVECTOR pos, int dir);
    void CreateUFOFire(D3DVECTOR pos);
    void CreateSteamFire(D3DVECTOR pos);
protected:
    int GetNullIndex() const;
private:
    XGameObject* game; // 0x04
    XEnemyFireBase* fireArray[60]; // 0x08
};
ASSERT_SIZE(XEnemyFireList, 0xF8);

// PDB type 0x17C1
class XEnemyFireBase : public JObject3D_Particle {
    friend struct LayoutCheck;
public:
    enum STATE {
        NON = 0,
        MOVE = 1,
    };

    XEnemyFireBase(XGameObject* _game, D3DVECTOR pos, float size);
    virtual ~XEnemyFireBase();
    virtual void Render2(const D3DMATRIX& _viewMatrix);
    STATE GetState() const;
    void SetState(STATE _state);
    virtual void ProcessFrame() = 0; // vtable 0x28
protected:
    XGameObject* game; // 0x88
    STATE state; // 0x8C
    unsigned int localTime; // 0x90
    D3DVECTOR refrencePos; // 0x94
};
ASSERT_SIZE(XEnemyFireBase, 0xA0);

// PDB type 0x2C71
class XEnemyFire_CannonUp : public XEnemyFireBase {
    friend struct LayoutCheck;
public:
    XEnemyFire_CannonUp(XGameObject* _game, D3DVECTOR pos);
    virtual ~XEnemyFire_CannonUp();
    void SetValues(float _distance, D3DVECTOR _dirVector);
    virtual void ProcessFrame();
private:
    float distance; // 0xA0
    D3DVECTOR dirVector; // 0xA4
};
ASSERT_SIZE(XEnemyFire_CannonUp, 0xB0);

// PDB type 0x2C85
class XEnemyFire_Cannon : public XEnemyFireBase {
    friend struct LayoutCheck;
public:
    XEnemyFire_Cannon(XGameObject* _game, D3DVECTOR pos);
    virtual ~XEnemyFire_Cannon();
    void SetValues(D3DVECTOR _dirVector);
    virtual void ProcessFrame();
private:
    D3DVECTOR dirVector; // 0xA0
};
ASSERT_SIZE(XEnemyFire_Cannon, 0xAC);

// PDB type 0x2CA3
class XEnemyFire_UFO : public XEnemyFireBase {
    friend struct LayoutCheck;
public:
    XEnemyFire_UFO(XGameObject* _game, D3DVECTOR pos);
    virtual ~XEnemyFire_UFO();
    virtual void ProcessFrame();
private:
    D3DVECTOR dirVector; // 0xA0
    int turned; // 0xAC
};
ASSERT_SIZE(XEnemyFire_UFO, 0xB0);

// PDB type 0x2C99
class XEnemyFire_Steam : public XEnemyFireBase {
    friend struct LayoutCheck;
public:
    XEnemyFire_Steam(XGameObject* _game, D3DVECTOR pos);
    virtual ~XEnemyFire_Steam();
    virtual void ProcessFrame();
};
ASSERT_SIZE(XEnemyFire_Steam, 0xA0);
