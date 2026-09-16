#include "XTitle.h"
#include "D3DApp.h"
#include "JAscii.h"
#include "JDInput.h"
#include "JMidi.h"
#include "JObject3D.h"
#include "JSound.h"
#include "JSprite.h"
#include "JSurface.h"
#include "XAppMain.h"
#include "XMap.h"
#include "XRegister.h"

// 0x004289C0, 442 bytes.
XTitleBackground::XTitleBackground(XApplication* _app)
{
    app = _app;
    backUpper = new JObject3D_Rect(app, 720.0f, 400.0f);
    backLower = new JObject3D_Rect(app, 720.0f, 400.0f);
    textureOffset = 0.0f;

    backUpper->RotateBy(JAXIS_X, 3.1415927f);
    backUpper->ShiftWorldPos(JAXIS_Y, 120.0f);
    backUpper->UsePerspective(1);
    backUpper->SetFilterState(JObject3D::FILTER_POINT);

    backLower->ShiftWorldPos(JAXIS_Y, -120.0f);
    backLower->UsePerspective(1);
    backLower->SetFilterState(JObject3D::FILTER_LINEAR);
}

// 0x00428B80, 142 bytes.
XTitleBackground::~XTitleBackground()
{
    SAFE_DELETE(backUpper);
    SAFE_DELETE(backLower);
}

// 0x00428C10, 75 bytes.
void XTitleBackground::ProcessFrame(unsigned long frameCount)
{
    textureOffset += frameCount * 0.02f;
}

// 0x00428C60, 222 bytes.
void XTitleBackground::Render()
{
    backUpper->SetTexture(0xC1, 0.0f, textureOffset, 18.0f, textureOffset + 10.0f);
    backUpper->Render();
    backLower->SetTexture(0xC1, 0.0f, -textureOffset, 18.0f, 10.0f - textureOffset);
    backLower->Render();
}

// 0x00428D40, 557 bytes.
XLogoObject::XLogoObject(XApplication* _app)
    : XProcessObject(_app)
{
    camera = NULL;
    logo = NULL;
    backWhite = NULL;

    app->GetMidiList()->Stop();
    app->GetSurfaceList()->CreateSurface(0x13C, 0);

    camera = new XCamera(app);
    camera->ResetForTitle();

    logo = new JSprite(app->GetSurfaceList(), 0x13C, 1);

    backWhite = new JObject3D_Rect(app, 600.0f, 600.0f);
    backWhite->RotateBy(JAXIS_X, -1.5707964f);
    backWhite->UsePerspective(0);
    backWhite->SetTexture(0x14C, 0.0f, 0.0f, 1.0f, 1.0f);

    StartProcess(0);
}

// 0x00428F70, 280 bytes.
XLogoObject::~XLogoObject()
{
    SAFE_DELETE(camera);
    SAFE_DELETE(logo);
    SAFE_DELETE(backWhite);
    app->GetSurfaceList()->DestroySurface(0x13C);
}

// 0x00429090, 142 bytes.
void XLogoObject::ProcessFrame(unsigned long frameCount)
{
    if (state == NON) {
        return;
    }

    for (unsigned long f = 0; f < frameCount; f++) {
        tick++;
        if (tick == 50) {
            app->GetSoundList()->Play(0x13D, 100, 0);
        }
        if (tick >= 950) {
            app->SetState(XApplication::START_TITLE);
            state = NON;
            return;
        }
    }
}

// 0x00429120, 148 bytes.
void XLogoObject::Render()
{
    if (state == NON) {
        return;
    }

    camera->DoSetTransform();
    backWhite->Render();
    int centerX = app->GetScreenWidth() / 2;
    int centerY = app->GetScreenHeight() / 2;
    logo->SetCenterPos(centerX, centerY);
    logo->DrawScale(app->GetBackBuffer(), 1.0f);
}

// 0x004291C0, 990 bytes.
XRegisterInfoObject::XRegisterInfoObject(XApplication* _app)
    : XProcessObject(_app)
{
    camera = NULL;
    background = NULL;
    cursorLeft = NULL;
    cursorRight = NULL;
    menuBack = NULL;
    screenShot1 = NULL;
    screenShot2 = NULL;
    regInfo = NULL;

    app->GetMidiList()->Play(3);
    app->GetSurfaceList()->CreateSurface(0xD0, 1);
    app->GetSurfaceList()->CreateSurface(0xD1, 1);
    app->GetSurfaceList()->CreateSurface(200, 1);
    app->GetSurfaceList()->CreateSurface(0x142, 0);
    app->GetSurfaceList()->CreateSurface(0x143, 0);
    app->GetSurfaceList()->CreateSurface(0x141, 0);

    camera = new XCamera(app);
    camera->ResetForTitle();

    background = new XTitleBackground(app);

    cursorLeft = new JSprite(app->GetSurfaceList(), 0xD0, 2);
    cursorRight = new JSprite(app->GetSurfaceList(), 0xD1, 2);
    menuBack = new JSprite(app->GetSurfaceList(), 200, 2);
    screenShot1 = new JSprite(app->GetSurfaceList(), 0x142, 1);
    screenShot2 = new JSprite(app->GetSurfaceList(), 0x143, 1);
    regInfo = new JSprite(app->GetSurfaceList(), 0x141, 1);

    StartFadeIn(0);
}

// 0x004295A0, 695 bytes.
XRegisterInfoObject::~XRegisterInfoObject()
{
    SAFE_DELETE(camera);
    SAFE_DELETE(background);
    SAFE_DELETE(cursorLeft);
    SAFE_DELETE(cursorRight);
    SAFE_DELETE(menuBack);
    SAFE_DELETE(screenShot1);
    SAFE_DELETE(screenShot2);
    SAFE_DELETE(regInfo);

    app->GetSurfaceList()->DestroySurface(0xD0);
    app->GetSurfaceList()->DestroySurface(0xD1);
    app->GetSurfaceList()->DestroySurface(200);
    app->GetSurfaceList()->DestroySurface(0x142);
    app->GetSurfaceList()->DestroySurface(0x143);
    app->GetSurfaceList()->DestroySurface(0x141);
}

// 0x00429860, 355 bytes.
void XRegisterInfoObject::ProcessFrame(unsigned long frameCount)
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
            app->SetState(XApplication::START_SELLEVEL);
            state = NON;
            return;
        }
    }

    background->ProcessFrame(frameCount);

    if (state == PROCESS) {
        int _tick = tick % 50;
        int cursorFrame = (_tick < 25) ? 0 : 1;
        cursorLeft->SetCurrentFrame(cursorFrame);
        cursorRight->SetCurrentFrame(cursorFrame);

        app->GetDInput()->UpdateInputState();

        int keyButton = CheckButtonPressed(app->GetDInput());
        if (app->GetDInput()->GetKeyboardPressed(1)) {
            keyButton = 1;
        }

        if (keyButton) {
            app->GetSoundList()->Play(0xED, 100, 0);
            StartFadeOut(0);
        }
    }
}

// 0x004299D0, 561 bytes.
void XRegisterInfoObject::Render()
{
    if (state == NON) {
        return;
    }

    camera->DoSetTransform();
    background->Render();

    int centerX = app->GetScreenWidth() / 2;
    int topY = app->GetScreenHeight() / 2 - 240;

    screenShot1->SetCenterPos(centerX - 0x9F, topY + 0x94);
    screenShot1->Draw(app->GetBackBuffer());

    screenShot2->SetCenterPos(centerX + 0x9F, topY + 0x94);
    screenShot2->Draw(app->GetBackBuffer());

    regInfo->SetCenterPos(centerX, topY + 0x160);
    regInfo->Draw(app->GetBackBuffer());

    int menuY = topY + 0x1C0;
    menuBack->SetCenterPos(centerX, menuY);
    menuBack->SetCurrentFrame(1);
    menuBack->Draw(app->GetBackBuffer());

    if (state == PROCESS) {
        int posX1 = menuBack->GetPosIntX() - cursorLeft->GetWidth() / 2 - 4;
        int posX2 = menuBack->GetPosIntX() + menuBack->GetWidth() + 4;
        int posY = menuBack->GetPosIntY() + 2;

        cursorLeft->SetPosition(posX1, posY);
        cursorLeft->Draw(app->GetBackBuffer());
        cursorRight->SetPosition(posX2, posY);
        cursorRight->Draw(app->GetBackBuffer());
    }

    if (state == FADEIN || state == FADEOUT) {
        fadeLeft->Draw(app->GetBackBuffer());
        fadeRight->Draw(app->GetBackBuffer());
    }
}

// 0x00429C10, 1699 bytes.
XSelectLevelObject::XSelectLevelObject(XApplication* _app)
    : XProcessObject(_app)
{
    camera = NULL;
    background = NULL;
    map = NULL;
    cursorLeft = NULL;
    cursorRight = NULL;
    cursor2Left = NULL;
    cursor2Right = NULL;
    textUnreg = NULL;
    textLevel = NULL;
    levelLED = NULL;
    numLevel = NULL;
    cursorPos = -1;
    inSelectLevel = 0;

    app->GetMidiList()->Play(0);
    app->GetSurfaceList()->CreateSurface(0xD0, 1);
    app->GetSurfaceList()->CreateSurface(0xD1, 1);
    app->GetSurfaceList()->CreateSurface(0xCE, 1);
    app->GetSurfaceList()->CreateSurface(0xCF, 1);
    app->GetSurfaceList()->CreateSurface(0xF7, 1);
    app->GetSurfaceList()->CreateSurface(0xB9, 1);
    app->GetSurfaceList()->CreateSurface(0x140, 1);
    app->GetSurfaceList()->CreateSurface(0xBA, 1);
    app->GetSurfaceList()->CreateSurface(0xCD, 1);
    app->GetSurfaceList()->CreateSurface(0xCC, 1);
    app->GetSurfaceList()->CreateSurface(200, 1);

    camera = new XCamera(app);
    camera->ResetForTitle();

    numLevel = new JNumber(app->GetSurfaceList(), 0xBA, 10);
    numLevel->SetDisplayZero(1);
    numLevel->SetNumDigits(2);

    background = new XTitleBackground(app);

    cursorLeft = new JSprite(app->GetSurfaceList(), 0xD0, 2);
    cursorRight = new JSprite(app->GetSurfaceList(), 0xD1, 2);
    cursor2Left = new JSprite(app->GetSurfaceList(), 0xCE, 2);
    cursor2Right = new JSprite(app->GetSurfaceList(), 0xCF, 2);
    textUnreg = new JSprite(app->GetSurfaceList(), 0xF7, 1);
    textLevel = new JSprite(app->GetSurfaceList(), 0xB9, 1);
    levelLED = new JSprite(app->GetSurfaceList(), 0x140, 3);
    menuItems[0] = new JSprite(app->GetSurfaceList(), 0xCD, 2);
    menuItems[1] = new JSprite(app->GetSurfaceList(), 0xCC, 2);
    menuItems[2] = new JSprite(app->GetSurfaceList(), 200, 2);

    CreatePreviewMap();
    StartFadeIn(0);
}

// 0x0042A2C0, 1088 bytes.
XSelectLevelObject::~XSelectLevelObject()
{
    SAFE_DELETE(camera);
    SAFE_DELETE(numLevel);
    SAFE_DELETE(background);
    SAFE_DELETE(map);
    SAFE_DELETE(cursorLeft);
    SAFE_DELETE(cursorRight);
    SAFE_DELETE(cursor2Left);
    SAFE_DELETE(cursor2Right);
    SAFE_DELETE(textUnreg);
    SAFE_DELETE(textLevel);
    SAFE_DELETE(levelLED);

    for (int i = 0; i < 3; i++) {
        SAFE_DELETE(menuItems[i]);
    }

    app->GetSurfaceList()->DestroySurface(0xD0);
    app->GetSurfaceList()->DestroySurface(0xD1);
    app->GetSurfaceList()->DestroySurface(0xCE);
    app->GetSurfaceList()->DestroySurface(0xCF);
    app->GetSurfaceList()->DestroySurface(0xF7);
    app->GetSurfaceList()->DestroySurface(0xB9);
    app->GetSurfaceList()->DestroySurface(0x140);
    app->GetSurfaceList()->DestroySurface(0xBA);
    app->GetSurfaceList()->DestroySurface(0xCD);
    app->GetSurfaceList()->DestroySurface(0xCC);
    app->GetSurfaceList()->DestroySurface(200);
}

// 0x0042B110, 209 bytes.
void XSelectLevelObject::CreatePreviewMap()
{
    SAFE_DELETE(map);
    map = new XPreviewMap(app, GetMapResId(app));
}

// 0x0042A700, 1354 bytes.
void XSelectLevelObject::ProcessFrame(unsigned long frameCount)
{
    if (state == NON) {
        return;
    }

    tick += frameCount;

    if (state == FADEIN) {
        ProcessFadeIn();
        if (tick >= 120) {
            cursorPos = 0;
            StartProcess(0);
        }
    } else if (state == FADEOUT) {
        ProcessFadeOut();
        if (tick >= 120) {
            if (processType == 0) {
                if (!IsRegistered() && app->currentLevel > app->maxFreeLevel) {
                    app->SetState(XApplication::START_REGINFO);
                    state = NON;
                    return;
                }
                app->SetState(XApplication::START_GAME);
                state = NON;
                return;
            } else if (processType == 1) {
                app->currentLevel = 0;
                app->SetState(XApplication::START_TITLE);
                state = NON;
                return;
            }
        }
    }

    background->ProcessFrame(frameCount);
    map->ProcessFrame(frameCount);

    if (state == PROCESS) {
        int rem = tick % 50;
        int cursorFrame = (rem < 25) ? 0 : 1;
        cursorLeft->SetCurrentFrame(cursorFrame);
        cursorRight->SetCurrentFrame(cursorFrame);
        cursor2Left->SetCurrentFrame(cursorFrame);
        cursor2Right->SetCurrentFrame(cursorFrame);

        app->GetDInput()->UpdateInputState();

        int up = app->GetDInput()->GetKeyboardPressed(0xC8) || app->GetDInput()->GetJoyDirPressed(0, 0);
        int right = app->GetDInput()->GetKeyboardPressed(0xCD) || app->GetDInput()->GetJoyDirPressed(0, 1);
        int down = app->GetDInput()->GetKeyboardPressed(0xD0) || app->GetDInput()->GetJoyDirPressed(0, 2);
        int left = app->GetDInput()->GetKeyboardPressed(0xCB) || app->GetDInput()->GetJoyDirPressed(0, 3);

        int button = CheckButtonPressed(app->GetDInput());

        if (inSelectLevel != 0) {
            if (app->GetDInput()->GetKeyboardPressed(1)) {
                button = 1;
            }

            if (left) {
                app->GetSoundList()->Play(0xEC, 100, 0);
                app->currentLevel--;
                if (app->currentLevel < 0) {
                    app->currentLevel = app->maxSelLevel;
                }
                CreatePreviewMap();
            } else if (right) {
                app->GetSoundList()->Play(0xEC, 100, 0);
                app->currentLevel++;
                if (app->currentLevel > app->maxSelLevel) {
                    app->currentLevel = 0;
                }
                CreatePreviewMap();
            } else if (button) {
                app->GetSoundList()->Play(0xED, 100, 0);
                inSelectLevel = 0;
            }
        } else {
            if (app->GetDInput()->GetKeyboardPressed(1)) {
                cursorPos = 2;
                button = 1;
            }

            if (up) {
                app->GetSoundList()->Play(0xEC, 100, 0);
                cursorPos--;
                if (cursorPos < 0) {
                    cursorPos = 2;
                }
            } else if (down) {
                app->GetSoundList()->Play(0xEC, 100, 0);
                cursorPos++;
                if (cursorPos >= 3) {
                    cursorPos = 0;
                }
            } else if (button) {
                app->GetSoundList()->Play(0xED, 100, 0);
                if (cursorPos == 0) {
                    StartFadeOut(0);
                } else if (cursorPos == 1) {
                    inSelectLevel = 1;
                } else {
                    StartFadeOut(1);
                }
            }
        }
    }
}

// 0x0042AC50, 1213 bytes.
void XSelectLevelObject::Render()
{
    if (state == NON) {
        return;
    }

    camera->DoSetTransform();
    background->Render();
    map->Render();

    int centerX = app->GetScreenWidth() / 2;
    int topY = app->GetScreenHeight() / 2 - 240;

    int x = centerX - 0x4C;
    int y = topY + 0x28;
    textLevel->SetPosition(x, y);
    textLevel->Draw(app->GetBackBuffer());

    x += textLevel->GetWidth() + 8;
    numLevel->SetNumber(app->currentLevel + 1);
    numLevel->SetPosition(x, y + 2);
    numLevel->Draw(app->GetBackBuffer());

    x = centerX - levelLED->GetWidth() / 4 * app->maxLevel / 2;
    y += numLevel->GetHeight() + 0x14;

    for (int level = 0; level <= app->maxLevel; level++) {
        if (level == app->currentLevel) {
            levelLED->SetCurrentFrame(2);
        } else if (level <= app->maxSelLevel) {
            levelLED->SetCurrentFrame(1);
        } else {
            levelLED->SetCurrentFrame(0);
        }
        levelLED->SetPosition(x, y);
        levelLED->Draw(app->GetBackBuffer());
        x += levelLED->GetWidth() / 4;
    }

    int menuY = topY + 0x15E;
    for (int i = 0; i < 3; i++) {
        menuItems[i]->SetCenterPos(centerX, menuY);
        menuItems[i]->SetCurrentFrame(cursorPos == i);
        menuItems[i]->Draw(app->GetBackBuffer());
        menuY += menuItems[i]->GetHeight() + 8;
    }

    if (!IsRegistered()) {
        textUnreg->SetPosition(centerX - 0x138, topY + 0x1CE);
        textUnreg->Draw(app->GetBackBuffer());
    }

    if (state == PROCESS) {
        if (inSelectLevel != 0) {
            int x1 = textLevel->GetPosIntX() - cursor2Right->GetWidth() / 2 - 6;
            int x2 = numLevel->GetPosX() + numLevel->GetWidth() + 6;
            int cy = textLevel->GetPosIntY() + 2;
            cursor2Right->SetPosition(x1, cy);
            cursor2Right->Draw(app->GetBackBuffer());
            cursor2Left->SetPosition(x2, cy);
            cursor2Left->Draw(app->GetBackBuffer());
        } else {
            if (cursorPos >= 0) {
                int x1 = menuItems[cursorPos]->GetPosIntX() - cursorLeft->GetWidth() / 2 - 4;
                int x2 = menuItems[cursorPos]->GetPosIntX() + menuItems[cursorPos]->GetWidth() + 4;
                int cy = menuItems[cursorPos]->GetPosIntY() + 2;
                cursorLeft->SetPosition(x1, cy);
                cursorLeft->Draw(app->GetBackBuffer());
                cursorRight->SetPosition(x2, cy);
                cursorRight->Draw(app->GetBackBuffer());
            }
        }
    }

    if (state == FADEIN || state == FADEOUT) {
        fadeLeft->Draw(app->GetBackBuffer());
        fadeRight->Draw(app->GetBackBuffer());
    }
}

// 0x0042B1F0, 1278 bytes.
XTitleObject::XTitleObject(XApplication* _app)
    : XProcessObject(_app)
{
    camera = NULL;
    pac = NULL;
    background = NULL;
    cursorLeft = NULL;
    cursorRight = NULL;
    textHopmon = NULL;
    textCopyright = NULL;
    textUnreg = NULL;
    cursorPos = -1;

    app->GetMidiList()->Play(0);
    app->GetSurfaceList()->CreateSurface(0xD0, 1);
    app->GetSurfaceList()->CreateSurface(0xD1, 1);
    app->GetSurfaceList()->CreateSurface(0xDB, 1);
    app->GetSurfaceList()->CreateSurface(0xF8, 1);
    app->GetSurfaceList()->CreateSurface(0xF7, 1);
    app->GetSurfaceList()->CreateSurface(0xCD, 1);
    app->GetSurfaceList()->CreateSurface(0xC9, 1);

    camera = new XCamera(app);
    camera->ResetForTitle();

    pac = new JObject3DX(app, 0x94);
    pac->RotateBy(JAXIS_X, -1.5707964f);
    pac->ScaleBy(170.0f);

    background = new XTitleBackground(app);
    cursorLeft = new JSprite(app->GetSurfaceList(), 0xD0, 2);
    cursorRight = new JSprite(app->GetSurfaceList(), 0xD1, 2);
    textHopmon = new JSprite(app->GetSurfaceList(), 0xDB, 1);
    textCopyright = new JSprite(app->GetSurfaceList(), 0xF8, 1);
    textUnreg = new JSprite(app->GetSurfaceList(), 0xF7, 1);
    menuItems[0] = new JSprite(app->GetSurfaceList(), 0xCD, 2);
    menuItems[1] = new JSprite(app->GetSurfaceList(), 0xC9, 2);

    StartFadeIn(0);
}

// 0x0042B6F0, 816 bytes.
XTitleObject::~XTitleObject()
{
    SAFE_DELETE(camera);
    SAFE_DELETE(pac);
    SAFE_DELETE(background);
    SAFE_DELETE(cursorLeft);
    SAFE_DELETE(cursorRight);
    SAFE_DELETE(textHopmon);
    SAFE_DELETE(textCopyright);
    SAFE_DELETE(textUnreg);

    for (int i = 0; i < 2; i++) {
        SAFE_DELETE(menuItems[i]);
    }

    app->GetSurfaceList()->DestroySurface(0xD0);
    app->GetSurfaceList()->DestroySurface(0xD1);
    app->GetSurfaceList()->DestroySurface(0xDB);
    app->GetSurfaceList()->DestroySurface(0xF8);
    app->GetSurfaceList()->DestroySurface(0xF7);
    app->GetSurfaceList()->DestroySurface(0xCD);
    app->GetSurfaceList()->DestroySurface(0xC9);
}

// 0x0042BA20, 806 bytes.
void XTitleObject::ProcessFrame(unsigned long frameCount)
{
    if (state == NON) {
        return;
    }

    tick += frameCount;

    if (state == FADEIN) {
        ProcessFadeIn();
        if (tick >= 120) {
            cursorPos = 0;
            StartProcess(0);
        }
    } else if (state == FADEOUT) {
        ProcessFadeOut();
        if (tick >= 120) {
            if (processType == 0) {
                app->SetState(XApplication::START_SELLEVEL);
                state = NON;
                return;
            } else if (processType == 1) {
                app->SetState(XApplication::EXIT);
                state = NON;
                return;
            }
        }
    }

    background->ProcessFrame(frameCount);
    pac->RotateBy(JAXIS_Y, (float)frameCount * 0.010471975f);

    if (state == PROCESS) {
        int rem = tick % 50;
        int frame = (rem < 25) ? 0 : 1;
        cursorLeft->SetCurrentFrame(frame);
        cursorRight->SetCurrentFrame(frame);

        app->GetDInput()->UpdateInputState();

        int button;
        int up = app->GetDInput()->GetKeyboardPressed(0xC8) || app->GetDInput()->GetJoyDirPressed(0, 0);
        int down = app->GetDInput()->GetKeyboardPressed(0xD0) || app->GetDInput()->GetJoyDirPressed(0, 2);
        button = CheckButtonPressed(app->GetDInput());

        if (app->GetDInput()->GetKeyboardPressed(1)) {
            cursorPos = 1;
            button = 1;
        }

        if (up) {
            app->GetSoundList()->Play(0xEC, 100, 0);
            cursorPos--;
            if (cursorPos < 0) {
                cursorPos = 1;
            }
        } else if (down) {
            app->GetSoundList()->Play(0xEC, 100, 0);
            cursorPos++;
            if (cursorPos >= 2) {
                cursorPos = 0;
            }
        } else if (button) {
            app->GetSoundList()->Play(0xED, 100, 0);
            if (cursorPos == 0) {
                StartFadeOut(0);
            } else {
                StartFadeOut(1);
            }
        }
    }
}

// 0x0042BD50, 726 bytes.
void XTitleObject::Render()
{
    if (state == NON) {
        return;
    }

    camera->DoSetTransform();
    background->Render();
    pac->Render();

    int i;
    int y;
    int centerX = app->GetScreenWidth() / 2;
    int menuY;
    int centerY = app->GetScreenHeight() / 2 - 240;

    y = centerY + 80;
    textHopmon->SetCenterPos(centerX, y);
    textHopmon->Draw(app->GetBackBuffer());

    y += 70;
    textCopyright->SetCenterPos(centerX, y);
    textCopyright->Draw(app->GetBackBuffer());

    menuY = centerY + 390;
    for (i = 0; i < 2; i++) {
        menuItems[i]->SetCenterPos(centerX, menuY);
        menuItems[i]->SetCurrentFrame(cursorPos == i);
        menuItems[i]->Draw(app->GetBackBuffer());
        menuY += menuItems[i]->GetHeight() + 8;
    }

    if (!IsRegistered()) {
        textUnreg->SetPosition(centerX - 312, centerY + 462);
        textUnreg->Draw(app->GetBackBuffer());
    }

    if (state == PROCESS && cursorPos >= 0) {
        int cursorY, x1, x2;
        x1 = menuItems[cursorPos]->GetPosIntX() - cursorLeft->GetWidth() / 2 - 4;
        x2 = menuItems[cursorPos]->GetPosIntX() + menuItems[cursorPos]->GetWidth() + 4;
        cursorY = menuItems[cursorPos]->GetPosIntY() + 2;
        cursorLeft->SetPosition(x1, cursorY);
        cursorLeft->Draw(app->GetBackBuffer());
        cursorRight->SetPosition(x2, cursorY);
        cursorRight->Draw(app->GetBackBuffer());
    }

    if (state == FADEIN || state == FADEOUT) {
        fadeLeft->Draw(app->GetBackBuffer());
        fadeRight->Draw(app->GetBackBuffer());
    }
}

// 0x0042C030, 2072 bytes.
XEndingObject::XEndingObject(XApplication* _app)
    : XProcessObject(_app)
{
    camera = NULL;
    background = NULL;
    textCongra = NULL;
    textStaff = NULL;
    textCopyright = NULL;
    charaIndex = -1;

    app->GetMidiList()->Play(0);
    app->GetSurfaceList()->CreateSurface(0x144, 1);
    app->GetSurfaceList()->CreateSurface(0x145, 1);
    app->GetSurfaceList()->CreateSurface(0xF8, 1);

    camera = new XCamera(app);
    camera->ResetForTitle();

    background = new XTitleBackground(app);

    textCongra = new JSprite(app->GetSurfaceList(), 0x144, 1);
    textStaff = new JSprite(app->GetSurfaceList(), 0x145, 1);
    textCopyright = new JSprite(app->GetSurfaceList(), 0xF8, 1);

    chara[0] = new JObject3DX(app, 0x94);
    chara[1] = new JObject3DX(app, 0x9D);
    chara[2] = new JObject3DX(app, 0xA4);
    chara[3] = new JObject3DX(app, 0xAF);
    chara[4] = new JObject3DX(app, 0xD4);
    chara[5] = new JObject3DX(app, 0xAD);
    chara[6] = new JObject3DX(app, 0xAC);
    chara[7] = new JObject3DX(app, 0xA6);
    chara[8] = new JObject3DX(app, 0xB3);
    chara[9] = new JObject3DX(app, 0xB1);

    for (int i = 0; i < 10; i++) {
        chara[i]->RotateBy(JAXIS_X, -1.5707964f);
    }

    charaMaxScale[0] = 180.0f;
    charaMaxScale[1] = charaMaxScale[0] * 1.0f;
    charaMaxScale[2] = charaMaxScale[0] * 1.5f;
    charaMaxScale[3] = charaMaxScale[0] * 1.78f;
    charaMaxScale[4] = charaMaxScale[0] * 1.1f;
    charaMaxScale[5] = charaMaxScale[0] * 1.6f;
    charaMaxScale[6] = charaMaxScale[0] * 1.4f;
    charaMaxScale[7] = charaMaxScale[0] * 1.4f;
    charaMaxScale[8] = charaMaxScale[0] * 0.65f;
    charaMaxScale[9] = charaMaxScale[0] * 0.7f;

    StartFadeIn(0);
}

// 0x0042C850, 544 bytes.
XEndingObject::~XEndingObject()
{
    SAFE_DELETE(camera);
    SAFE_DELETE(background);
    SAFE_DELETE(textCongra);
    SAFE_DELETE(textStaff);
    SAFE_DELETE(textCopyright);

    for (int i = 0; i < 10; i++) {
        SAFE_DELETE(chara[i]);
    }

    app->GetSurfaceList()->DestroySurface(0x144);
    app->GetSurfaceList()->DestroySurface(0x145);
    app->GetSurfaceList()->DestroySurface(0xF8);
}

// 0x0042CA70, 653 bytes.
void XEndingObject::ProcessFrame(unsigned long frameCount)
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
            app->SetState(XApplication::START_LOGO);
            state = NON;
            return;
        }
    }

    background->ProcessFrame(frameCount);

    if (state == PROCESS) {
        unsigned int cycle = tick % 8000;
        charaIndex = cycle / 800;
        unsigned int segment = cycle % 800;

        float scale;
        if (segment < 100) {
            scale = ((float)segment / 100.0f) * charaMaxScale[charaIndex];
        } else if (segment < 700) {
            scale = charaMaxScale[charaIndex];
        } else {
            scale = ((float)(800 - segment) / 100.0f) * charaMaxScale[charaIndex];
        }
        if (scale < 1.0f) {
            scale = 1.0f;
        }

        chara[charaIndex]->ScaleTo(scale);
        chara[charaIndex]->RotateBy(JAXIS_Y, (float)frameCount * 0.013962634f);

        app->GetDInput()->UpdateInputState();

        int button = CheckButtonPressed(app->GetDInput());
        if (app->GetDInput()->GetKeyboardPressed(1)) {
            button = 1;
        }

        if (button) {
            app->GetSoundList()->Play(0xED, 100, 0);
            StartFadeOut(0);
        }
    }
}

// 0x0042CD00, 335 bytes.
void XEndingObject::Render()
{
    if (state == NON) {
        return;
    }

    camera->DoSetTransform();
    background->Render();

    if (charaIndex >= 0) {
        chara[charaIndex]->Render();
    }

    int centerX = app->GetScreenWidth() / 2;
    int centerY = app->GetScreenHeight() / 2;

    textCongra->SetCenterPos(centerX, centerY - 0xA0);
    textCongra->Draw(app->GetBackBuffer());

    textStaff->SetCenterPos(centerX, centerY + 0x3C);
    textStaff->Draw(app->GetBackBuffer());

    textCopyright->SetCenterPos(centerX, centerY + 0xBE);
    textCopyright->Draw(app->GetBackBuffer());

    if (state == FADEIN || state == FADEOUT) {
        fadeLeft->Draw(app->GetBackBuffer());
        fadeRight->Draw(app->GetBackBuffer());
    }
}

// 0x0042CE50, 168 bytes.
int CheckButtonPressed(JDInput* input)
{
    if (input->GetKeyboardPressed(156) || input->GetKeyboardPressed(28) ||
        input->GetKeyboardPressed(42) || input->GetKeyboardPressed(54) ||
        input->GetKeyboardPressed(29) || input->GetKeyboardPressed(157) ||
        input->GetKeyboardPressed(44) || input->GetKeyboardPressed(45) ||
        input->GetJoyButtonPressed(0, 0) || input->GetJoyButtonPressed(0, 1)) {
        return 1;
    }
    return 0;
}

