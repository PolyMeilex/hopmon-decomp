#include "common.h"
#undef D3D_OVERLOADS
#include "XGame.h"
#include "XAppMain.h"
#include "XMap.h"
#include "XPac.h"
#include "XEnemy.h"
#include "XMapObject.h"
#include "XChara.h"
#include "JMidi.h"
#include "JDInput.h"
#include "JStandard.h"
#include "JSurface.h"
#include "JSprite.h"
#include "JAscii.h"
#include "XRegister.h"
#include "D3DUtil.h"

// 0x0041F1E0, 829 bytes.
XStatusDisplay::XStatusDisplay(XGameObject* _game)
{
    game = _game;
    textLevel = new JSprite(game->app->GetSurfaceList(), 0xB9, 1);
    textCrystals = new JSprite(game->app->GetSurfaceList(), 0xBC, 1);
    textFireReady = new JSprite(game->app->GetSurfaceList(), 0xBD, 2);
    fireLED = new JSprite(game->app->GetSurfaceList(), 0xC0, 2);
    numLevel = new JNumber(game->app->GetSurfaceList(), 0xBA, 10);
    numCrystals = new JNumber(game->app->GetSurfaceList(), 0xBA, 10);

    int posX;
    int posY = 4;
    posX = 12;

    textLevel->SetPosition(posX, posY);
    posX += textLevel->GetWidth() + 8;
    numLevel->SetPosition(posX, posY + 1);
    numLevel->SetDisplayZero(1);
    numLevel->SetNumDigits(2);
    posX += numLevel->GetDigitWidth() * 2 + 24;

    textCrystals->SetPosition(posX, posY);
    posX += textCrystals->GetWidth() + 8;
    numCrystals->SetPosition(posX, posY + 1);
    numCrystals->SetDisplayZero(1);
    numCrystals->SetNumDigits(2);
}

// 0x0041F520, 382 bytes.
XStatusDisplay::~XStatusDisplay()
{
    SAFE_DELETE(textCrystals);
    SAFE_DELETE(textLevel);
    SAFE_DELETE(textFireReady);
    SAFE_DELETE(numCrystals);
    SAFE_DELETE(numLevel);
    SAFE_DELETE(fireLED);
}

// 0x0041F6A0, 427 bytes.
void XStatusDisplay::Draw(IDirectDrawSurface7* lpSurface)
{
    textLevel->Draw(lpSurface);
    textCrystals->Draw(lpSurface);

    numLevel->SetNumber(game->app->currentLevel + 1);
    numLevel->Draw(lpSurface);

    numCrystals->SetNumber(game->crystalList->GetCrystalLeft());
    numCrystals->Draw(lpSurface);

    int ledX = game->app->GetScreenWidth() - 286;
    int ledY = game->app->GetScreenHeight() - 31;
    int fireEnergy = game->pac->GetFireEnergy() / 100;

    textFireReady->SetCurrentFrame(fireEnergy < 10 ? 0 : 1);
    textFireReady->SetPosition(ledX, ledY);
    textFireReady->Draw(lpSurface);

    ledX += textFireReady->GetWidth() + 7;
    ledY += 3;

    for (int i = 0; i < 10; i++) {
        fireLED->SetCurrentFrame(i < fireEnergy);
        fireLED->SetPosition(ledX, ledY);
        fireLED->Draw(lpSurface);
        ledX += fireLED->GetWidth() / 3 + 1;
    }
}

// 0x0041F850, 282 bytes.
XProcessObject::XProcessObject(XApplication* _app)
    : app(_app),
      state(NON),
      tick(0),
      fadeLeft(new JSprite(app->GetSurfaceList(), 0xC3, 1)),
      fadeRight(new JSprite(app->GetSurfaceList(), 0xC2, 1)),
      processType(0)
{
}

// 0x0041F970, 142 bytes.
XProcessObject::~XProcessObject()
{
    SAFE_DELETE(fadeLeft);
    SAFE_DELETE(fadeRight);
}

// 0x0041FA00, 13 bytes.
void XProcessObject::ProcessFrame(unsigned long frameCount)
{
}

// 0x0041FA10, 11 bytes.
void XProcessObject::Render()
{
}

// 0x0041FA20, 50 bytes.
void XProcessObject::StartFadeIn(int _processType)
{
    _ResetFadeIn();
    processType = _processType;
    state = FADEIN;
    tick = 0;
}

// 0x0041FA60, 50 bytes.
void XProcessObject::StartFadeOut(int _processType)
{
    _ResetFadeOut();
    processType = _processType;
    state = FADEOUT;
    tick = 0;
}

// 0x0041FAA0, 42 bytes.
void XProcessObject::StartProcess(int _processType)
{
    processType = _processType;
    state = PROCESS;
    tick = 0;
}

// 0x0041FAD0, 167 bytes.
void XProcessObject::ProcessFadeIn()
{
    _ResetFadeIn();

    const int fadeProcessFrame = 100;
    int _tick = tick - 20;
    if (_tick < 0) {
        _tick = 0;
    } else if (_tick > fadeProcessFrame) {
        _tick = fadeProcessFrame;
    }

    float shiftX = ((float)_tick / 100.0f) * (float)(app->GetScreenWidth() / 2);
    fadeLeft->ShiftPosX(-shiftX);
    fadeRight->ShiftPosX(shiftX);
}

// 0x0041FB80, 153 bytes.
void XProcessObject::ProcessFadeOut()
{
    _ResetFadeOut();

    const int fadeProcessFrame = 100;
    int _tick = tick;
    if (_tick > fadeProcessFrame) {
        _tick = fadeProcessFrame;
    }

    float shiftX = ((float)_tick / 100.0f) * (float)(app->GetScreenWidth() / 2);
    fadeLeft->ShiftPosX(shiftX);
    fadeRight->ShiftPosX(-shiftX);
}

// 0x0041FC20, 118 bytes.
void XProcessObject::_ResetFadeIn()
{
    int fadeWidth = app->GetScreenWidth() / 2;
    fadeLeft->SetDrawSize(fadeWidth, app->GetScreenHeight());
    fadeLeft->SetPosition(0, 0);
    fadeRight->SetDrawSize(fadeWidth, app->GetScreenHeight());
    fadeRight->SetPosition(fadeWidth, 0);
}

// 0x0041FCA0, 130 bytes.
void XProcessObject::_ResetFadeOut()
{
    int fadeWidth = app->GetScreenWidth() / 2;
    fadeLeft->SetDrawSize(fadeWidth, app->GetScreenHeight());
    fadeLeft->SetPosition(-fadeWidth, 0);
    fadeRight->SetDrawSize(fadeWidth, app->GetScreenHeight());
    fadeRight->SetPosition(app->GetScreenWidth(), 0);
}

// 0x0041FD30, 937 bytes.
XGameObject::XGameObject(XApplication* _app)
    : XProcessObject(_app),
      camera(0),
      map(0),
      pac(0),
      enemyList(0),
      mapObjList(0),
      pacFireList(0),
      enemyFireList(0),
      particleList(0),
      crystalList(0),
      statusDisplay(0)
{
    app->GetSurfaceList()->CreateSurface(0xbc, 1);
    app->GetSurfaceList()->CreateSurface(0xb9, 1);
    app->GetSurfaceList()->CreateSurface(0xbd, 1);
    app->GetSurfaceList()->CreateSurface(0xba, 1);
    app->GetSurfaceList()->CreateSurface(0xc0, 1);

    camera = new XCamera(_app);
    pac = new XPac(this);
    enemyList = new XEnemyList(this);
    mapObjList = new XMapObjectList(this);
    pacFireList = new XPacFireList(this);
    enemyFireList = new XEnemyFireList(this);
    particleList = new XParticleList(this);
    crystalList = new XCrystalList(this);
    statusDisplay = new XStatusDisplay(this);

    InitLevel();
}

// 0x004200E0, 792 bytes.
XGameObject::~XGameObject()
{
    SAFE_DELETE(map);
    SAFE_DELETE(camera);
    SAFE_DELETE(pac);
    SAFE_DELETE(enemyList);
    SAFE_DELETE(mapObjList);
    SAFE_DELETE(pacFireList);
    SAFE_DELETE(enemyFireList);
    SAFE_DELETE(particleList);
    SAFE_DELETE(crystalList);
    SAFE_DELETE(statusDisplay);

    app->GetSurfaceList()->DestroySurface(0xbc);
    app->GetSurfaceList()->DestroySurface(0xb9);
    app->GetSurfaceList()->DestroySurface(0xbd);
    app->GetSurfaceList()->DestroySurface(0xba);
    app->GetSurfaceList()->DestroySurface(0xc0);
}

// 0x00420400, 1730 bytes.
void XGameObject::ProcessFrame(unsigned long frameCount)
{
    if (state == NON) {
        return;
    }

    tick += frameCount;

    if (state == FADEIN) {
        ProcessFadeIn();
        if (tick >= 120) {
            StartProcess(0);
        }
    } else if (state == FADEOUT) {
        ProcessFadeOut();
        if (tick >= 120) {
            if (processType == 0) {
                app->SetState(XApplication::START_SELLEVEL);
                state = NON;
            } else {
                app->currentLevel++;
                if (app->currentLevel > app->maxLevel) {
                    app->currentLevel = 0;
                    app->SetState(XApplication::START_ENDING);
                    state = NON;
                } else {
                    if (app->currentLevel > app->maxSelLevel) {
                        app->maxSelLevel = app->currentLevel;
                        app->SaveWinRegData();
                    }
                    if (!IsRegistered() && app->currentLevel > app->maxFreeLevel) {
                        app->SetState(XApplication::START_REGINFO);
                        state = NON;
                    } else {
                        InitLevel();
                    }
                }
            }
            return;
        }
    } else if (state == PROCESS) {
        if (processType == 1) {
            if (tick > 300) {
                StartFadeOut(1);
            }
        } else if (processType == 2) {
            if (tick > 300) {
                StartFadeOut(0);
            }
        }
    }

    pac->ProcessRotate(frameCount);
    enemyList->ProcessRotate(frameCount);
    mapObjList->ProcessRotate(frameCount);
    crystalList->ProcessRotate(frameCount);

    for (unsigned long i = 0; i < frameCount; i++) {
        enemyList->CheckCollisionWithPac();
        enemyFireList->CheckCollisionWithPac();
        crystalList->CheckCollisionWithPac();
        pacFireList->CheckCollisionWithEnemy();
        map->ProcessFrame();
        pac->ProcessFrame();
        camera->ProcessFrame(pac);
        crystalList->ProcessFrame();
        pacFireList->ProcessFrame();
        enemyFireList->ProcessFrame();
        particleList->ProcessFrame();
        enemyList->ProcessFrame();
        mapObjList->ProcessFrame();
    }

    app->GetDInput()->UpdateInputState();

    int up = app->GetDInput()->GetKeyboardState(200) || app->GetDInput()->GetJoyDirState(0, 0);
    int right = app->GetDInput()->GetKeyboardState(205) || app->GetDInput()->GetJoyDirState(0, 1);
    int down = app->GetDInput()->GetKeyboardState(208) || app->GetDInput()->GetJoyDirState(0, 2);
    int left = app->GetDInput()->GetKeyboardState(203) || app->GetDInput()->GetJoyDirState(0, 3);

    int fire = app->GetDInput()->GetKeyboardPressed(42) ||
               app->GetDInput()->GetKeyboardPressed(54) ||
               app->GetDInput()->GetKeyboardPressed(44) ||
               app->GetDInput()->GetJoyButtonPressed(0, 0);

    int rotate = app->GetDInput()->GetKeyboardState(29) ||
                 app->GetDInput()->GetKeyboardState(157) ||
                 app->GetDInput()->GetKeyboardState(45) ||
                 app->GetDInput()->GetJoyButtonState(0, 1);

    if (pac->GetState() == XPac::WAIT || pac->GetState() == XPac::MOVE || pac->GetState() == XPac::ON_ARROW) {
        if (app->GetDInput()->GetKeyboardPressed(1)) {
            pac->StartDying();
        }
    }

    if (pac->GetState() == XPac::WAIT || pac->GetState() == XPac::MOVE || pac->GetState() == XPac::ON_ARROW) {
        if (fire && pac->GetFireEnergy() >= 1000) {
            pacFireList->CreateFire();
            pac->ResetFireEnergy();
        }

        if (rotate) {
            if (up) {
                camera->IncrVerticalAngle(frameCount);
            } else if (down) {
                camera->DecrVerticalAngle(frameCount);
            }
        }

        if (camera->GetState() == XCamera::WAIT) {
            if (rotate) {
                if (right) {
                    camera->StartRotatingLeft();
                } else if (left) {
                    camera->StartRotatingRight();
                }
            } else {
                int dir = -1;
                if (up) {
                    dir = 0;
                } else if (right) {
                    dir = 1;
                } else if (down) {
                    dir = 2;
                } else if (left) {
                    dir = 3;
                }

                if (dir >= 0) {
                    dir = (camera->GetViewDirection() + dir) % 4;
                    if (pac->GetState() == XPac::WAIT) {
                        pac->StartMoving(dir);
                    } else if (pac->GetState() == XPac::ON_ARROW) {
                        pac->SetFaceDirection(dir);
                    }
                }
            }
        }
    }
}

// 0x00420AD0, 255 bytes.
void XGameObject::Render()
{
    if (state == NON) {
        return;
    }

    camera->DoSetTransform();
    map->Render();
    mapObjList->Render();
    crystalList->Render();
    enemyList->Render();
    pac->Render();
    particleList->Render(camera->GetViewMatrix());
    enemyFireList->Render(camera->GetViewMatrix());
    pacFireList->Render(camera->GetViewMatrix());
    statusDisplay->Draw(app->GetBackBuffer());
    if (state == FADEIN || state == FADEOUT) {
        fadeLeft->Draw(app->GetBackBuffer());
        fadeRight->Draw(app->GetBackBuffer());
    }
}

// 0x00420BD0, 448 bytes.
void XGameObject::InitLevel()
{
    if (app->GetCurrentWorld() == 0) {
        app->GetMidiList()->Play(1);
    } else if (app->GetCurrentWorld() == 1) {
        app->GetMidiList()->Play(2);
    } else {
        app->GetMidiList()->Play(3);
    }

    pac->Reset();
    enemyList->Empty();
    mapObjList->Empty();
    crystalList->Empty();
    pacFireList->Empty();
    enemyFireList->Empty();
    particleList->Empty();
    crystalList->Empty();

    SAFE_DELETE(map);

    map = new XMap(app, GetMapResId(app), pac, enemyList, mapObjList, crystalList);
    camera->ResetForGame(pac);
    StartFadeIn(0);
}

// 0x00420D90, 188 bytes.
unsigned char XGameObject::GetVolumeOfPosition(D3DVECTOR pos)
{
    const float normalZone = 130.0f;
    const float deadZone = 550.0f;

    if (pac->GetState() == XPac::NON) {
        return 0;
    }

    float distance = D3DFindDistance(pos, pac->GetWorldPos());
    if (distance < 130.0f) {
        return 100;
    } else {
        if (distance > 550.0f) {
            return 0;
        }
    }
    return (unsigned char)((550.0f - distance) / 420.0f * 30.0f) + 70;
}
