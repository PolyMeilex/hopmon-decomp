#include "JDInput.h"
#include "JString.h"
#include <string.h>

static HWND g_hWind;
static unsigned char g_numJoysticks;
static IDirectInputDevice7A* g_lpJoystick[4];

// 0x0040D900, 228 bytes.
JDInput::JDInput(HWND _hWind)
    : hWind(_hWind), lpInput(NULL), lpKeyboard(NULL), acquireKeyboard(1), acquireJoystick(1)
{
    memset(inputState1.keyState, 0, sizeof(inputState1));
    memset(inputState2.keyState, 0, sizeof(inputState2));
    for (int i = 0; i < 4; i++) {
        lpJoystick[i] = NULL;
    }
    g_hWind = hWind;
    g_numJoysticks = 0;
    for (int i = 0; i < 4; i++) {
        g_lpJoystick[i] = NULL;
    }
}

// 0x0040D9F0, 75 bytes.
JDInput::~JDInput()
{
    Destroy();
}

// 0x0040DA40, 101 bytes.
int JDInput::Create()
{
    HINSTANCE hinst = (HINSTANCE)GetWindowLongA(hWind, GWL_HINSTANCE);
    HRESULT hr = DirectInputCreateEx(hinst, 0x700, IID_IDirectInput7A, (void**)&lpInput, NULL);
    if (hr < 0) {
        return 0;
    }
    if (!_CreateKeyboard()) {
        return 0;
    }
    _CreateJoystick();
    return 1;
}

// 0x0040DAB0, 192 bytes.
void JDInput::Destroy()
{
    SetAcquireJoystick(0);
    SetAcquireKeyboard(0);
    for (int i = 0; i < 4; i++) {
        if (lpJoystick[i] != NULL) {
            lpJoystick[i]->Release();
            lpJoystick[i] = NULL;
        }
    }
    if (lpKeyboard != NULL) {
        lpKeyboard->Release();
        lpKeyboard = NULL;
    }
    if (lpInput != NULL) {
        lpInput->Release();
        lpInput = NULL;
    }
}

// 0x0040DB70, 467 bytes.
void JDInput::UpdateInputState()
{
    HRESULT hr;
    inputState2 = inputState1;
    memset(inputState1.keyState, 0, sizeof(inputState1));
    if (lpKeyboard != NULL) {
        hr = lpKeyboard->GetDeviceState(sizeof(inputState1.keyState), inputState1.keyState);
        if (acquireKeyboard != 0 && (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED)) {
            lpKeyboard->Acquire();
            lpKeyboard->GetDeviceState(sizeof(inputState1.keyState), inputState1.keyState);
        }
    }
    for (int i = 0; i < 4; i++) {
        if (lpJoystick[i] != NULL) {
            hr = lpJoystick[i]->Poll();
            if (hr < 0) {
                if (acquireJoystick != 0) {
                    lpJoystick[i]->Acquire();
                }
                continue;
            }
            hr = lpJoystick[i]->GetDeviceState(sizeof(inputState1.joyState[i]), &inputState1.joyState[i]);
            if (acquireJoystick != 0 && (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED)) {
                lpJoystick[i]->Acquire();
                lpJoystick[i]->GetDeviceState(sizeof(inputState1.joyState[i]), &inputState1.joyState[i]);
            }
        }
    }
}

// 0x0040DD50, 88 bytes.
void JDInput::SetAcquireKeyboard(int acquire)
{
    acquireKeyboard = acquire;
    if (lpKeyboard != NULL) {
        if (acquireKeyboard != 0) {
            lpKeyboard->Acquire();
        } else {
            lpKeyboard->Unacquire();
        }
    }
}

// 0x0040DDB0, 41 bytes.
int JDInput::GetKeyboardState(int keyNo)
{
    if (lpKeyboard == NULL) {
        return 0;
    }
    return inputState1.keyState[keyNo] & 0x80;
}

// 0x0040DDE0, 86 bytes.
int JDInput::GetKeyboardPressed(int keyNo)
{
    if (lpKeyboard == NULL) {
        return 0;
    }
    int result;
    if ((inputState2.keyState[keyNo] & 0x80) == 0 && (inputState1.keyState[keyNo] & 0x80) != 0) {
        result = 1;
    } else {
        result = 0;
    }
    return result;
}

// 0x0040DE40, 86 bytes.
int JDInput::GetKeyboardReleased(int keyNo)
{
    if (lpKeyboard == NULL) {
        return 0;
    }
    int result;
    if ((inputState2.keyState[keyNo] & 0x80) != 0 && (inputState1.keyState[keyNo] & 0x80) == 0) {
        result = 1;
    } else {
        result = 0;
    }
    return result;
}

// 0x0040DEA0, 460 bytes.
void JDInput::GetKeyboardBuffer(JString& inputText)
{
    if (lpKeyboard == NULL) {
        return;
    }
    unsigned long count = (unsigned long)-1;
    HRESULT hr = lpKeyboard->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), NULL, &count, DIGDD_PEEK);
    if (hr < 0) {
        if (acquireKeyboard != 0 && hr == DIERR_INPUTLOST) {
            lpKeyboard->Acquire();
        }
        return;
    }
    DIDEVICEOBJECTDATA* data = new DIDEVICEOBJECTDATA[count];
    hr = lpKeyboard->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), data, &count, 0);
    if (hr < 0) {
        if (acquireKeyboard != 0 && hr == DIERR_INPUTLOST) {
            lpKeyboard->Acquire();
        }
    } else {
        {
            JString empty("");
            inputText = empty;
        }
        for (unsigned long i = 0; i < count; i++) {
            if ((data[i].dwData & 0x80) != 0) {
                char ch = ConvertKeyCodeToAscii(data[i].dwOfs);
                if (ch != '\0') {
                    inputText += ch;
                    if (inputText.Length() >= 0x10) {
                        break;
                    }
                }
            }
        }
        delete[] data;
    }
}

// 0x0040E070, 134 bytes.
void JDInput::ClearKeyboardBuffer()
{
    if (lpKeyboard == NULL) {
        return;
    }
    for (unsigned int i = 0; i < 10; i++) {
        JString unused;
        GetKeyboardBuffer(unused);
    }
}

// 0x0040E100, 136 bytes.
void JDInput::SetAcquireJoystick(int acquire)
{
    acquireJoystick = acquire;
    for (int i = 0; i < 4; i++) {
        if (lpJoystick[i] != NULL) {
            if (acquireJoystick != 0) {
                lpJoystick[i]->Acquire();
            } else {
                lpJoystick[i]->Unacquire();
            }
        }
    }
}

// 0x0040E190, 57 bytes.
int JDInput::GetJoyButtonState(int joyNo, int buttonNo)
{
    if (lpJoystick[joyNo] == NULL) {
        return 0;
    }
    return inputState1.joyState[joyNo].rgbButtons[buttonNo] & 0x80;
}

// 0x0040E1D0, 110 bytes.
int JDInput::GetJoyButtonPressed(int joyNo, int buttonNo)
{
    if (lpJoystick[joyNo] == NULL) {
        return 0;
    }
    int result;
    if ((inputState2.joyState[joyNo].rgbButtons[buttonNo] & 0x80) == 0 &&
        (inputState1.joyState[joyNo].rgbButtons[buttonNo] & 0x80) != 0) {
        result = 1;
    } else {
        result = 0;
    }
    return result;
}

// 0x0040E240, 110 bytes.
int JDInput::GetJoyButtonReleased(int joyNo, int buttonNo)
{
    if (lpJoystick[joyNo] == NULL) {
        return 0;
    }
    int result;
    if ((inputState2.joyState[joyNo].rgbButtons[buttonNo] & 0x80) != 0 &&
        (inputState1.joyState[joyNo].rgbButtons[buttonNo] & 0x80) == 0) {
        result = 1;
    } else {
        result = 0;
    }
    return result;
}

// 0x0040E2B0, 237 bytes.
int JDInput::GetJoyDirState(int joyNo, int joyDir)
{
    if (lpJoystick[joyNo] == NULL) {
        return 0;
    }
    if (joyDir == 0) {
        return inputState1.joyState[joyNo].lY < -800;
    } else if (joyDir == 2) {
        return inputState1.joyState[joyNo].lY > 800;
    } else if (joyDir == 3) {
        return inputState1.joyState[joyNo].lX < -800;
    } else if (joyDir == 1) {
        return inputState1.joyState[joyNo].lX > 800;
    }
    return 0;
}

// 0x0040E3A0, 91 bytes.
int JDInput::GetJoyDirPressed(int joyNo, int joyDir)
{
    if (lpJoystick[joyNo] == NULL) {
        return 0;
    }
    int result;
    if (!_GetJoyDirStatePrev(joyNo, joyDir) && GetJoyDirState(joyNo, joyDir)) {
        result = 1;
    } else {
        result = 0;
    }
    return result;
}

// 0x0040E400, 91 bytes.
int JDInput::GetJoyDirReleased(int joyNo, int joyDir)
{
    if (lpJoystick[joyNo] == NULL) {
        return 0;
    }
    int result;
    if (_GetJoyDirStatePrev(joyNo, joyDir) && !GetJoyDirState(joyNo, joyDir)) {
        result = 1;
    } else {
        result = 0;
    }
    return result;
}

// 0x0040E460, 269 bytes.
int JDInput::_CreateKeyboard()
{
    HRESULT hr = lpInput->CreateDeviceEx(GUID_SysKeyboard, IID_IDirectInputDevice7A, (void**)&lpKeyboard, NULL);
    if (hr < 0) {
        return 0;
    }
    hr = lpKeyboard->SetDataFormat(&c_dfDIKeyboard);
    if (hr < 0) {
        return 0;
    }
    hr = lpKeyboard->SetCooperativeLevel(hWind, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
    if (hr < 0) {
        return 0;
    }
    DIPROPDWORD dipdw = { { sizeof(DIPROPDWORD), sizeof(DIPROPHEADER), 0, DIPH_DEVICE }, 0x100 };
    lpKeyboard->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph);
    if (acquireKeyboard != 0) {
        lpKeyboard->Acquire();
    }
    return 1;
}

// 0x0040E570, 159 bytes.
int JDInput::_CreateJoystick()
{
    HRESULT hr = lpInput->EnumDevices(DIDEVTYPE_JOYSTICK, EnumJoysticksCallback, lpInput, DIEDFL_ATTACHEDONLY);
    if (hr < 0) {
        return 0;
    }
    for (int i = 0; i < (int)(unsigned int)g_numJoysticks; i++) {
        lpJoystick[i] = g_lpJoystick[i];
        if (acquireJoystick != 0) {
            lpJoystick[i]->Acquire();
        }
    }
    return 1;
}

// 0x0040E610, 237 bytes.
int JDInput::_GetJoyDirStatePrev(int joyNo, int joyDir)
{
    if (lpJoystick[joyNo] == NULL) {
        return 0;
    }
    if (joyDir == 0) {
        return inputState2.joyState[joyNo].lY < -800;
    } else if (joyDir == 2) {
        return inputState2.joyState[joyNo].lY > 800;
    } else if (joyDir == 3) {
        return inputState2.joyState[joyNo].lX < -800;
    } else if (joyDir == 1) {
        return inputState2.joyState[joyNo].lX > 800;
    }
    return 0;
}

// 0x0040E700, 618 bytes.
char ConvertKeyCodeToAscii(unsigned long keyCode)
{
    switch (keyCode) {
    case 0x1e: return 'A';
    case 0x30: return 'B';
    case 0x2e: return 'C';
    case 0x20: return 'D';
    case 0x12: return 'E';
    case 0x21: return 'F';
    case 0x22: return 'G';
    case 0x23: return 'H';
    case 0x17: return 'I';
    case 0x24: return 'J';
    case 0x25: return 'K';
    case 0x26: return 'L';
    case 0x32: return 'M';
    case 0x31: return 'N';
    case 0x18: return 'O';
    case 0x19: return 'P';
    case 0x10: return 'Q';
    case 0x13: return 'R';
    case 0x1f: return 'S';
    case 0x14: return 'T';
    case 0x16: return 'U';
    case 0x2f: return 'V';
    case 0x11: return 'W';
    case 0x2d: return 'X';
    case 0x15: return 'Y';
    case 0x2c: return 'Z';
    case 0xb: case 0x52: return '0';
    case 2: case 0x4f: return '1';
    case 3: case 0x50: return '2';
    case 4: case 0x51: return '3';
    case 5: case 0x4b: return '4';
    case 6: case 0x4c: return '5';
    case 7: case 0x4d: return '6';
    case 8: case 0x47: return '7';
    case 9: case 0x48: return '8';
    case 10: case 0x49: return '9';
    case 0xc: case 0x4a: return '-';
    case 0x34: case 0x53: return '.';
    case 0x39: return ' ';
    case 0xe: case 0xcb: return '\b';
    case 0x1c: case 0x9c: return '\n';
    default: return '\0';
    }
}

// 0x0040E970, 470 bytes.
int __stdcall EnumJoysticksCallback(const DIDEVICEINSTANCEA* deviceInst, void* pContext)
{
    IDirectInput7A* input = (IDirectInput7A*)pContext;
    IDirectInputDevice7A* device = NULL;
    HRESULT hr = input->CreateDeviceEx(deviceInst->guidInstance, IID_IDirectInputDevice2A, (void**)&device, NULL);
    if (hr < 0) {
        return DIENUM_CONTINUE;
    }
    hr = device->SetDataFormat(&c_dfDIJoystick);
    if (hr < 0) {
        return DIENUM_CONTINUE;
    }
    hr = device->SetCooperativeLevel(g_hWind, DISCL_FOREGROUND | DISCL_EXCLUSIVE);
    if (hr < 0) {
        return DIENUM_CONTINUE;
    }
    DIPROPRANGE range;
    range.diph.dwSize = sizeof(DIPROPRANGE);
    range.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    range.diph.dwHow = DIPH_BYOFFSET;
    range.diph.dwObj = DIJOFS_X;
    range.lMin = -1000;
    range.lMax = 1000;
    hr = device->SetProperty(DIPROP_RANGE, &range.diph);
    if (hr < 0) {
        return DIENUM_CONTINUE;
    }
    range.diph.dwObj = DIJOFS_Y;
    hr = device->SetProperty(DIPROP_RANGE, &range.diph);
    if (hr < 0) {
        return DIENUM_CONTINUE;
    }
    DIPROPDWORD dead;
    dead.diph.dwSize = sizeof(DIPROPDWORD);
    dead.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    dead.diph.dwHow = DIPH_BYOFFSET;
    dead.diph.dwObj = DIJOFS_X;
    dead.dwData = 100;
    hr = device->SetProperty(DIPROP_DEADZONE, &dead.diph);
    if (hr < 0) {
        return DIENUM_CONTINUE;
    }
    dead.diph.dwObj = DIJOFS_Y;
    hr = device->SetProperty(DIPROP_DEADZONE, &dead.diph);
    if (hr < 0) {
        return DIENUM_CONTINUE;
    }
    g_lpJoystick[g_numJoysticks] = device;
    g_numJoysticks++;
    if (g_numJoysticks >= 4) {
        return DIENUM_STOP;
    }
    return DIENUM_CONTINUE;
}
