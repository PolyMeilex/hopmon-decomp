#include "XEnemy.h"
#include "XAppMain.h"
#include "XGame.h"
#include "XMap.h"
#include "XPac.h"
#include "XChara.h"
#include "JStandard.h"
#include "JList.h"
#include "JSound.h"
#include <math.h>
#include <stdlib.h>

// 0x0041B670, 76 bytes.
XEnemyList::XEnemyList(XGameObject* _game)
    : game(_game)
{
    for (int i = 0; i < 30; i++) {
        enemyArray[i] = 0;
    }
}

// 0x0041B6C0, 75 bytes.
XEnemyList::~XEnemyList()
{
    Empty();
}

// 0x0041B710, 1419 bytes.
XEnemyBase* XEnemyList::CreateEnemy(unsigned char type)
{
    int index = GetNullIndex();
    if (index >= 0) {
        if (type == 'a') {
            enemyArray[index] = new XEnemy_Ghost(game);
        } else if (type == 'b') {
            enemyArray[index] = new XEnemy_Cube(game);
        } else if (type == 'c') {
            enemyArray[index] = new XEnemy_Kurage(game);
        } else if (type == 'd') {
            enemyArray[index] = new XEnemy_UFO(game);
        } else if (type == 'e') {
            enemyArray[index] = new XEnemy_Sesame(game);
        } else if (type == 'm') {
            enemyArray[index] = new XNeedleBall(game, 0);
        } else if (type == 'n') {
            enemyArray[index] = new XNeedleBall(game, 2);
        } else if (type == 'o') {
            enemyArray[index] = new XNeedleBallFast(game, 0);
        } else if (type == 'p') {
            enemyArray[index] = new XNeedleBallFast(game, 2);
        } else if (type == 'q') {
            enemyArray[index] = new XNeedleBox(game);
        } else if (type == 'r') {
            enemyArray[index] = new XNeedleBoxDrop(game);
        } else if (type == 'x') {
            enemyArray[index] = new XGate(game, 0);
        } else if (type == 'y') {
            enemyArray[index] = new XGate(game, 1);
        }
        return enemyArray[index];
    }
    return 0;
}

// 0x0041BCA0, 23 bytes.
XEnemyBase* XEnemyList::GetAt(unsigned long index) const
{
    return enemyArray[index];
}

// 0x0041BCC0, 111 bytes.
void XEnemyList::Empty()
{
    for (int i = 0; i < 30; i++) {
        if (enemyArray[i]) {
            delete enemyArray[i];
            enemyArray[i] = 0;
        }
    }
}

// 0x0041BD30, 213 bytes.
void XEnemyList::CheckCollisionWithPac()
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
    for (int i = 0; i < 30; i++) {
        if (!enemyArray[i]) {
            continue;
        }
        if (!enemyArray[i]->GetState()) {
            continue;
        }
        if (enemyArray[i]->GetState() == XEnemyBase::DYING) {
            continue;
        }
        if (game->pac->CheckCollisionRadius(enemyArray[i])) {
            game->pac->StartDying();
            break;
        }
    }
}

// 0x0041BE10, 73 bytes.
void XEnemyList::ProcessRotate(unsigned long frameCount)
{
    for (int i = 0; i < 30; i++) {
        if (enemyArray[i]) {
            enemyArray[i]->ProcessRotate(frameCount);
        }
    }
}

// 0x0041BE60, 145 bytes.
void XEnemyList::ProcessFrame()
{
    for (int i = 0; i < 30; i++) {
        if (enemyArray[i]) {
            enemyArray[i]->ProcessFrame();
            if (enemyArray[i]->GetState() == XEnemyBase::NON) {
                delete enemyArray[i];
                enemyArray[i] = 0;
            }
        }
    }
}

// 0x0041BF00, 79 bytes.
void XEnemyList::Render()
{
    for (int i = 0; i < 30; i++) {
        if (enemyArray[i]) {
            enemyArray[i]->Render();
        }
    }
}

// 0x0041BF50, 60 bytes.
int XEnemyList::GetNullIndex() const
{
    for (int i = 0; i < 30; i++) {
        if (enemyArray[i] == 0) {
            return i;
        }
    }
    return -1;
}

// 0x0041BF90, 428 bytes.
XEnemyBase::XEnemyBase(XGameObject* _game, unsigned short resId)
    : JObject3DX(_game->app, resId), game(_game), state(NON), localTime(0),
      mapPosX(-1), mapPosZ(-1), refrencePos(0.0f, 0.0f, 0.0f), direction(-1),
      shadow(new JObject3D_Shadow(_game->app, 1.0f, 1.0f)), faceToDir(1),
      breakable(1), shield(1)
{
    shadow->SetTexture(0xA3, 0.0f, 0.0f, 1.0f, 1.0f);
    state = WAIT;
}

// 0x0041C140, 99 bytes.
XEnemyBase::~XEnemyBase()
{
    if (shadow) {
        delete shadow;
        shadow = 0;
    }
}

// 0x0041C1B0, 120 bytes.
void XEnemyBase::Render()
{
    if (state != NON) {
        JObject3DX::Render();
        shadow->SetWorldPos(D3DVECTOR(worldPos.x, 1.0f, worldPos.z));
        shadow->Render();
    }
}

// 0x0041C230, 17 bytes.
XEnemyBase::STATE XEnemyBase::GetState() const
{
    return state;
}

// 0x0041C250, 20 bytes.
int XEnemyBase::Breakable() const
{
    return breakable;
}

// 0x0041C270, 31 bytes.
void XEnemyBase::SetMapPos(int x, int z)
{
    mapPosX = x;
    mapPosZ = z;
}

// 0x0041C290, 155 bytes.
void XEnemyBase::SetDirection(int dir)
{
    if (faceToDir) {
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
    direction = dir;
}

// 0x0041C330, 226 bytes.
void XEnemyBase::StartMoving(int dir)
{
    if (state == WAIT) {
        int newMapPosX = mapPosX;
        int newMapPosZ = mapPosZ;
        if (dir == 0) {
            newMapPosZ = newMapPosZ - 1;
        } else if (dir == 1) {
            newMapPosX = newMapPosX + 1;
        } else if (dir == 2) {
            newMapPosZ = newMapPosZ + 1;
        } else if (dir == 3) {
            newMapPosX = newMapPosX - 1;
        }
        game->map->UpdateEnemyMap(mapPosX, mapPosZ, newMapPosX, newMapPosZ);
        mapPosX = newMapPosX;
        mapPosZ = newMapPosZ;
        SetDirection(dir);
        refrencePos = worldPos;
        state = MOVE;
        localTime = 0;
    }
}

// 0x0041C420, 126 bytes.
void XEnemyBase::StartDying(int dir)
{
    game->map->ZeroEnemyMap(mapPosX, mapPosZ);
    SetCurrentFrame(0);
    SetDirection((dir + 2) % 4);
    refrencePos = worldPos;
    state = DYING;
    localTime = 0;
}

// 0x0041C4A0, 59 bytes.
void XEnemyBase::StartFiring()
{
    refrencePos = worldPos;
    state = FIRE;
    localTime = 0;
}

// 0x0041C4E0, 58 bytes.
void XEnemyBase::DecreaseShield(int dir)
{
    shield--;
    if (shield <= 0) {
        StartDying(dir);
    }
}

// 0x0041C520, 95 bytes.
void XEnemyBase::ProcessRotate(unsigned long frameCount)
{
    if (state == DYING) {
        SubProcessDyingRotate(frameCount);
    } else if (state == MOVE) {
        SubProcessMoveRotate(frameCount);
    } else if (state == FIRE) {
        SubProcessFireRotate(frameCount);
    }
}

// 0x0041C580, 237 bytes.
void XEnemyBase::SubProcessDyingRotate(unsigned long frameCount)
{
    const float angleStep = 0.13962634f;
    float angle = (float)frameCount * angleStep;
    if (direction == 0) {
        RotateBy(JAXIS_X, -angle);
    } else if (direction == 2) {
        RotateBy(JAXIS_X, angle);
    } else if (direction == 1) {
        RotateBy(JAXIS_Z, angle);
    } else if (direction == 3) {
        RotateBy(JAXIS_Z, -angle);
    }
}

// 0x0041C670, 13 bytes.
void XEnemyBase::SubProcessMoveRotate(unsigned long frameCount)
{
}

// 0x0041C680, 13 bytes.
void XEnemyBase::SubProcessFireRotate(unsigned long frameCount)
{
}

// 0x0041C690, 118 bytes.
void XEnemyBase::ProcessFrame()
{
    localTime++;
    if (state == DYING) {
        SubProcessDying();
    } else if (state == MOVE) {
        SubProcessMove();
    } else if (state == FIRE) {
        SubProcessFire();
    }
    if (state == WAIT) {
        SubProcessWait();
    }
}

// 0x0041C710, 257 bytes.
void XEnemyBase::SubProcessDying()
{
    float height = sinf((float)localTime / 160.0f * 3.1415927f) * 120.0f;
    D3DVECTOR offset(0.0f, height, 0.0f);
    SetWorldPos(refrencePos + offset);
    if (worldPos.y - collisionRadius < 0.0f) {
        game->particleList->CreateEnemyExplode(worldPos);
        state = NON;
        localTime = 0;
    }
}

// 0x0041C820, 11 bytes.
void XEnemyBase::SubProcessMove()
{
}

// 0x0041C830, 11 bytes.
void XEnemyBase::SubProcessWait()
{
}

// 0x0041C840, 11 bytes.
void XEnemyBase::SubProcessFire()
{
}

// 0x0041C850, 121 bytes.
int XEnemyBase::IsBlocked(int dir) const
{
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
    return game->map->IsBlocked_Enemy(mapX, mapZ);
}

// 0x0041C8D0, 275 bytes.
int XEnemyBase::FindDirectionToPac() const
{
    int pacX = game->pac->GetMapPosX();
    int pacZ = game->pac->GetMapPosZ();
    int dx = abs(pacX - mapPosX);
    int dz = abs(pacZ - mapPosZ);
    int dir = -1;
    if (dx == 0 && dz == 0) {
        dir = -1;
    } else if (dx > dz) {
        if (pacX > mapPosX) {
            dir = 1;
        } else {
            dir = 3;
        }
    } else if (dx < dz) {
        if (pacZ > mapPosZ) {
            dir = 2;
        } else {
            dir = 0;
        }
    } else if (dx == dz) {
        if (Random(2) == 0) {
            if (pacX > mapPosX) {
                dir = 1;
            } else {
                dir = 3;
            }
        } else if (pacZ > mapPosZ) {
            dir = 2;
        } else {
            dir = 0;
        }
    }
    return dir;
}

// 0x0041C9F0, 406 bytes.
void XEnemyBase::DefaultProcessWait()
{
    JBaseList possibleDirs;
    int dirRight = (direction + 1) % 4;
    int dirBack = (direction + 2) % 4;
    int dirLeft = (direction + 3) % 4;

    if (!IsBlocked(direction)) {
        possibleDirs.Append((void*)direction);
        possibleDirs.Append((void*)direction);
    }
    if (!IsBlocked(dirRight)) {
        possibleDirs.Append((void*)dirRight);
    }
    if (!IsBlocked(dirLeft)) {
        possibleDirs.Append((void*)dirLeft);
    }
    int dirToPac = FindDirectionToPac();
    if (dirToPac >= 0 && dirToPac != dirBack && !IsBlocked(dirToPac)) {
        possibleDirs.Append((void*)dirToPac);
    }
    int count = possibleDirs.Count();
    if (count <= 0) {
        if (!IsBlocked(dirBack)) {
            StartMoving(dirBack);
        }
    } else {
        unsigned long index = Random(count);
        int dirPicked = (int)possibleDirs.GetData(index);
        StartMoving(dirPicked);
    }
}

// 0x0041CB90, 282 bytes.
XEnemy_Ghost::XEnemy_Ghost(XGameObject* _game)
    : XEnemyBase(_game, 0x9D)
{
    shadow->ScaleBy(40.0f);
    RotateBy(JAXIS_X, -1.5707964f);
    ScaleBy(50.0f);
    ShiftWorldPos_OnGround();
    ShiftWorldPos(JAXIS_Y, 9.0f);
    SetCollisionRadius(15.0f);
    faceToDir = 1;
    SetDirection(Random(4));
    shield = 1;
}

// 0x0041CCB0, 28 bytes.
XEnemy_Ghost::~XEnemy_Ghost()
{
}

// 0x0041CCD0, 239 bytes.
void XEnemy_Ghost::SubProcessMove()
{
    float distance = 0.0f;
    if (localTime < 60) {
        distance = (float)localTime / 60.0f * game->map->GetChipSize();
    } else {
        distance = game->map->GetChipSize();
    }
    D3DVECTOR moveVec = D3DVectorMultiply(GetVectorFromDir(direction), distance);
    SetWorldPos(refrencePos + moveVec);
    if (localTime >= 60) {
        state = WAIT;
        localTime = 0;
    }
}

// 0x0041CDC0, 19 bytes.
void XEnemy_Ghost::SubProcessWait()
{
    DefaultProcessWait();
}

// 0x0041CDE0, 305 bytes.
XEnemy_Cube::XEnemy_Cube(XGameObject* _game)
    : XEnemyBase(_game, 0x149)
{
    AppendExtraFrame(0xA4);
    SetCurrentFrame(1);
    shadow->ScaleBy(40.0f);
    RotateBy(JAXIS_X, -1.5707964f);
    ScaleBy(63.0f);
    ShiftWorldPos_OnGround();
    ShiftWorldPos(JAXIS_Y, 10.0f);
    SetCollisionRadius(15.0f);
    faceToDir = 0;
    SetDirection(Random(4));
    shield = 2;
}

// 0x0041CF20, 28 bytes.
XEnemy_Cube::~XEnemy_Cube()
{
}

// 0x0041CF40, 237 bytes.
void XEnemy_Cube::SubProcessMoveRotate(unsigned long frameCount)
{
    const float rot = 0.02685122f;
    float angle = (float)frameCount * rot;
    if (direction == 0) {
        RotateBy(JAXIS_X, angle);
    } else if (direction == 2) {
        RotateBy(JAXIS_X, -angle);
    } else if (direction == 1) {
        RotateBy(JAXIS_Z, -angle);
    } else if (direction == 3) {
        RotateBy(JAXIS_Z, angle);
    }
}

// 0x0041D030, 279 bytes.
void XEnemy_Cube::SubProcessMove()
{
    float distance = 0.0f;
    if (localTime < 65) {
        distance = (float)localTime / 65.0f * game->map->GetChipSize();
    } else {
        distance = game->map->GetChipSize();
    }
    D3DVECTOR moveVec = D3DVectorMultiply(GetVectorFromDir(direction), distance);
    SetWorldPos(refrencePos + moveVec);
    SetCurrentFrame((shield <= 1) ? 0 : 1);
    if (localTime >= 65) {
        state = WAIT;
        localTime = 0;
    }
}

// 0x0041D150, 19 bytes.
void XEnemy_Cube::SubProcessWait()
{
    DefaultProcessWait();
}

// 0x0041D170, 295 bytes.
XEnemy_Kurage::XEnemy_Kurage(XGameObject* _game)
    : XEnemyBase(_game, 0xAF)
{
    AppendExtraFrame(0xAE);
    shadow->ScaleBy(40.0f);
    RotateBy(JAXIS_X, -1.5707964f);
    ScaleBy(80.0f);
    ShiftWorldPos_OnGround();
    ShiftWorldPos(JAXIS_Y, 3.0f);
    SetCollisionRadius(15.0f);
    faceToDir = 0;
    SetDirection(Random(4));
    shield = 1;
}

// 0x0041D2A0, 28 bytes.
XEnemy_Kurage::~XEnemy_Kurage()
{
}

// 0x0041D2C0, 379 bytes.
void XEnemy_Kurage::SubProcessMove()
{
    float distance = 0.0f;
    float verticalOffset = 0.0f;
    if (localTime < 80) {
        distance = (float)localTime / 80.0f * game->map->GetChipSize();
        verticalOffset = sinf((float)localTime / 80.0f * 3.1415927f) * 47.0f;
        SetCurrentFrame(1);
    } else {
        distance = game->map->GetChipSize();
        verticalOffset = 0.0f;
        SetCurrentFrame(0);
    }
    D3DVECTOR moveVec = D3DVectorMultiply(GetVectorFromDir(direction), distance);
    moveVec.y = verticalOffset;
    SetWorldPos(refrencePos + moveVec);
    if (localTime >= 80) {
        state = WAIT;
        localTime = 0;
    }
}

// 0x0041D440, 28 bytes.
void XEnemy_Kurage::SubProcessWait()
{
    if (localTime > 20) {
        DefaultProcessWait();
    }
}

// 0x0041DB00, 379 bytes.
XEnemy_Sesame::XEnemy_Sesame(XGameObject* _game)
    : XEnemyBase(_game, 0x14A), moveCountDown(0), movePhase(0)
{
    AppendExtraFrame(0x14B);
    AppendExtraFrame(0xD2);
    AppendExtraFrame(0xD4);
    SetCurrentFrame(2);
    shadow->ScaleBy(40.0f);
    RotateBy(JAXIS_X, -1.5707964f);
    ScaleBy(53.0f);
    ShiftWorldPos_OnGround();
    ShiftWorldPos(JAXIS_Y, 9.0f);
    SetCollisionRadius(15.0f);
    faceToDir = 1;
    SetDirection(Random(4));
    shield = 2;
    moveCountDown = Random(15) + 15;
}

// 0x0041DC80, 28 bytes.
XEnemy_Sesame::~XEnemy_Sesame()
{
}

// 0x0041DCA0, 422 bytes.
void XEnemy_Sesame::SubProcessMove()
{
    int frameBase;
    if (shield > 1) {
        frameBase = 2;
    } else {
        frameBase = 0;
    }
    int frame = frameBase;

    unsigned int moveDuration = 0;
    if (movePhase == 0) {
        moveDuration = 0x46;
        if (localTime < moveDuration / 2) {
            SetCurrentFrame(frame);
        } else {
            SetCurrentFrame(frame + 1);
        }
    } else {
        moveDuration = 0x19;
        SetCurrentFrame(frame + 1);
    }

    float distance = 0.0f;
    if (localTime < moveDuration) {
        distance = (float)localTime / (float)moveDuration * game->map->GetChipSize();
    } else {
        distance = game->map->GetChipSize();
    }
    D3DVECTOR moveVec = D3DVectorMultiply(GetVectorFromDir(direction), distance);
    SetWorldPos(refrencePos + moveVec);
    if (localTime >= moveDuration) {
        moveCountDown--;
        state = WAIT;
        localTime = 0;
    }
}

// 0x0041DE50, 218 bytes.
void XEnemy_Sesame::SubProcessWait()
{
    if (movePhase == 0) {
        if (moveCountDown <= 0) {
            movePhase = 1;
        } else {
            DefaultProcessWait();
        }
    } else if (movePhase == 1) {
        int frameBase;
        if (shield > 1) {
            frameBase = 2;
        } else {
            frameBase = 0;
        }
        int frame = frameBase;
        SetCurrentFrame(frame);
        if (localTime >= 0xFA) {
            moveCountDown = 10;
            movePhase = 2;
        }
    } else {
        if (moveCountDown <= 0) {
            moveCountDown = Random(0xF) + 0xF;
            movePhase = 0;
        }
        DefaultProcessWait();
    }
}

// 0x0041D460, 330 bytes.
XEnemy_UFO::XEnemy_UFO(XGameObject* _game)
    : XEnemyBase(_game, 0xAD), moveCount(0), countDownToFire(0)
{
    shadow->ScaleBy(50.0f);
    RotateBy(JAXIS_X, -1.5707964f);
    ScaleBy(60.0f);
    ShiftWorldPos_OnGround();
    ShiftWorldPos(JAXIS_Y, 100.0f);
    SetCollisionRadius(18.0f);
    faceToDir = 0;
    SetDirection(Random(4));
    breakable = 0;
    countDownToFire = Random(6) + 3;
}

// 0x0041D5B0, 28 bytes.
XEnemy_UFO::~XEnemy_UFO()
{
}

// 0x0041D5D0, 103 bytes.
void XEnemy_UFO::SubProcessMoveRotate(unsigned long frameCount)
{
    const float angleStep = 0.017453292f;
    float angle = (float)frameCount * angleStep;
    RotateBy(JAXIS_Y, angle);
}

// 0x0041D640, 103 bytes.
void XEnemy_UFO::SubProcessFireRotate(unsigned long frameCount)
{
    const float angleStep = 0.04363323f;
    float angle = (float)frameCount * angleStep;
    RotateBy(JAXIS_Y, angle);
}

// 0x0041D6B0, 260 bytes.
void XEnemy_UFO::SubProcessMove()
{
    float distance = 0.0f;
    if (localTime < 100) {
        distance = (float)localTime / 100.0f * game->map->GetChipSize();
    } else {
        distance = game->map->GetChipSize();
    }
    D3DVECTOR moveVec = D3DVectorMultiply(GetVectorFromDir(direction), distance);
    SetWorldPos(refrencePos + moveVec);
    if (localTime >= 100) {
        moveCount++;
        state = WAIT;
        localTime = 0;
    }
}

// 0x0041D7C0, 350 bytes.
void XEnemy_UFO::SubProcessWait()
{
    int targetDir = direction;
    if (moveCount >= 3) {
        moveCount = 0;
        countDownToFire--;
        if (countDownToFire <= 0) {
            countDownToFire = Random(5) + 2;
            if (!game->map->IsBlocked_Fire(mapPosX, mapPosZ)) {
                unsigned char volume = game->GetVolumeOfPosition(worldPos);
                game->app->GetSoundList()->Play(0x13F, volume, 0);
                StartFiring();
                return;
            }
        }
        if (Random(2) == 0) {
            int pacDir = FindDirectionToPac();
            if (pacDir >= 0) {
                targetDir = pacDir;
            } else {
                targetDir = Random(4);
            }
        } else {
            targetDir = Random(4);
        }
    }
    if (IsBlocked_UFO(targetDir)) {
        targetDir = Random(4);
    }
    if (!IsBlocked_UFO(targetDir)) {
        StartMoving_UFO(targetDir);
    }
}

// 0x0041D920, 98 bytes.
void XEnemy_UFO::SubProcessFire()
{
    if (localTime == 0xAA) {
        game->enemyFireList->CreateUFOFire(worldPos);
    } else if (localTime >= 0x12C) {
        state = WAIT;
        localTime = 0;
    }
}

// 0x0041D990, 226 bytes.
void XEnemy_UFO::StartMoving_UFO(int dir)
{
    if (state == WAIT) {
        int newMapPosX = mapPosX;
        int newMapPosZ = mapPosZ;
        if (dir == 0) {
            newMapPosZ = newMapPosZ - 1;
        } else if (dir == 1) {
            newMapPosX = newMapPosX + 1;
        } else if (dir == 2) {
            newMapPosZ = newMapPosZ + 1;
        } else if (dir == 3) {
            newMapPosX = newMapPosX - 1;
        }
        game->map->UpdateUFOMap(mapPosX, mapPosZ, newMapPosX, newMapPosZ);
        mapPosX = newMapPosX;
        mapPosZ = newMapPosZ;
        SetDirection(dir);
        refrencePos = worldPos;
        state = MOVE;
        localTime = 0;
    }
}

// 0x0041DA80, 121 bytes.
int XEnemy_UFO::IsBlocked_UFO(int dir) const
{
    int x = mapPosX;
    int z = mapPosZ;
    if (dir == 0) {
        z = z - 1;
    } else if (dir == 1) {
        x = x + 1;
    } else if (dir == 2) {
        z = z + 1;
    } else if (dir == 3) {
        x = x - 1;
    }
    return game->map->IsBlocked_UFO(x, z);
}

// 0x0041DF30, 275 bytes.
XNeedleBall::XNeedleBall(XGameObject* _game, int initDir)
    : XEnemyBase(_game, 0xA5)
{
    shadow->ScaleBy(40.0f);
    RotateBy(JAXIS_X, -1.5707964f);
    ScaleBy(30.0f);
    ShiftWorldPos_OnGround();
    ShiftWorldPos(JAXIS_Y, 3.0f);
    SetCollisionRadius(15.0f);
    faceToDir = 0;
    SetDirection(initDir);
    breakable = 0;
}

// 0x0041E050, 28 bytes.
XNeedleBall::~XNeedleBall()
{
}

// 0x0041E070, 237 bytes.
void XNeedleBall::SubProcessMoveRotate(unsigned long frameCount)
{
    const float rot = 0.03141593f;
    float angle = (float)frameCount * rot;
    if (direction == 0) {
        RotateBy(JAXIS_X, angle);
    } else if (direction == 2) {
        RotateBy(JAXIS_X, -angle);
    } else if (direction == 1) {
        RotateBy(JAXIS_Z, -angle);
    } else if (direction == 3) {
        RotateBy(JAXIS_Z, angle);
    }
}

// 0x0041E160, 239 bytes.
void XNeedleBall::SubProcessMove()
{
    float distance = 0.0f;
    if (localTime < 50) {
        distance = (float)localTime / 50.0f * game->map->GetChipSize();
    } else {
        distance = game->map->GetChipSize();
    }
    D3DVECTOR moveVec = D3DVectorMultiply(GetVectorFromDir(direction), distance);
    SetWorldPos(refrencePos + moveVec);
    if (localTime >= 50) {
        state = WAIT;
        localTime = 0;
    }
}

// 0x0041E250, 286 bytes.
void XNeedleBall::SubProcessWait()
{
    JBaseList possibleDirs;
    int dir;
    int dirRight;
    int dirLeft;
    int dirBack;

    dir = direction;
    dirRight = (direction + 1) % 4;
    dirBack = (direction + 2) % 4;
    dirLeft = (direction + 3) % 4;

    if (!IsBlocked(dir)) {
        StartMoving(dir);
    } else if (!IsBlocked(dirRight)) {
        StartMoving(dirRight);
    } else if (!IsBlocked(dirLeft)) {
        StartMoving(dirLeft);
    } else if (!IsBlocked(dirBack)) {
        StartMoving(dirBack);
    }
}

// 0x0041E370, 275 bytes.
XNeedleBallFast::XNeedleBallFast(XGameObject* _game, int initDir)
    : XEnemyBase(_game, 0xB3)
{
    shadow->ScaleBy(40.0f);
    RotateBy(JAXIS_X, -1.5707964f);
    ScaleBy(30.0f);
    ShiftWorldPos_OnGround();
    ShiftWorldPos(JAXIS_Y, 3.0f);
    SetCollisionRadius(15.0f);
    faceToDir = 0;
    SetDirection(initDir);
    breakable = 0;
}

// 0x0041E490, 28 bytes.
XNeedleBallFast::~XNeedleBallFast()
{
}

// 0x0041E4B0, 237 bytes.
void XNeedleBallFast::SubProcessMoveRotate(unsigned long frameCount)
{
    const float rot = 0.044879895f;
    float angle = (float)frameCount * rot;
    if (direction == 0) {
        RotateBy(JAXIS_X, angle);
    } else if (direction == 2) {
        RotateBy(JAXIS_X, -angle);
    } else if (direction == 1) {
        RotateBy(JAXIS_Z, -angle);
    } else if (direction == 3) {
        RotateBy(JAXIS_Z, angle);
    }
}

// 0x0041E5A0, 239 bytes.
void XNeedleBallFast::SubProcessMove()
{
    float distance = 0.0f;
    if (localTime < 35) {
        distance = (float)localTime / 35.0f * game->map->GetChipSize();
    } else {
        distance = game->map->GetChipSize();
    }
    D3DVECTOR moveVec = D3DVectorMultiply(GetVectorFromDir(direction), distance);
    SetWorldPos(refrencePos + moveVec);
    if (localTime >= 35) {
        state = WAIT;
        localTime = 0;
    }
}

// 0x0041E690, 286 bytes.
void XNeedleBallFast::SubProcessWait()
{
    JBaseList possibleDirs;
    int dir;
    int dirRight;
    int dirLeft;
    int dirBack;

    dir = direction;
    dirRight = (direction + 1) % 4;
    dirBack = (direction + 2) % 4;
    dirLeft = (direction + 3) % 4;

    if (!IsBlocked(dir)) {
        StartMoving(dir);
    } else if (!IsBlocked(dirRight)) {
        StartMoving(dirRight);
    } else if (!IsBlocked(dirLeft)) {
        StartMoving(dirLeft);
    } else if (!IsBlocked(dirBack)) {
        StartMoving(dirBack);
    }
}

// 0x0041E7B0, 250 bytes.
XNeedleBox::XNeedleBox(XGameObject* _game)
    : XEnemyBase(_game, 0xB2)
{
    shadow->ScaleBy(40.0f);
    RotateBy(JAXIS_X, -1.5707964f);
    ScaleBy(35.0f);
    ShiftWorldPos_OnGround();
    ShiftWorldPos(JAXIS_Y, 60.0f);
    SetCollisionRadius(15.0f);
    breakable = 0;
}

// 0x0041E8B0, 28 bytes.
XNeedleBox::~XNeedleBox()
{
}

// 0x0041E8D0, 457 bytes.
void XNeedleBox::SubProcessMove()
{
    int frameWait = 200;
    int frameDrop = 240;
    int frameStay = 390;
    int frameRise = 540;
    float shiftY = 0.0f;

    if (localTime < 200) {
    } else if (localTime < 240) {
        int dropTime = localTime - 200;
        int totalDropTime = 40;
        shiftY = 60.0f - cosf((float)dropTime / 40.0f * 1.5707964f) * 60.0f;
    } else if (localTime < 390) {
        if (localTime == 240) {
            unsigned char volume = game->GetVolumeOfPosition(worldPos);
            game->app->GetSoundList()->Play(0xF5, volume, 0);
        }
        shiftY = 60.0f;
    } else if (localTime < 540) {
        int riseTime = localTime - 390;
        int totalRiseTime = 150;
        shiftY = 60.0f - (float)riseTime * 0.4f;
    }
    D3DVECTOR offset(0.0f, -shiftY, 0.0f);
    SetWorldPos(refrencePos + offset);
    if (localTime >= 540) {
        state = WAIT;
        localTime = 0;
    }
}

// 0x0041EAA0, 59 bytes.
void XNeedleBox::SubProcessWait()
{
    refrencePos = worldPos;
    state = MOVE;
    localTime = 0;
}

// 0x0041EAE0, 250 bytes.
XNeedleBoxDrop::XNeedleBoxDrop(XGameObject* _game)
    : XEnemyBase(_game, 0xB1)
{
    shadow->ScaleBy(40.0f);
    RotateBy(JAXIS_X, -1.5707964f);
    ScaleBy(35.0f);
    ShiftWorldPos_OnGround();
    ShiftWorldPos(JAXIS_Y, 60.0f);
    SetCollisionRadius(15.0f);
    breakable = 0;
}

// 0x0041EBE0, 28 bytes.
XNeedleBoxDrop::~XNeedleBoxDrop()
{
}

// 0x0041EC00, 319 bytes.
void XNeedleBoxDrop::SubProcessMove()
{
    int frameWait = 80;
    int frameDrop = 120;
    float shiftY = 0.0f;

    if (localTime < 80) {
    } else if (localTime < 120) {
        int dropTime = localTime - 80;
        int totalDropTime = 40;
        shiftY = 60.0f - cosf((float)dropTime / 40.0f * 1.5707964f) * 60.0f;
    } else {
        if (localTime == 120) {
            unsigned char volume = game->GetVolumeOfPosition(worldPos);
            game->app->GetSoundList()->Play(0xF5, volume, 0);
        }
        shiftY = 60.0f;
    }
    D3DVECTOR offset(0.0f, -shiftY, 0.0f);
    SetWorldPos(refrencePos + offset);
}

// 0x0041ED40, 117 bytes.
void XNeedleBoxDrop::SubProcessWait()
{
    int pacX = game->pac->GetMapPosX();
    int pacZ = game->pac->GetMapPosZ();
    if (pacX == mapPosX && pacZ == mapPosZ) {
        refrencePos = worldPos;
        state = MOVE;
        localTime = 0;
    }
}

// 0x0041EDC0, 246 bytes.
XGate::XGate(XGameObject* _game, int initDir)
    : XEnemyBase(_game, 0xB4)
{
    shadow->ScaleBy(0.0f);
    RotateBy(JAXIS_X, -1.5707964f);
    ScaleBy(35.0f);
    ShiftWorldPos_OnGround();
    SetCollisionRadius(10.0f);
    faceToDir = 1;
    SetDirection(initDir);
    shield = 1;
}

// 0x0041EEC0, 28 bytes.
XGate::~XGate()
{
}

