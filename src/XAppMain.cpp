#include "XAppMain.h"
#include "JSprite.h"
#include "JAscii.h"
#include "JSurface.h"
#include "JStringList.h"
#include "XRegister.h"
#include "JWinReg.h"
#include "JString.h"
#include "JDInput.h"
#include "JMidi.h"
#include "JSound.h"
#include "D3DTextr.h"
#include "XGame.h"
#include "XTitle.h"
#include <d3dcaps.h>

// 0x00415E20, 231 bytes.
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, char* strCmdLine, int)
{
    void* mutex = CreateMutexA(NULL, TRUE, "JJsoft Hopmon Mutex");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        MessageBoxA(NULL, "HOPMON is already running.", "Hopmon", MB_ICONEXCLAMATION);
        return 0;
    }

    XApplication d3dApp;
    if (d3dApp.Create(hInst, strCmdLine) < 0) {
        return 0;
    }

    int res = d3dApp.Run();
    ReleaseMutex(mutex);
    CloseHandle(mutex);
    return res;
}

// 0x00416110, 432 bytes.
XApplication::XApplication()
    : currentLevel(0), maxSelLevel(0), maxFreeLevel(9), maxLevel(0x2C),
      tempList(new JStringList())
{
    state = NON;
    dinput = 0;
    midiList = 0;
    soundList = 0;
    surfaceList = 0;
    processObj = 0;
    fpsDisplay = 0;
    _DisableStickyKeys();
    m_strWinClassName = "SAITOGAMES_HOPMON_CLASS";
    m_strWindowTitle = "Hopmon";
    m_bAppUseZBuffer = TRUE;
    m_fnConfirmDevice = ConfirmDevice;
    surfaceList = new JSurfaceList(this);
    LoadWinRegData();
}

// 0x004162C0, 88 bytes.
XApplication::~XApplication()
{
    SaveWinRegData();
    DeleteRegisterInfo();
    _RestoreStickyKeys();
}

// 0x00416320, 253 bytes.
void XApplication::LoadWinRegData()
{
    LoadRegisterInfo("Software\\SaitoGames\\Hopmon");
    JWinReg reg;
    {
        JString keyName("Software\\SaitoGames\\Hopmon");
        reg.Create((HKEY)0x80000001, keyName);
    }
    unsigned short level = 0;
    {
        JString valueName("Level");
        reg.GetWord(valueName, level);
    }
    maxSelLevel = level;
    if (maxSelLevel < 0 || maxSelLevel > maxLevel) {
        maxSelLevel = 0;
    }
}

// 0x00416420, 203 bytes.
void XApplication::SaveWinRegData()
{
    SaveRegisterInfo("Software\\SaitoGames\\Hopmon");
    JWinReg reg;
    {
        JString keyName("Software\\SaitoGames\\Hopmon");
        reg.Create((HKEY)0x80000001, keyName);
    }
    unsigned short level = (unsigned short)maxSelLevel;
    {
        JString valueName("Level");
        reg.SetWord(valueName, level);
    }
}

// 0x004164F0, 25 bytes.
void XApplication::SetState(STATE _state)
{
    state = _state;
}

// 0x00416510, 28 bytes.
int XApplication::GetCurrentWorld() const
{
    return currentLevel / 0xF;
}

// 0x00416530, 20 bytes.
JDInput* XApplication::GetDInput() const
{
    return dinput;
}

// 0x00416550, 20 bytes.
JMidiList* XApplication::GetMidiList() const
{
    return midiList;
}

// 0x00416570, 20 bytes.
JSoundList* XApplication::GetSoundList() const
{
    return soundList;
}

// 0x00416590, 20 bytes.
JSurfaceList* XApplication::GetSurfaceList() const
{
    return surfaceList;
}

// 0x004165B0, 997 bytes.
HRESULT XApplication::Create(HINSTANCE hInst, char* strCmdLine)
{
    HRESULT hr = CD3DApplication::Create(hInst, strCmdLine);
    if (hr < 0) {
        return hr;
    }
    dinput = new JDInput(m_hWnd);
    if (!dinput->Create()) {
        return 0;
    }
    midiList = new JMidiList(m_hWnd);
    midiList->Append("Music01.mid");
    midiList->Append("Music02.mid");
    midiList->Append("Music03.mid");
    midiList->Append("Music04.mid");
    soundList = new JSoundList(m_hWnd);
    soundList->Create();
    soundList->CreateSound(0xF3, 4);
    soundList->CreateSound(0xF4, 4);
    soundList->CreateSound(0xF5, 4);
    soundList->CreateSound(0x13E, 4);
    soundList->CreateSound(0x13F, 2);
    soundList->CreateSound(0xEB, 1);
    soundList->CreateSound(0xEA, 1);
    soundList->CreateSound(0xFF, 1);
    soundList->CreateSound(0xEE, 1);
    soundList->CreateSound(0xEF, 1);
    soundList->CreateSound(0xF0, 1);
    soundList->CreateSound(0xF1, 1);
    soundList->CreateSound(0xE8, 1);
    soundList->CreateSound(0xE9, 1);
    soundList->CreateSound(0xFE, 1);
    soundList->CreateSound(0xEC, 1);
    soundList->CreateSound(0xED, 1);
    soundList->CreateSound(0x13D, 1);
    surfaceList->CreateSurface(0xC3, 0);
    surfaceList->CreateSurface(0xC2, 0);
    surfaceList->CreateSurface(0xBF, 1);
    surfaceList->CreateSurface(0xBB, 1);
    fpsDisplay = new XFPSDisplay(this);
    state = START_LOGO;
    return 0;
}

// 0x004169A0, 351 bytes.
long XApplication::MsgProc(HWND hWnd, unsigned int uMsg, unsigned int wParam, long lParam)
{
    switch (uMsg) {
    case 0x112:
        switch (wParam) {
        case 0xF100:
            return 0;
        }
        break;
    case 0x3B9:
        if (wParam == 1 && midiList) {
            midiList->Replay();
        }
        break;
    case 0x100:
        if (wParam == 0x50) {
            PostMessageA(m_hWnd, 0x111, 0x9C44, 0);
        }
        break;
    case 0x1C:
        if (wParam == 1) {
            if (midiList) {
                midiList->Resume();
            }
            if (dinput) {
                dinput->SetAcquireKeyboard(1);
                dinput->SetAcquireJoystick(1);
            }
        } else {
            if (midiList) {
                midiList->Pause();
            }
            if (dinput) {
                dinput->SetAcquireKeyboard(0);
                dinput->SetAcquireJoystick(0);
            }
        }
        break;
    }
    return CD3DApplication::MsgProc(hWnd, uMsg, wParam, lParam);
}

// 0x00416B00, 251 bytes.
HRESULT XApplication::OneTimeSceneInit()
{
    D3DTextr_CreateTextureFromResource(0x9C, 0, 0);
    D3DTextr_CreateTextureFromResource(0xFB, 0, 0);
    D3DTextr_CreateTextureFromResource(0xFC, 0, 0);
    D3DTextr_CreateTextureFromResource(0xA0, 0, 0);
    D3DTextr_CreateTextureFromResource(0xA1, 0, 0);
    D3DTextr_CreateTextureFromResource(0xA9, 0, 0);
    D3DTextr_CreateTextureFromResource(0xAA, 0, 0);
    D3DTextr_CreateTextureFromResource(0xB6, 0, 0);
    D3DTextr_CreateTextureFromResource(0xB7, 0, 0);
    D3DTextr_CreateTextureFromResource(0xDC, D3DTEXTR_TRANSPARENTBLACK, 0);
    D3DTextr_CreateTextureFromResource(0xA2, 0, 0);
    D3DTextr_CreateTextureFromResource(0xA3, 0, 0);
    D3DTextr_CreateTextureFromResource(0xC1, 0, 0);
    D3DTextr_CreateTextureFromResource(0x14C, 0, 0);
    return 0;
}

// 0x00416C00, 452 bytes.
HRESULT XApplication::InitDeviceObjects()
{
    surfaceList->RestoreAllSurfaces();
    D3DTextr_RestoreAllTextures(m_pd3dDevice);
    m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
    m_pd3dDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE, TRUE);
    m_pd3dDevice->SetRenderState(D3DRENDERSTATE_LIGHTING, TRUE);
    m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, TRUE);
    D3DMATRIX matProj;
#ifdef HOPMON_FHD
    FLOAT fAspect = (FLOAT)GetScreenHeight() / (FLOAT)GetScreenWidth();
    D3DUtil_SetProjectionMatrix(matProj, 0.7330383f, fAspect, 10.0f, 2000.0f);
#else
    D3DUtil_SetProjectionMatrix(matProj, 0.7330383f, 0.75f, 10.0f, 2000.0f);
#endif
    m_pd3dDevice->SetTransform(D3DTRANSFORMSTATE_PROJECTION, &matProj);
    D3DLIGHT7 light;
    D3DUtil_InitLight(light, D3DLIGHT_DIRECTIONAL, 0.77f, 0.1f, 0.5f, -1.0f, 0.5f);
    m_pd3dDevice->SetLight(0, &light);
    m_pd3dDevice->LightEnable(0, TRUE);
    m_pd3dDevice->SetRenderState(D3DRENDERSTATE_AMBIENT, 0xFF898989);
    return 0;
}

// 0x00416DD0, 32 bytes.
HRESULT XApplication::DeleteDeviceObjects()
{
    surfaceList->InvalidateAllSurfaces();
    D3DTextr_InvalidateAllTextures();
    return 0;
}

// 0x00416DF0, 155 bytes.
HRESULT XApplication::Render()
{
    m_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x2D1D62, 1.0f, 0);
    if (state == IN_PROCESS) {
        if (m_pd3dDevice->BeginScene() >= 0) {
            processObj->Render();
            fpsDisplay->Render();
            m_pd3dDevice->EndScene();
        }
    }
    return 0;
}

// 0x00417370, 498 bytes.
HRESULT XApplication::FinalCleanup()
{
    SAFE_DELETE(processObj);
    SAFE_DELETE(fpsDisplay);
    SAFE_DELETE(surfaceList);
    SAFE_DELETE(soundList);
    SAFE_DELETE(midiList);
    SAFE_DELETE(dinput);
    SAFE_DELETE(tempList);
    return 0;
}

// 0x00417570, 27 bytes.
HRESULT XApplication::RestoreSurfaces()
{
    surfaceList->RestoreAllSurfaces();
    return 0;
}

// 0x00417590, 86 bytes.
HRESULT XApplication::ConfirmDevice(DDCAPS* pddDriverCaps, D3DDEVICEDESC7* pd3dDeviceDesc)
{
    D3DPRIMCAPS* caps = &pd3dDeviceDesc->dpcTriCaps;
    if (!(caps->dwSrcBlendCaps & D3DPBLENDCAPS_ONE)) {
        return E_FAIL;
    }
    if (!(caps->dwDestBlendCaps & D3DPBLENDCAPS_ONE)) {
        return E_FAIL;
    }
    if (!(caps->dwTextureCaps & D3DPTEXTURECAPS_ALPHAPALETTE) &&
        !(caps->dwTextureCaps & D3DPTEXTURECAPS_ALPHA)) {
        return E_FAIL;
    }
    return S_OK;
}

// 0x004175F0, 506 bytes.
void XApplication::_DisableStickyKeys()
{
    memset(&prevStikeyKeys, 0, sizeof(STICKYKEYS));
    memset(&prevFilterKeys, 0, sizeof(FILTERKEYS));
    memset(&prevToggleKeys, 0, sizeof(TOGGLEKEYS));

    prevStikeyKeys.cbSize = sizeof(STICKYKEYS);
    if (SystemParametersInfoA(SPI_GETSTICKYKEYS, sizeof(STICKYKEYS), &prevStikeyKeys, 0)) {
        if (prevStikeyKeys.dwFlags & SKF_AVAILABLE) {
            STICKYKEYS sk = prevStikeyKeys;
            sk.dwFlags &= ~(SKF_STICKYKEYSON | SKF_HOTKEYACTIVE);
            SystemParametersInfoA(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &sk, 0);
        } else {
            prevStikeyKeys.cbSize = 0;
        }
    } else {
        prevStikeyKeys.cbSize = 0;
    }

    prevFilterKeys.cbSize = sizeof(FILTERKEYS);
    if (SystemParametersInfoA(SPI_GETFILTERKEYS, sizeof(FILTERKEYS), &prevFilterKeys, 0)) {
        if (prevFilterKeys.dwFlags & FKF_AVAILABLE) {
            FILTERKEYS fk = prevFilterKeys;
            fk.dwFlags &= ~(FKF_FILTERKEYSON | FKF_HOTKEYACTIVE);
            SystemParametersInfoA(SPI_SETFILTERKEYS, sizeof(FILTERKEYS), &fk, 0);
        } else {
            prevFilterKeys.cbSize = 0;
        }
    } else {
        prevFilterKeys.cbSize = 0;
    }

    prevToggleKeys.cbSize = sizeof(TOGGLEKEYS);
    if (SystemParametersInfoA(SPI_GETTOGGLEKEYS, sizeof(TOGGLEKEYS), &prevToggleKeys, 0)) {
        if (prevToggleKeys.dwFlags & TKF_AVAILABLE) {
            TOGGLEKEYS tk = prevToggleKeys;
            tk.dwFlags &= ~(TKF_TOGGLEKEYSON | TKF_HOTKEYACTIVE);
            SystemParametersInfoA(SPI_SETTOGGLEKEYS, sizeof(TOGGLEKEYS), &tk, 0);
        } else {
            prevToggleKeys.cbSize = 0;
        }
    } else {
        prevToggleKeys.cbSize = 0;
    }
}

// 0x004177F0, 112 bytes.
void XApplication::_RestoreStickyKeys()
{
    if (prevStikeyKeys.cbSize != 0) {
        SystemParametersInfoA(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &prevStikeyKeys, 0);
    }
    if (prevFilterKeys.cbSize != 0) {
        SystemParametersInfoA(SPI_SETFILTERKEYS, sizeof(FILTERKEYS), &prevFilterKeys, 0);
    }
    if (prevToggleKeys.cbSize != 0) {
        SystemParametersInfoA(SPI_SETTOGGLEKEYS, sizeof(TOGGLEKEYS), &prevToggleKeys, 0);
    }
}

// 0x00416E90, 1243 bytes.
HRESULT XApplication::FrameMove(unsigned long frameCount)
{
    if (frameCount > 0x14) {
        frameCount = 0x14;
    }
    if (state == IN_PROCESS) {
        processObj->ProcessFrame(frameCount);
    }
    if (state == START_LOGO) {
        SAFE_DELETE(processObj);
        processObj = new XLogoObject(this);
        state = IN_PROCESS;
    } else if (state == START_TITLE) {
        SAFE_DELETE(processObj);
        processObj = new XTitleObject(this);
        state = IN_PROCESS;
    } else if (state == START_SELLEVEL) {
        SAFE_DELETE(processObj);
        processObj = new XSelectLevelObject(this);
        state = IN_PROCESS;
    } else if (state == START_GAME) {
        SAFE_DELETE(processObj);
        processObj = new XGameObject(this);
        state = IN_PROCESS;
    } else if (state == START_REGINFO) {
        SAFE_DELETE(processObj);
        processObj = new XRegisterInfoObject(this);
        state = IN_PROCESS;
    } else if (state == START_ENDING) {
        SAFE_DELETE(processObj);
        processObj = new XEndingObject(this);
        state = IN_PROCESS;
    } else if (state == EXIT) {
        PostMessageA(m_hWnd, 0x111, 0x9C46, 0);
        state = NON;
    }
    return 0;
}

// 0x00415F10, 278 bytes.
XFPSDisplay::XFPSDisplay(XApplication* _app)
    : app(_app), textFPS(new JSprite(app->GetSurfaceList(), 0xBF, 1)),
      numFPS(new JNumber(app->GetSurfaceList(), 0xBB, 10))
{
    numFPS->SetDisplayZero(1);
    numFPS->SetNumDigits(3);
}

// 0x00416030, 20 bytes.
XFPSDisplay::~XFPSDisplay()
{
}

// 0x00416050, 184 bytes.
void XFPSDisplay::Render()
{
    if (app->DisplayFPS()) {
        int x = app->GetScreenWidth() - 0x5A;
        int y = 4;
        textFPS->SetPosition(x, y);
        textFPS->Draw(app->GetBackBuffer());
        x += textFPS->GetWidth() + 4;
        numFPS->SetPosition(x, y);
        numFPS->SetNumber(app->GetFPS());
        numFPS->Draw(app->GetBackBuffer());
    }
}
