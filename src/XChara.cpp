#include "XChara.h"
#include "XAppMain.h"
#include "XGame.h"
#include "XPac.h"
#include "XMap.h"
#include "JSound.h"
#include "JStandard.h"
#include "D3DTextr.h"
#include <math.h>

// 0x00417900, 102 bytes.
XCrystalList::XCrystalList(XGameObject* _game)
    : game(_game), takenCount(0), crystalLeft(0)
{
    for (int i = 0; i < 30; i++) {
        crystalArray[i] = 0;
    }
}

// 0x00417970, 75 bytes.
XCrystalList::~XCrystalList()
{
    Empty();
}

// 0x004179C0, 189 bytes.
XCrystal* XCrystalList::CreateCrystal()
{
    int index = GetNullIndex();
    if (index >= 0) {
        crystalLeft++;
        crystalArray[index] = new XCrystal(game);
        return crystalArray[index];
    }
    return 0;
}

// 0x00417A80, 137 bytes.
void XCrystalList::Empty()
{
    takenCount = 0;
    crystalLeft = 0;
    for (int i = 0; i < 30; i++) {
        if (crystalArray[i]) {
            delete crystalArray[i];
            crystalArray[i] = 0;
        }
    }
}

// 0x00417B10, 249 bytes.
void XCrystalList::CheckCollisionWithPac()
{
    if (game->pac->GetState() == XPac::NON) {
        return;
    }
    if (game->pac->GetState() == XPac::LEVEL_COMPLETED) {
        return;
    }
    if (game->pac->GetState() == XPac::DYING) {
        return;
    }
    if (game->pac->GetState() == XPac::WAIT) {
        return;
    }
    for (int i = 0; i < 30; i++) {
        if (crystalArray[i] == 0) {
            continue;
        }
        if (crystalArray[i]->GetState() != XCrystal::PROCESS) {
            continue;
        }
        if (game->pac->CheckCollisionRadius(crystalArray[i])) {
            takenCount++;
            crystalArray[i]->StartTaken(takenCount);
        }
    }
}

// 0x00417C10, 181 bytes.
void XCrystalList::DoVanishCrystal()
{
    if (takenCount <= 0) {
        return;
    }
    for (int i = 0; i < 30; i++) {
        if (crystalArray[i] == 0) {
            continue;
        }
        if (crystalArray[i]->GetState() != XCrystal::TAKEN) {
            continue;
        }
        if (crystalArray[i]->GetTakenPosition() == takenCount) {
            crystalArray[i]->StartVanishing();
            takenCount--;
            crystalLeft--;
            return;
        }
    }
}

// 0x00417CD0, 112 bytes.
void XCrystalList::StartGameOver()
{
    for (int i = 0; i < 30; i++) {
        if (crystalArray[i] == 0) {
            continue;
        }
        if (crystalArray[i]->GetState() != XCrystal::TAKEN) {
            continue;
        }
        crystalArray[i]->StartGameOver();
        takenCount--;
    }
}

// 0x00417D40, 20 bytes.
int XCrystalList::GetTakenCount() const
{
    return takenCount;
}

// 0x00417D60, 20 bytes.
int XCrystalList::GetCrystalLeft() const
{
    return crystalLeft;
}

// 0x00417D80, 73 bytes.
void XCrystalList::ProcessRotate(unsigned long frameCount)
{
    for (int i = 0; i < 30; i++) {
        if (crystalArray[i]) {
            crystalArray[i]->ProcessRotate(frameCount);
        }
    }
}

// 0x00417DD0, 145 bytes.
void XCrystalList::ProcessFrame()
{
    for (int i = 0; i < 30; i++) {
        if (crystalArray[i]) {
            crystalArray[i]->ProcessFrame();
            if (crystalArray[i]->GetState() == XCrystal::NON) {
                delete crystalArray[i];
                crystalArray[i] = 0;
            }
        }
    }
}

// 0x00417E70, 79 bytes.
void XCrystalList::Render()
{
    for (int i = 0; i < 30; i++) {
        if (crystalArray[i]) {
            crystalArray[i]->Render();
        }
    }
}

// 0x00417EC0, 60 bytes.
int XCrystalList::GetNullIndex() const
{
    for (int i = 0; i < 30; i++) {
        if (crystalArray[i] == 0) {
            return i;
        }
    }
    return -1;
}

// 0x00417F00, 508 bytes.
XCrystal::XCrystal(XGameObject* _game)
    : JObject3DX(_game->app, 0xAB), game(_game), state(NON), localTime(0),
      shadow(new JObject3D_Shadow(_game->app, 20.0f, 20.0f)), takenPosition(0),
      dirVector(0.0f, 0.0f, 0.0f), referencePos(0.0f, 0.0f, 0.0f)
{
    shadow->SetTexture(0xA3, 0.0f, 0.0f, 1.0f, 1.0f);
    RotateBy(JAXIS_X, -1.5707964f);
    ScaleBy(30.0f);
    ShiftWorldPos_OnGround();
    ShiftWorldPos(JAXIS_Y, 30.0f);
    SetCollisionRadius(8.0f);
    SetFlatShade(1);
    state = PROCESS;
}

// 0x00418100, 90 bytes.
XCrystal::~XCrystal()
{
    if (shadow) {
        delete shadow;
        shadow = 0;
    }
}

// 0x00418160, 120 bytes.
void XCrystal::Render()
{
    if (state != NON) {
        JObject3DX::Render();
        if (state == PROCESS) {
            shadow->SetWorldPos(D3DVECTOR(worldPos.x, 1.0f, worldPos.z));
            shadow->Render();
        }
    }
}

// 0x004181E0, 17 bytes.
XCrystal::STATE XCrystal::GetState() const
{
    return state;
}

// 0x00418200, 22 bytes.
void XCrystal::SetState(STATE _state)
{
    state = _state;
}

// 0x00418220, 30 bytes.
int XCrystal::GetTakenPosition() const
{
    if (state == TAKEN) {
        return takenPosition;
    }
    return 0;
}

// 0x00418240, 72 bytes.
void XCrystal::StartTaken(int takenPos)
{
    game->app->GetSoundList()->Play(0xE8, 100, 0);
    takenPosition = takenPos;
    state = TAKEN;
    localTime = 0;
}

// 0x00418290, 61 bytes.
void XCrystal::StartVanishing()
{
    game->app->GetSoundList()->Play(0xE9, 100, 0);
    state = VANISHING;
    localTime = 0;
}

// 0x004182D0, 154 bytes.
void XCrystal::StartGameOver()
{
    long angle = Random(360);
    dirVector.y = 0.0f;
    dirVector.x = sinf((float)angle * 0.017453292f);
    dirVector.z = cosf((float)angle * 0.017453292f);
    referencePos = worldPos;
    state = GAME_OVER;
    localTime = 0;
}

// 0x00418370, 313 bytes.
void XCrystal::ProcessRotate(unsigned long frameCount)
{
    if (state == PROCESS) {
        const float angleStep = 0.012217305f;
        float angle = (float)frameCount * angleStep;
        RotateBy(JAXIS_Y, angle);
    } else if (state == VANISHING) {
        const float angleStep = 0.06981317f;
        float angle = (float)frameCount * angleStep;
        RotateBy(JAXIS_Y, angle);
    } else if (state == GAME_OVER) {
        const float angleStep = 0.06981317f;
        float angle = (float)frameCount * angleStep;
        RotateBy(JAXIS_Y, angle);
    }
}

// 0x004184B0, 548 bytes.
void XCrystal::ProcessFrame()
{
    localTime++;

    if (state == TAKEN) {
        SetWorldPos(game->pac->GetWorldPos());
        ShiftWorldPos(JAXIS_Y, (float)(takenPosition - 1) * 12.0f + 20.0f);
    } else if (state == VANISHING) {
        ShiftWorldPos(JAXIS_Y, 0.8f);
        if (worldPos.y > 800.0f) {
            state = NON;
            localTime = 0;
        }
    } else if (state == GAME_OVER) {
        const int frames = 0xA0;
        float radius = (float)localTime / 160.0f * 160.0f;
        float height = 0.0f;
        if (localTime < frames) {
            height = sinf((float)localTime / 160.0f * 3.1415927f) * 60.0f;
        } else {
            height = (float)(localTime - frames) * -1.4f;
        }

        D3DVECTOR offset = D3DVectorMultiply(dirVector, radius);
        offset.y = height;
        SetWorldPos(referencePos + offset);

        if (worldPos.y < -20.0f) {
            state = NON;
            localTime = 0;
        }
    }
}

// 0x004186E0, 88 bytes.
XParticleList::XParticleList(XGameObject* _game)
    : game(_game), _arrayPointer(0)
{
    for (int i = 0; i < 150; i++) {
        particleArray[i] = 0;
    }
}

// 0x00418740, 75 bytes.
XParticleList::~XParticleList()
{
    Empty();
}

// 0x00418EB0, 111 bytes.
int XParticleList::GetNullIndex()
{
    unsigned short _startPointer = _arrayPointer;
    int index = -1;
    while (index == -1) {
        if (particleArray[_arrayPointer] == 0) {
            index = _arrayPointer;
        }
        _arrayPointer = (_arrayPointer + 1) % 150;
        if (_arrayPointer == _startPointer) {
            break;
        }
    }
    return index;
}

// 0x00418790, 114 bytes.
void XParticleList::Empty()
{
    for (int i = 0; i < 150; i++) {
        if (particleArray[i]) {
            delete particleArray[i];
            particleArray[i] = 0;
        }
    }
}

// 0x00418810, 163 bytes.
void XParticleList::ProcessFrame()
{
    for (int i = 0; i < 150; i++) {
        if (particleArray[i]) {
            particleArray[i]->ProcessFrame();
            if (particleArray[i]->GetState() == XParticleBase::NON) {
                delete particleArray[i];
                particleArray[i] = 0;
            }
        }
    }
}

// 0x004188C0, 88 bytes.
void XParticleList::Render(const D3DMATRIX& _viewMatrix)
{
    for (int i = 0; i < 150; i++) {
        if (particleArray[i]) {
            particleArray[i]->Render2(_viewMatrix);
        }
    }
}

// 0x00418920, 521 bytes.
void XParticleList::CreateEnemyExplode(D3DVECTOR pos)
{
    unsigned char volume = game->GetVolumeOfPosition(pos);
    game->app->GetSoundList()->Play(0xEB, volume, 0);
    for (int i = 0; i < 30; i++) {
        int index = GetNullIndex();
        if (index < 0) {
            break;
        }
        XParticle_EnemyExplode* particle = new XParticle_EnemyExplode(game, pos);
        int verticalAngle = Random(180);
        int horizontalAngle = Random(360);
        float vDistance = sinf((float)verticalAngle * 0.017453292f);
        float hDistance = cosf((float)verticalAngle * 0.017453292f);
        D3DVECTOR vector;
        vector.y = vDistance;
        vector.x = sinf((float)horizontalAngle * 0.017453292f) * hDistance;
        vector.z = cosf((float)horizontalAngle * 0.017453292f) * hDistance;
        particle->SetDirVector(vector);
        particleArray[index] = particle;
    }
}

// 0x00418B30, 178 bytes.
void XParticleList::CreateEnemyFireSmokePurple(D3DVECTOR pos)
{
    int index = GetNullIndex();
    if (index >= 0) {
        particleArray[index] = new XParticle_EnemyFireSmokePurple(game, pos);
    }
}

// 0x00418BF0, 178 bytes.
void XParticleList::CreateEnemyFireSmokeGreen(D3DVECTOR pos)
{
    int index = GetNullIndex();
    if (index >= 0) {
        particleArray[index] = new XParticle_EnemyFireSmokeGreen(game, pos);
    }
}

// 0x00418CB0, 267 bytes.
void XParticleList::CreatePacFireHit(D3DVECTOR pos)
{
    int index = GetNullIndex();
    if (index >= 0) {
        unsigned char volume = game->GetVolumeOfPosition(pos);
        game->app->GetSoundList()->Play(0xEA, volume, 0);
        particleArray[index] = new XParticle_PacFireHit(game, pos, 70.0f, 16);
    }
}

// 0x00418DC0, 231 bytes.
void XParticleList::CreatePacVanish(D3DVECTOR pos)
{
    int index = GetNullIndex();
    if (index >= 0) {
        game->app->GetSoundList()->Play(0xF1, 100, 0);
        particleArray[index] = new XParticle_PacFireHit(game, pos, 150.0f, 150);
    }
}

// 0x00418F20, 207 bytes.
XParticleBase::XParticleBase(XGameObject* _game, D3DVECTOR pos, float size)
    : JObject3D_Particle(_game->app, size, size),
      game(_game),
      state(NON),
      localTime(0),
      refrencePos(pos)
{
    SetWorldPos(pos);
    state = PROCESS;
}

// 0x00418FF0, 28 bytes.
XParticleBase::~XParticleBase()
{
}

// 0x00419010, 37 bytes.
void XParticleBase::Render2(const D3DMATRIX& _viewMatrix)
{
    if (state != NON) {
        JObject3D_Particle::Render2(_viewMatrix);
    }
}

// 0x00419040, 20 bytes.
XParticleBase::STATE XParticleBase::GetState() const
{
    return state;
}

// 0x00419060, 229 bytes.
XParticle_EnemyExplode::XParticle_EnemyExplode(XGameObject* _game, D3DVECTOR pos)
    : XParticleBase(_game, pos, 50.0f),
      dirVector(0.0f, 0.0f, 0.0f)
{
    SetTexture(0xA2, 0.0f, 0.0f, 1.0f, 1.0f);
}

// 0x00419150, 28 bytes.
XParticle_EnemyExplode::~XParticle_EnemyExplode()
{
}

// 0x00419170, 38 bytes.
void XParticle_EnemyExplode::SetDirVector(D3DVECTOR _dirVector)
{
    dirVector = _dirVector;
}

// 0x004191A0, 374 bytes.
void XParticle_EnemyExplode::ProcessFrame()
{
    const int PROCESS_FRAME = 120;
    localTime++;
    if (state == PROCESS) {
        float scaleValue = 0.0f;
        float movedDistance = 0.0f;
        if (localTime < PROCESS_FRAME) {
            scaleValue = 1.0f - (float)localTime / (float)PROCESS_FRAME;
            movedDistance = sinf((float)localTime / (float)PROCESS_FRAME * 1.5707964f) * 55.0f;
        }
        ScaleTo(scaleValue);
        D3DVECTOR movedVector = D3DVectorMultiply(dirVector, movedDistance);
        SetWorldPos(refrencePos + movedVector);
        if (localTime >= PROCESS_FRAME) {
            state = NON;
            localTime = 0;
        }
    }
}

// 0x004194D0, 188 bytes.
XParticle_EnemyFireSmokeGreen::XParticle_EnemyFireSmokeGreen(XGameObject* _game, D3DVECTOR pos)
    : XParticleBase(_game, pos, 17.0f)
{
    SetTexture(0xB7, 0.0f, 0.0f, 1.0f, 1.0f);
}

// 0x00419590, 28 bytes.
XParticle_EnemyFireSmokeGreen::~XParticle_EnemyFireSmokeGreen()
{
}

// 0x004195B0, 201 bytes.
void XParticle_EnemyFireSmokeGreen::ProcessFrame()
{
    const int PROCESS_FRAME = 50;
    localTime++;
    if (state == PROCESS) {
        float scaleValue = 0.0f;
        if (localTime < PROCESS_FRAME) {
            scaleValue = 1.0f - (float)localTime / (float)PROCESS_FRAME;
        }
        ScaleTo(scaleValue);
        if (localTime >= PROCESS_FRAME) {
            state = NON;
            localTime = 0;
        }
    }
}

// 0x00419320, 188 bytes.
XParticle_EnemyFireSmokePurple::XParticle_EnemyFireSmokePurple(XGameObject* _game, D3DVECTOR pos)
    : XParticleBase(_game, pos, 18.0f)
{
    SetTexture(0xAA, 0.0f, 0.0f, 1.0f, 1.0f);
}

// 0x004193E0, 28 bytes.
XParticle_EnemyFireSmokePurple::~XParticle_EnemyFireSmokePurple()
{
}

// 0x00419400, 201 bytes.
void XParticle_EnemyFireSmokePurple::ProcessFrame()
{
    const int PROCESS_FRAME = 50;
    localTime++;
    if (state == PROCESS) {
        float scaleValue = 0.0f;
        if (localTime < PROCESS_FRAME) {
            scaleValue = 1.0f - (float)localTime / (float)PROCESS_FRAME;
        }
        ScaleTo(scaleValue);
        if (localTime >= PROCESS_FRAME) {
            state = NON;
            localTime = 0;
        }
    }
}

// 0x00419680, 95 bytes.
XParticle_PacFireHit::XParticle_PacFireHit(XGameObject* _game, D3DVECTOR pos, float size, unsigned int _processFrame)
    : XParticleBase(_game, pos, size),
      processFrame(_processFrame),
      renderCount(2)
{
}

// 0x004196E0, 28 bytes.
XParticle_PacFireHit::~XParticle_PacFireHit()
{
}

// 0x00419960, 766 bytes.
void XParticle_PacFireHit::_SetTextureRandomly()
{
    const float unit = 1.0f / 3.0f;
    int r = Random(9);
    if (r == 0) {
        SetTexture(0xA0, 0.0f, 0.0f, unit, unit);
    } else if (r == 1) {
        SetTexture(0xA0, unit, 0.0f, unit * 2.0f, unit);
    } else if (r == 2) {
        SetTexture(0xA0, unit * 2.0f, 0.0f, 1.0f, unit);
    } else if (r == 3) {
        SetTexture(0xA0, 0.0f, unit, unit, unit * 2.0f);
    } else if (r == 4) {
        SetTexture(0xA0, unit, unit, unit * 2.0f, unit * 2.0f);
    } else if (r == 5) {
        SetTexture(0xA0, unit * 2.0f, unit, 1.0f, unit * 2.0f);
    } else if (r == 6) {
        SetTexture(0xA0, 0.0f, unit * 2.0f, unit, 1.0f);
    } else if (r == 7) {
        SetTexture(0xA0, unit, unit * 2.0f, unit * 2.0f, 1.0f);
    } else if (r == 8) {
        SetTexture(0xA0, unit * 2.0f, unit * 2.0f, 1.0f, 1.0f);
    }
}

// 0x00419700, 296 bytes.
void XParticle_PacFireHit::Render2(const D3DMATRIX& _viewMatrix)
{
    if (state != NON) {
        _SetWorldTransform2(_viewMatrix);
        _SetRenderState();
        for (unsigned int i = 0; i < renderCount; i++) {
            _SetTextureRandomly();
            if (textureResId) {
                app->GetD3DDevice()->SetTexture(0, D3DTextr_GetSurface(textureResId));
            }
            HRESULT result = app->GetD3DDevice()->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, D3DFVF_LVERTEX, vertexList, vertexCount, indexList, indexCount, 0);
        }
        game->app->GetD3DDevice()->SetTexture(0, 0);
        _ResetRenderState();
        _ResetWorldTransform();
    }
}

// 0x00419830, 296 bytes.
void XParticle_PacFireHit::ProcessFrame()
{
    localTime++;
    if (state == PROCESS) {
        float scaleValue = 0.2f;
        if (localTime < processFrame) {
            scaleValue = 1.0f - ((float)localTime / (float)processFrame) * 0.8f;
        }
        ScaleTo(scaleValue);
        if (localTime >= processFrame / 2) {
            renderCount = 1;
        }
        if (localTime >= processFrame) {
            state = NON;
            localTime = 0;
        }
    }
}


// 0x00419C60, 76 bytes.
XEnemyFireList::XEnemyFireList(XGameObject* _game)
    : game(_game)
{
    for (int i = 0; i < 60; i++) {
        fireArray[i] = 0;
    }
}

// 0x00419CB0, 75 bytes.
XEnemyFireList::~XEnemyFireList()
{
    Empty();
}

// 0x00419D00, 205 bytes.
void XEnemyFireList::CheckCollisionWithPac()
{
    if (game->pac->GetState() == XPac::NON) {
        return;
    }
    if (game->pac->GetState() == XPac::LEVEL_COMPLETED) {
        return;
    }
    if (game->pac->GetState() == XPac::DYING) {
        return;
    }
    for (int i = 0; i < 60; i++) {
        if (!fireArray[i]) {
            continue;
        }
        if (!fireArray[i]->GetState()) {
            continue;
        }
        if (game->pac->CheckCollisionRadius(fireArray[i])) {
            fireArray[i]->SetState(XEnemyFireBase::NON);
            game->pac->StartDying();
            break;
        }
    }
}

// 0x00419DD0, 111 bytes.
void XEnemyFireList::Empty()
{
    for (int i = 0; i < 60; i++) {
        if (fireArray[i]) {
            delete fireArray[i];
            fireArray[i] = 0;
        }
    }
}

// 0x00419E40, 160 bytes.
void XEnemyFireList::ProcessFrame()
{
    for (int i = 0; i < 60; i++) {
        if (fireArray[i]) {
            fireArray[i]->ProcessFrame();
            if (fireArray[i]->GetState() == XEnemyFireBase::NON) {
                delete fireArray[i];
                fireArray[i] = 0;
            }
        }
    }
}

// 0x00419EE0, 85 bytes.
void XEnemyFireList::Render(const D3DMATRIX& _viewMatrix)
{
    for (int i = 0; i < 60; i++) {
        if (fireArray[i]) {
            fireArray[i]->Render2(_viewMatrix);
        }
    }
}

// 0x00419F40, 403 bytes.
void XEnemyFireList::CreateCannonUpFire(D3DVECTOR pos)
{
    int index = GetNullIndex();
    if (index >= 0) {
        unsigned char volume = game->GetVolumeOfPosition(pos);
        game->app->GetSoundList()->Play(0xF4, volume, 0);
        XEnemyFire_CannonUp* fire = new XEnemyFire_CannonUp(game, pos);
        float dist = 45.0f;
        int angle = Random(360);
        D3DVECTOR dir;
        dir.y = 0.0f;
        dir.x = sinf((float)angle * 0.017453292f);
        dir.z = cosf((float)angle * 0.017453292f);
        fire->SetValues(dist, dir);
        fireArray[index] = fire;
    }
}

// 0x0041A0E0, 403 bytes.
void XEnemyFireList::CreateVolcanoFire(D3DVECTOR pos)
{
    int index = GetNullIndex();
    if (index >= 0) {
        unsigned char volume = game->GetVolumeOfPosition(pos);
        game->app->GetSoundList()->Play(0xF4, volume, 0);
        XEnemyFire_CannonUp* fire = new XEnemyFire_CannonUp(game, pos);
        float dist = 80.0f;
        int angle = Random(360);
        D3DVECTOR dir;
        dir.y = 0.0f;
        dir.x = sinf((float)angle * 0.017453292f);
        dir.z = cosf((float)angle * 0.017453292f);
        fire->SetValues(dist, dir);
        fireArray[index] = fire;
    }
}

// 0x0041A280, 299 bytes.
void XEnemyFireList::CreateCannonFire(D3DVECTOR pos, int dir)
{
    int index = GetNullIndex();
    if (index >= 0) {
        unsigned char volume = game->GetVolumeOfPosition(pos);
        game->app->GetSoundList()->Play(0xF3, volume, 0);
        XEnemyFire_Cannon* fire = new XEnemyFire_Cannon(game, pos);
        fire->SetValues(GetVectorFromDir(dir));
        fireArray[index] = fire;
    }
}

// 0x0041A3B0, 257 bytes.
void XEnemyFireList::CreateUFOFire(D3DVECTOR pos)
{
    int index = GetNullIndex();
    if (index >= 0) {
        unsigned char volume = game->GetVolumeOfPosition(pos);
        game->app->GetSoundList()->Play(0xF3, volume, 0);
        XEnemyFire_UFO* fire = new XEnemyFire_UFO(game, pos);
        fireArray[index] = fire;
    }
}

// 0x0041A4C0, 184 bytes.
void XEnemyFireList::CreateSteamFire(D3DVECTOR pos)
{
    int index = GetNullIndex();
    if (index >= 0) {
        XEnemyFire_Steam* fire = new XEnemyFire_Steam(game, pos);
        fireArray[index] = fire;
    }
}

// 0x0041A580, 60 bytes.
int XEnemyFireList::GetNullIndex() const
{
    for (int i = 0; i < 60; i++) {
        if (!fireArray[i]) {
            return i;
        }
    }
    return -1;
}

// 0x0041A5C0, 207 bytes.
XEnemyFireBase::XEnemyFireBase(XGameObject* _game, D3DVECTOR pos, float size)
    : JObject3D_Particle(_game->app, size, size),
      game(_game),
      state(NON),
      localTime(0),
      refrencePos(pos)
{
    SetWorldPos(pos);
    state = MOVE;
}

// 0x0041A690, 28 bytes.
XEnemyFireBase::~XEnemyFireBase()
{
}

// 0x0041A6B0, 37 bytes.
void XEnemyFireBase::Render2(const D3DMATRIX& _viewMatrix)
{
    if (state != NON) {
        JObject3D_Particle::Render2(_viewMatrix);
    }
}

// 0x0041A6E0, 20 bytes.
XEnemyFireBase::STATE XEnemyFireBase::GetState() const
{
    return state;
}

// 0x0041A700, 25 bytes.
void XEnemyFireBase::SetState(STATE _state)
{
    state = _state;
}

// 0x0041A720, 265 bytes.
XEnemyFire_CannonUp::XEnemyFire_CannonUp(XGameObject* _game, D3DVECTOR pos)
    : XEnemyFireBase(_game, pos, 18.0f),
      distance(0.0f),
      dirVector(0.0f, 0.0f, 0.0f)
{
    SetTexture(0xA9, 0.0f, 0.0f, 1.0f, 1.0f);
    SetCollisionRadius(7.0f);
}

// 0x0041A830, 28 bytes.
XEnemyFire_CannonUp::~XEnemyFire_CannonUp()
{
}

// 0x0041A850, 55 bytes.
void XEnemyFire_CannonUp::SetValues(float _distance, D3DVECTOR _dirVector)
{
    distance = _distance;
    dirVector = _dirVector;
}

// 0x0041A890, 391 bytes.
void XEnemyFire_CannonUp::ProcessFrame()
{
    localTime++;
    if (state == MOVE) {
        float curDist = (float)localTime / 200.0f * distance;
        float height = sinf((float)localTime / 200.0f * 3.1415927f) * 100.0f;
        D3DVECTOR moveVec = D3DVectorMultiply(dirVector, curDist);
        moveVec.y = height;
        SetWorldPos(refrencePos + moveVec);
        if (localTime % 8 == 0) {
            game->particleList->CreateEnemyFireSmokePurple(worldPos);
        }
        if (worldPos.y < 0.0f) {
            state = NON;
            localTime = 0;
        }
    }
}

// 0x0041AA20, 251 bytes.
XEnemyFire_Cannon::XEnemyFire_Cannon(XGameObject* _game, D3DVECTOR pos)
    : XEnemyFireBase(_game, pos, 18.0f),
      dirVector(0.0f, 0.0f, 0.0f)
{
    SetTexture(0xA9, 0.0f, 0.0f, 1.0f, 1.0f);
    SetCollisionRadius(7.0f);
}

// 0x0041AB20, 28 bytes.
XEnemyFire_Cannon::~XEnemyFire_Cannon()
{
}

// 0x0041AB40, 38 bytes.
void XEnemyFire_Cannon::SetValues(D3DVECTOR _dirVector)
{
    dirVector = _dirVector;
}

// 0x0041AB70, 312 bytes.
void XEnemyFire_Cannon::ProcessFrame()
{
    localTime++;
    if (state == MOVE) {
        const float speed = 1.6f;
        D3DVECTOR moveVec = D3DVectorMultiply(dirVector, 1.6f);
        ShiftWorldPos(moveVec);
        if (localTime % 8 == 0) {
            game->particleList->CreateEnemyFireSmokePurple(worldPos);
        }
        int mapX = game->map->ConvertPosXToMapX(worldPos.x);
        int mapZ = game->map->ConvertPosZToMapZ(worldPos.z);
        if (localTime > 15) {
            if (game->map->IsBlocked_Fire(mapX, mapZ)) {
                state = NON;
                localTime = 0;
            }
        }
    }
}

// 0x0041ACB0, 269 bytes.
XEnemyFire_UFO::XEnemyFire_UFO(XGameObject* _game, D3DVECTOR pos)
    : XEnemyFireBase(_game, pos, 17.0f),
      dirVector(0.0f, -1.0f, 0.0f),
      turned(0)
{
    SetTexture(0xB6, 0.0f, 0.0f, 1.0f, 1.0f);
    SetCollisionRadius(7.0f);
}

// 0x0041ADC0, 28 bytes.
XEnemyFire_UFO::~XEnemyFire_UFO()
{
}

// 0x0041ADE0, 721 bytes.
void XEnemyFire_UFO::ProcessFrame()
{
    localTime++;
    if (state == MOVE) {
        const float speed = 1.2f;
        D3DVECTOR moveVec = D3DVectorMultiply(dirVector, 1.2f);
        ShiftWorldPos(moveVec);
        if (localTime % 9 == 0) {
            game->particleList->CreateEnemyFireSmokeGreen(worldPos);
        }
        if (turned == 0) {
            if (worldPos.y < 20.0f) {
                int pacMapX = game->pac->GetMapPosX();
                int pacMapZ = game->pac->GetMapPosZ();
                int fireMapX = game->map->ConvertPosXToMapX(worldPos.x);
                int fireMapZ = game->map->ConvertPosZToMapZ(worldPos.z);
                int dx = abs(pacMapX - fireMapX);
                int dz = abs(pacMapZ - fireMapZ);
                int dir = -1;
                if (dx == 0 && dz == 0) {
                    dir = Random(4);
                } else if (dx > dz) {
                    if (pacMapX > fireMapX) {
                        dir = 1;
                    } else {
                        dir = 3;
                    }
                } else if (dx < dz) {
                    if (pacMapZ > fireMapZ) {
                        dir = 2;
                    } else {
                        dir = 0;
                    }
                } else {
                    if (Random(2) == 0) {
                        if (pacMapX > fireMapX) {
                            dir = 1;
                        } else {
                            dir = 3;
                        }
                    } else if (pacMapZ > fireMapZ) {
                        dir = 2;
                    } else {
                        dir = 0;
                    }
                }
                dirVector = GetVectorFromDir(dir);
                turned = 1;
            }
        } else {
            int mapX = game->map->ConvertPosXToMapX(worldPos.x);
            int mapZ = game->map->ConvertPosZToMapZ(worldPos.z);
            if (localTime > 15) {
                if (game->map->IsBlocked_Fire(mapX, mapZ)) {
                    state = NON;
                    localTime = 0;
                }
            }
        }
    }
}

// 0x0041B0C0, 230 bytes.
XEnemyFire_Steam::XEnemyFire_Steam(XGameObject* _game, D3DVECTOR pos)
    : XEnemyFireBase(_game, pos, 18.0f)
{
    SetAlphaState(ALPHA_TRANSPARENT);
    SetFilterState(FILTER_POINT);
    SetTexture(0xDC, 0.0f, 0.0f, 1.0f, 1.0f);
    SetCollisionRadius(7.0f);
}

// 0x0041B1B0, 28 bytes.
XEnemyFire_Steam::~XEnemyFire_Steam()
{
}

// 0x0041B1D0, 285 bytes.
void XEnemyFire_Steam::ProcessFrame()
{
    localTime++;
    if (state == MOVE) {
        float height = (float)localTime / 80.0f * 120.0f;
        SetWorldPos(JAXIS_Y, refrencePos.y + height);
        float alpha = (1.0f - (float)localTime / 80.0f) * 0.9f + 0.1f;
        SetAlphaFactor(alpha);
        if (localTime >= 80) {
            state = NON;
            localTime = 0;
        }
    }
}

