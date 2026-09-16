#pragma once
#include "common.h"
#include <d3d.h>
#include "D3DApp.h"

class JDInput;
class JMidiList;
class JNumber;
class JSoundList;
class JSprite;
class JStringList;
class JSurfaceList;
class XApplication;
class XProcessObject;

// PDB type 0x173F
class XFPSDisplay {
    friend struct LayoutCheck;
public:
    XFPSDisplay(XApplication* _app);
    virtual ~XFPSDisplay(); // vtable 0x00
    void Render();
private:
    XApplication* app; // 0x04
    JSprite* textFPS; // 0x08
    JNumber* numFPS; // 0x0C
};
ASSERT_SIZE(XFPSDisplay, 0x10);

// PDB type 0x12A0
class XApplication : public CD3DApplication {
    friend struct LayoutCheck;
public:
    enum STATE {
        NON = 0,
        EXIT = 1,
        IN_PROCESS = 2,
        START_LOGO = 3,
        START_TITLE = 4,
        START_SELLEVEL = 5,
        START_GAME = 6,
        START_REGINFO = 7,
        START_ENDING = 8,
    };

    XApplication();
    virtual ~XApplication(); // vtable 0x34
    void LoadWinRegData();
    void SaveWinRegData();
    void SetState(STATE _state);
    int GetCurrentWorld() const;
    JDInput* GetDInput() const;
    JMidiList* GetMidiList() const;
    JSoundList* GetSoundList() const;
    JSurfaceList* GetSurfaceList() const;
    virtual HRESULT Create(HINSTANCE hInst, char* strCmdLine);
    virtual long MsgProc(HWND hWnd, unsigned int uMsg, unsigned int wParam, long lParam);
    int currentLevel; // 0x1A8
    int maxSelLevel; // 0x1AC
    const int maxFreeLevel; // 0x1B0
    const int maxLevel; // 0x1B4
    JStringList* tempList; // 0x1B8
protected:
    virtual HRESULT OneTimeSceneInit();
    virtual HRESULT InitDeviceObjects();
    virtual HRESULT DeleteDeviceObjects();
    virtual HRESULT Render();
    virtual HRESULT FrameMove(unsigned long frameCount);
    virtual HRESULT FinalCleanup();
    virtual HRESULT RestoreSurfaces();
    static HRESULT ConfirmDevice(DDCAPS* pddDriverCaps, D3DDEVICEDESC7* pd3dDeviceDesc);
    void _DisableStickyKeys();
    void _RestoreStickyKeys();
    STICKYKEYS prevStikeyKeys; // 0x1BC
    FILTERKEYS prevFilterKeys; // 0x1C4
    TOGGLEKEYS prevToggleKeys; // 0x1DC
private:
    STATE state; // 0x1E4
    JDInput* dinput; // 0x1E8
    JMidiList* midiList; // 0x1EC
    JSoundList* soundList; // 0x1F0
    JSurfaceList* surfaceList; // 0x1F4
    XProcessObject* processObj; // 0x1F8
    XFPSDisplay* fpsDisplay; // 0x1FC
};
ASSERT_SIZE(XApplication, 0x200);
