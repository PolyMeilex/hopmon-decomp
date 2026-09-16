#include "XMapObject.h"
#include "JSound.h"
#include "XAppMain.h"
#include "XChara.h"
#include "XGame.h"

// 0x00425620, 76 bytes.
XMapObjectList::XMapObjectList(XGameObject* _game)
    : game(_game)
{
    for (int i = 0; i < 25; i++) {
        objArray[i] = NULL;
    }
}

// 0x00425670, 75 bytes.
XMapObjectList::~XMapObjectList()
{
    Empty();
}

// 0x004256C0, 1082 bytes.
XMapObjectBase* XMapObjectList::CreateMapObject(unsigned char type)
{
    if (type == 'Z') {
        return NULL;
    }
    int index = GetNullIndex();
    if (index >= 0) {
        if (type == 'A') {
            objArray[index] = new XCannonUp(game);
        } else if (type == 'N') {
            objArray[index] = new XCannon(game, 0);
        } else if (type == 'S') {
            objArray[index] = new XCannon(game, 2);
        } else if (type == 'E') {
            objArray[index] = new XCannon(game, 1);
        } else if (type == 'W') {
            objArray[index] = new XCannon(game, 3);
        } else if (type == 'V') {
            objArray[index] = new XVolcano(game);
        } else if (type == '!') {
            objArray[index] = new XSteam(game);
        } else if (type == 'P') {
            if (game->app->GetCurrentWorld() == 0) {
                objArray[index] = new XPole(game);
            } else if (game->app->GetCurrentWorld() == 1) {
                objArray[index] = new XTree(game);
            } else {
                objArray[index] = new XShortPole(game);
            }
        }
        return objArray[index];
    }
    return NULL;
}

// 0x00425B00, 111 bytes.
void XMapObjectList::Empty()
{
    for (int i = 0; i < 25; i++) {
        if (objArray[i] != NULL) {
            delete objArray[i];
            objArray[i] = NULL;
        }
    }
}

// 0x00425B70, 85 bytes.
void XMapObjectList::ProcessRotate(unsigned long frameCount)
{
    for (int i = 0; i < 25; i++) {
        if (objArray[i] != NULL) {
            objArray[i]->ProcessRotate(frameCount);
        }
    }
}

// 0x00425BD0, 160 bytes.
void XMapObjectList::ProcessFrame()
{
    for (int i = 0; i < 25; i++) {
        if (objArray[i] != NULL) {
            objArray[i]->ProcessFrame();
            if (objArray[i]->GetState() == XMapObjectBase::NON) {
                delete objArray[i];
                objArray[i] = NULL;
            }
        }
    }
}

// 0x00425C70, 79 bytes.
void XMapObjectList::Render()
{
    for (int i = 0; i < 25; i++) {
        if (objArray[i] != NULL) {
            objArray[i]->Render();
        }
    }
}

// 0x00425CC0, 60 bytes.
int XMapObjectList::GetNullIndex() const
{
    for (int i = 0; i < 25; i++) {
        if (objArray[i] == NULL) {
            return i;
        }
    }
    return -1;
}

// 0x00425D00, 346 bytes.
XMapObjectBase::XMapObjectBase(XGameObject* _game, unsigned short resId)
    : JObject3DX(_game->app, resId), game(_game), state(NON), localTime(0),
      direction(-1), shadow(new JObject3D_Shadow(_game->app, 1.0f, 1.0f))
{
    shadow->SetTexture(0xA3, 0.0f, 0.0f, 1.0f, 1.0f);
    RotateBy(JAXIS_X, -1.5707964f);
    state = PROCESS;
}

// 0x00425E60, 90 bytes.
XMapObjectBase::~XMapObjectBase()
{
    if (shadow != NULL) {
        delete shadow;
        shadow = NULL;
    }
}

// 0x00425EC0, 111 bytes.
void XMapObjectBase::Render()
{
    if (state != NON) {
        JObject3DX::Render();
        shadow->SetWorldPos(D3DVECTOR(worldPos.x, 1.0f, worldPos.z));
        shadow->Render();
    }
}

// 0x00425F30, 17 bytes.
XMapObjectBase::STATE XMapObjectBase::GetState() const
{
    return state;
}

// 0x00425F50, 149 bytes.
void XMapObjectBase::SetDirection(int dir, int faceToDir)
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

// 0x00425FF0, 13 bytes.
void XMapObjectBase::ProcessRotate(unsigned long frameCount)
{
}

// 0x00426000, 11 bytes.
void XMapObjectBase::ProcessFrame()
{
}

// 0x00426010, 180 bytes.
XVolcano::XVolcano(XGameObject* _game)
    : XMapObjectBase(_game, 0xB0)
{
    shadow->ScaleBy(0.0f);
    ScaleBy(135.0f);
    ShiftWorldPos_OnGround();
    ShiftWorldPos(JAXIS_Y, 2.0f);
}

// 0x004260D0, 28 bytes.
XVolcano::~XVolcano()
{
}

// 0x004260F0, 175 bytes.
void XVolcano::ProcessFrame()
{
    localTime++;
    if (state == PROCESS) {
        if (localTime < 1500) {
            return;
        }
        if (localTime < 2500) {
            if (localTime % 50 == 0) {
                D3DVECTOR pos = worldPos;
                pos.y += 20.0f;
                game->enemyFireList->CreateVolcanoFire(pos);
            }
        } else {
            localTime = 0;
        }
    }
}

// 0x004261A0, 185 bytes.
XCannonUp::XCannonUp(XGameObject* _game)
    : XMapObjectBase(_game, 0xA6)
{
    shadow->ScaleBy(60.0f);
    ScaleBy(80.0f);
    ShiftWorldPos_OnGround();
    ShiftWorldPos(JAXIS_Y, 3.0f);
}

// 0x00426260, 28 bytes.
XCannonUp::~XCannonUp()
{
}

// 0x00426280, 133 bytes.
void XCannonUp::ProcessFrame()
{
    localTime++;
    if (state == PROCESS && localTime % 200 == 0) {
        D3DVECTOR pos = worldPos;
        pos.y += 10.0f;
        game->enemyFireList->CreateCannonUpFire(pos);
    }
}

// 0x00426310, 199 bytes.
XCannon::XCannon(XGameObject* _game, int initDir)
    : XMapObjectBase(_game, 0xAC)
{
    shadow->ScaleBy(55.0f);
    ScaleBy(80.0f);
    ShiftWorldPos_OnGround();
    ShiftWorldPos(JAXIS_Y, 3.0f);
    SetDirection(initDir, 1);
}

// 0x004263E0, 28 bytes.
XCannon::~XCannon()
{
}

// 0x00426400, 122 bytes.
void XCannon::ProcessFrame()
{
    localTime++;
    if (state == PROCESS && localTime % 450 == 0) {
        D3DVECTOR pos = worldPos;
        game->enemyFireList->CreateCannonFire(pos, direction);
    }
}

// 0x00426480, 185 bytes.
XPole::XPole(XGameObject* _game)
    : XMapObjectBase(_game, 0xA7)
{
    shadow->ScaleBy(60.0f);
    ScaleBy(120.0f);
    ShiftWorldPos_OnGround();
    ShiftWorldPos(JAXIS_Y, 3.0f);
}

// 0x00426540, 28 bytes.
XPole::~XPole()
{
}

// 0x00426560, 185 bytes.
XShortPole::XShortPole(XGameObject* _game)
    : XMapObjectBase(_game, 0x13B)
{
    shadow->ScaleBy(55.0f);
    ScaleBy(145.0f);
    ShiftWorldPos_OnGround();
    ShiftWorldPos(JAXIS_Y, 3.0f);
}

// 0x00426620, 28 bytes.
XShortPole::~XShortPole()
{
}

// 0x00426640, 185 bytes.
XTree::XTree(XGameObject* _game)
    : XMapObjectBase(_game, 0xA8)
{
    shadow->ScaleBy(90.0f);
    ScaleBy(130.0f);
    ShiftWorldPos_OnGround();
    ShiftWorldPos(JAXIS_Y, 3.0f);
}

// 0x00426700, 28 bytes.
XTree::~XTree()
{
}

// 0x00426720, 145 bytes.
XSteam::XSteam(XGameObject* _game)
    : XMapObjectBase(_game, 0xFFFF)
{
    shadow->ScaleBy(0.0f);
    SetWorldPos(JAXIS_Y, 0.0f);
}

// 0x004267C0, 28 bytes.
XSteam::~XSteam()
{
}

// 0x004267E0, 221 bytes.
void XSteam::ProcessFrame()
{
    localTime++;
    if (state == PROCESS) {
        if (localTime == 200) {
            unsigned char vol = game->GetVolumeOfPosition(worldPos);
            game->app->GetSoundList()->Play(0x13E, vol, 0);
        }
        if (localTime > 200 && localTime % 10 == 0) {
            game->enemyFireList->CreateSteamFire(worldPos);
        }
        if (localTime > 400) {
            localTime = 0;
        }
    }
}

