#include "XPac.h"
#include "XAppMain.h"
#include "XGame.h"
#include "XMap.h"
#include "XChara.h"
#include "JMidi.h"
#include "JSound.h"
#include "JStandard.h"
#include "XEnemy.h"
#include "D3DTextr.h"
#include <math.h>

// 0x00426A70, 253 bytes.
D3DVECTOR GetVectorFromDir(int dir)
{
    if (dir == 0) {
        return D3DVECTOR(0.0f, 0.0f, 1.0f);
    }
    if (dir == 1) {
        return D3DVECTOR(1.0f, 0.0f, 0.0f);
    }
    if (dir == 2) {
        return D3DVECTOR(0.0f, 0.0f, -1.0f);
    }
    if (dir == 3) {
        return D3DVECTOR(-1.0f, 0.0f, 0.0f);
    }
    return D3DVECTOR(0.0f, 0.0f, 0.0f);
}

// 0x00426B70, 76 bytes.
XPacFireList::XPacFireList(XGameObject* _game)
    : game(_game)
{
    for (int i = 0; i < 3; i++) {
        fireArray[i] = 0;
    }
}

// 0x00426BC0, 75 bytes.
XPacFireList::~XPacFireList()
{
    Empty();
}

// 0x00426C10, 312 bytes.
void XPacFireList::CreateFire()
{
    int index = GetNullIndex();
    if (index >= 0) {
        game->app->GetSoundList()->Play(0xEE, 100, 0);
        fireArray[index] = new XPacFire(game, game->pac->GetFaceDirection());
        fireArray[index]->SetWorldPos(game->pac->GetWorldPos());
        D3DVECTOR dirVec = GetVectorFromDir(game->pac->GetFaceDirection());
        D3DVECTOR offset = D3DVectorMultiply(dirVec, 20.0f);
        fireArray[index]->ShiftWorldPos(offset);
    }
}

// 0x00426D50, 111 bytes.
void XPacFireList::Empty()
{
    for (int i = 0; i < 3; i++) {
        if (fireArray[i]) {
            delete fireArray[i];
            fireArray[i] = 0;
        }
    }
}

// 0x00426DC0, 303 bytes.
void XPacFireList::CheckCollisionWithEnemy()
{
    for (int i = 0; i < 3; i++) {
        if (!fireArray[i]) {
            continue;
        }
        if (fireArray[i]->GetState() == XPacFire::NON) {
            continue;
        }
        for (int j = 0; j < 30; j++) {
            XEnemyBase* enemy = game->enemyList->GetAt(j);
            if (!enemy) {
                continue;
            }
            if (enemy->GetState() == XEnemyBase::NON) {
                continue;
            }
            if (enemy->GetState() == XEnemyBase::DYING) {
                continue;
            }
            if (enemy->CheckCollisionRadius(fireArray[i])) {
                if (enemy->Breakable()) {
                    enemy->DecreaseShield(fireArray[i]->GetDirection());
                    game->particleList->CreatePacFireHit(fireArray[i]->GetWorldPos());
                }
                fireArray[i]->SetState(XPacFire::NON);
            }
        }
    }
}

// 0x00426EF0, 145 bytes.
void XPacFireList::ProcessFrame()
{
    for (int i = 0; i < 3; i++) {
        if (fireArray[i]) {
            fireArray[i]->ProcessFrame();
            if (fireArray[i]->GetState() == XPacFire::NON) {
                delete fireArray[i];
                fireArray[i] = 0;
            }
        }
    }
}

// 0x00426F90, 85 bytes.
void XPacFireList::Render(const D3DMATRIX& _viewMatrix)
{
    for (int i = 0; i < 3; i++) {
        if (fireArray[i]) {
            fireArray[i]->Render2(_viewMatrix);
        }
    }
}

// 0x00426FF0, 60 bytes.
int XPacFireList::GetNullIndex() const
{
    for (int i = 0; i < 3; i++) {
        if (fireArray[i] == 0) {
            return i;
        }
    }
    return -1;
}

// 0x00427030, 209 bytes.
XPacFire::XPacFire(XGameObject* _game, int dir)
    : JObject3D_Particle(_game->app, 24.0f, 24.0f),
      game(_game),
      state(NON),
      localTime(0),
      direction(dir)
{
    SetCollisionRadius(8.0f);
    state = MOVE;
}

// 0x00427110, 28 bytes.
XPacFire::~XPacFire()
{
}

// 0x00427130, 360 bytes.
void XPacFire::Render2(const D3DMATRIX& _viewMatrix)
{
    if (state != NON) {
        _SetWorldTransform2(_viewMatrix);
        _SetRenderState();
        for (int i = 0; i < 3; i++) {
            if (i == 0) {
                SetTexture(0xA1, 0.0f, 0.0f, 1.0f, 1.0f);
            } else {
                _SetTextureRandomly();
            }
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

// 0x004272A0, 20 bytes.
XPacFire::STATE XPacFire::GetState() const
{
    return state;
}

// 0x004272C0, 25 bytes.
void XPacFire::SetState(STATE _state)
{
    state = _state;
}

// 0x004272E0, 20 bytes.
int XPacFire::GetDirection() const
{
    return direction;
}

// 0x00427300, 247 bytes.
void XPacFire::ProcessFrame()
{
    localTime++;
    if (state == MOVE) {
        const float speed = 2.5f;
        D3DVECTOR offset = D3DVectorMultiply(GetVectorFromDir(direction), speed);
        ShiftWorldPos(offset);
        int mapX = game->map->ConvertPosXToMapX(worldPos.x);
        int mapZ = game->map->ConvertPosZToMapZ(worldPos.z);
        if (game->map->IsBlocked_Fire(mapX, mapZ)) {
            state = NON;
            localTime = 0;
        }
    }
}

// 0x00427400, 766 bytes.
void XPacFire::_SetTextureRandomly()
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

// 0x00427700, 522 bytes.
XPac::XPac(XGameObject* _game)
    : JObject3DX(_game->app, 0x94),
      game(_game),
      state(NON),
      localTime(0),
      mapPosX(-1),
      mapPosZ(-1),
      faceDirection(0),
      moveDirection(0),
      refrencePos(0.0f, 0.0f, 0.0f),
      shadow(new JObject3D_Shadow(_game->app, 40.0f, 40.0f)),
      pacMoveFrame(0),
      fireEnergy(0)
{
    AppendExtraFrame(0x95);
    AppendExtraFrame(0x96);
    AppendExtraFrame(199);
    RotateBy(JAXIS_X, -1.5707964f);
    ScaleBy(45.0f);
    SetCollisionRadius(13.0f);
    shadow->SetTexture(0xA3, 0.0f, 0.0f, 1.0f, 1.0f);
}

// 0x00427910, 99 bytes.
XPac::~XPac()
{
    if (shadow) {
        delete shadow;
        shadow = 0;
    }
}

// 0x00427980, 120 bytes.
void XPac::Render()
{
    if (state != NON) {
        JObject3DX::Render();
        shadow->SetWorldPos(D3DVECTOR(worldPos.x, 1.0f, worldPos.z));
        shadow->Render();
    }
}

// 0x00427A00, 112 bytes.
void XPac::Reset()
{
    ShiftWorldPos_OnGround();
    ShiftWorldPos(JAXIS_Y, 3.0f);
    RotateTo(JAXIS_Y, 0.0f);
    faceDirection = 0;
    moveDirection = 0;
    SetCurrentFrame(0);
    state = WAIT;
    localTime = 0;
}

// 0x00427A70, 17 bytes.
XPac::STATE XPac::GetState() const
{
    return state;
}

// 0x00427A90, 17 bytes.
int XPac::GetFaceDirection() const
{
    return faceDirection;
}

// 0x00427AB0, 31 bytes.
void XPac::SetMapPos(int x, int z)
{
    mapPosX = x;
    mapPosZ = z;
}

// 0x00427AD0, 17 bytes.
int XPac::GetMapPosX() const
{
    return mapPosX;
}

// 0x00427AF0, 17 bytes.
int XPac::GetMapPosZ() const
{
    return mapPosZ;
}

// 0x00427B10, 20 bytes.
int XPac::GetFireEnergy() const
{
    return fireEnergy;
}

// 0x00427B30, 24 bytes.
void XPac::ResetFireEnergy()
{
    fireEnergy = 0;
}

// 0x00427B50, 143 bytes.
void XPac::SetFaceDirection(int dir)
{
    faceDirection = dir;
    if (dir == 0) {
        RotateTo(JAXIS_Y, 0.0f);
    } else if (dir == 1) {
        RotateTo(JAXIS_Y, 1.5707964f);
    } else if (dir == 2) {
        RotateTo(JAXIS_Y, 3.1415927f);
    } else if (dir == 3) {
        RotateTo(JAXIS_Y, 4.712389f);
    }
}

// 0x00427BE0, 295 bytes.
void XPac::StartMoving(int dir)
{
    if (dir < 0) {
        return;
    }
    if (state == WAIT) {
        SetFaceDirection(dir);
        moveDirection = dir;
        int mapX = mapPosX;
        int mapZ = mapPosZ;
        if (dir == 0) {
            mapZ = mapZ - 1;
        } else if (dir == 1) {
            mapX = mapX + 1;
        } else if (dir == 2) {
            mapZ = mapZ + 1;
        } else if (dir == 3) {
            mapX = mapX - 1;
        }
        if (!game->map->IsBlocked_Pac(mapX, mapZ)) {
            game->app->GetSoundList()->Play(0xFF, 100, 0);
            refrencePos = worldPos;
            mapPosX = mapX;
            mapPosZ = mapZ;
            pacMoveFrame = game->crystalList->GetTakenCount() * 3 + 40;
            state = MOVE;
            localTime = 0;
        }
    }
}

// 0x00427D10, 234 bytes.
void XPac::StartArrow(int dir)
{
    if (dir < 0) {
        return;
    }
    if (state == WAIT) {
        moveDirection = dir;
        int mapX = mapPosX;
        int mapZ = mapPosZ;
        if (dir == 0) {
            mapZ = mapZ - 1;
        } else if (dir == 1) {
            mapX = mapX + 1;
        } else if (dir == 2) {
            mapZ = mapZ + 1;
        } else if (dir == 3) {
            mapX = mapX - 1;
        }
        if (!game->map->IsBlocked_Pac(mapX, mapZ)) {
            SetCurrentFrame(3);
            refrencePos = worldPos;
            mapPosX = mapX;
            mapPosZ = mapZ;
            state = ON_ARROW;
            localTime = 0;
        }
    }
}

// 0x00427E00, 176 bytes.
void XPac::StartDying()
{
    if (state == NON) {
        return;
    }
    if (state == DYING) {
        return;
    }
    game->app->GetMidiList()->Stop();
    game->app->GetSoundList()->Play(0xEF, 100, 0);
    game->camera->StartGameOver();
    game->crystalList->StartGameOver();
    SetCurrentFrame(3);
    refrencePos = worldPos;
    state = DYING;
    localTime = 0;
}

// 0x00427EB0, 352 bytes.
void XPac::ProcessRotate(unsigned long frameCount)
{
    if (state == LEVEL_COMPLETED) {
        const float angleStep = 0.06981317f;
        float angle = (float)frameCount * angleStep;
        RotateBy(JAXIS_Y, angle);
    } else if (state == DYING) {
        const float angleStep = 0.13962634f;
        float angle = (float)frameCount * angleStep;
        if (faceDirection == 0) {
            RotateBy(JAXIS_X, -angle);
        } else if (faceDirection == 2) {
            RotateBy(JAXIS_X, angle);
        } else if (faceDirection == 1) {
            RotateBy(JAXIS_Z, angle);
        } else if (faceDirection == 3) {
            RotateBy(JAXIS_Z, -angle);
        }
    }
}

// 0x00428010, 1597 bytes.
void XPac::ProcessFrame()
{
    localTime++;
    if (fireEnergy < 1000) {
        fireEnergy += 3;
        if (fireEnergy > 1000) {
            game->app->GetSoundList()->Play(0xFE, 100, 0);
            fireEnergy = 1000;
        }
    }

    if (state == MOVE) {
        float dist = 0.0f;
        float height = 0.0f;
        if (localTime < pacMoveFrame) {
            dist = ((float)localTime / (float)pacMoveFrame) * game->map->GetChipSize();
            height = sinf(((float)localTime / (float)pacMoveFrame) * 3.1415927f) * 15.0f;
            unsigned int frameStep = pacMoveFrame / 5;
            if (localTime < frameStep * 2) {
                SetCurrentFrame(1);
            } else if (localTime < frameStep * 3) {
                SetCurrentFrame(0);
            } else {
                SetCurrentFrame(2);
            }
        } else {
            dist = game->map->GetChipSize();
            height = 0.0f;
            SetCurrentFrame(0);
        }
        D3DVECTOR dirVec = GetVectorFromDir(moveDirection);
        D3DVECTOR offset = D3DVectorMultiply(dirVec, dist);
        offset.y = height;
        SetWorldPos(refrencePos + offset);
        if (localTime >= pacMoveFrame) {
            state = WAIT;
            localTime = 0;
        }
    } else if (state == ON_ARROW) {
        float dist = 0.0f;
        if (localTime < 20) {
            dist = ((float)localTime / 20.0f) * game->map->GetChipSize();
        } else {
            dist = game->map->GetChipSize();
        }
        D3DVECTOR dirVec = GetVectorFromDir(moveDirection);
        D3DVECTOR offset = D3DVectorMultiply(dirVec, dist);
        SetWorldPos(refrencePos + offset);
        if (localTime >= 20) {
            SetCurrentFrame(0);
            state = WAIT;
            localTime = 0;
        }
    } else if (state == LEVEL_COMPLETED) {
        ShiftWorldPos(JAXIS_Y, 0.7f);
        if (worldPos.y > 500.0f) {
            game->particleList->CreatePacVanish(worldPos);
            game->StartProcess(1);
            state = NON;
        }
    } else if (state == DYING) {
        float height = sinf(((float)localTime / 200.0f) * 3.1415927f) * 140.0f;
        D3DVECTOR offset(0.0f, height, 0.0f);
        SetWorldPos(refrencePos + offset);
        if (worldPos.y - collisionRadius < 0.0f) {
            game->particleList->CreateEnemyExplode(worldPos);
            game->StartProcess(2);
            state = NON;
        }
    }

    if (state == WAIT) {
        if (game->map->IsWarpZone(mapPosX, mapPosZ)) {
            if (localTime > 0 && localTime % 80 == 0) {
                if (game->crystalList->GetCrystalLeft() == 0) {
                    game->app->GetSoundList()->Play(0xF0, 100, 0);
                    game->camera->StartLevelCompleted();
                    SetCurrentFrame(3);
                    state = LEVEL_COMPLETED;
                    localTime = 0;
                } else {
                    game->crystalList->DoVanishCrystal();
                }
            }
            return;
        }
        if (game->map->IsArrow(mapPosX, mapPosZ)) {
            StartArrow(game->map->GetArrowDirection(mapPosX, mapPosZ));
        }
    }
}
