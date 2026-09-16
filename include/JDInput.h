#pragma once
#include "common.h"
#define DIRECTINPUT_VERSION 0x0700  // Hopmon uses IDirectInput7
#include <dinput.h>

class JString;

// PDB type 0x167B
struct JINPUTSTATE {
    friend struct LayoutCheck;

    unsigned char keyState[256]; // 0x00
    DIJOYSTATE joyState[4]; // 0x100
};
ASSERT_SIZE(JINPUTSTATE, 0x240);

// PDB type 0x16EA
class JDInput {
    friend struct LayoutCheck;
public:
    JDInput(HWND _hWind);
    virtual ~JDInput(); // vtable 0x00
    int Create();
    void Destroy();
    void UpdateInputState();
    void SetAcquireKeyboard(int acquire);
    int GetKeyboardState(int keyNo);
    int GetKeyboardPressed(int keyNo);
    int GetKeyboardReleased(int keyNo);
    void GetKeyboardBuffer(JString& inputText);
    void ClearKeyboardBuffer();
    void SetAcquireJoystick(int acquire);
    int GetJoyButtonState(int joyNo, int buttonNo);
    int GetJoyButtonPressed(int joyNo, int buttonNo);
    int GetJoyButtonReleased(int joyNo, int buttonNo);
    int GetJoyDirState(int joyNo, int joyDir);
    int GetJoyDirPressed(int joyNo, int joyDir);
    int GetJoyDirReleased(int joyNo, int joyDir);
protected:
    int _CreateKeyboard();
    int _CreateJoystick();
    int _GetJoyDirStatePrev(int joyNo, int joyDir);
private:
    HWND hWind; // 0x04
    IDirectInput7A* lpInput; // 0x08
    IDirectInputDevice7A* lpKeyboard; // 0x0C
    IDirectInputDevice7A* lpJoystick[4]; // 0x10
    JINPUTSTATE inputState1; // 0x20
    JINPUTSTATE inputState2; // 0x260
    int acquireKeyboard; // 0x4A0
    int acquireJoystick; // 0x4A4
};
ASSERT_SIZE(JDInput, 0x4A8);

char ConvertKeyCodeToAscii(unsigned long keyCode);
int __stdcall EnumJoysticksCallback(const DIDEVICEINSTANCEA* deviceInst, void* pContext);
