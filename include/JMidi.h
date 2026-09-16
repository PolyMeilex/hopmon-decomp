#pragma once
#include "common.h"

class JStringList;

// PDB type 0x174E
class JMidiList {
    friend struct LayoutCheck;

    JMidiList(const JMidiList&);
    JMidiList();
public:
    JMidiList(HWND _hWind);
    virtual ~JMidiList(); // vtable 0x00
    void Append(const char* midiFileName);
    void Play(int index);
    void Replay();
    void Pause();
    void Resume();
    void Stop();
private:
    HWND hWind; // 0x04
    int paused; // 0x08
    int currentMidiIndex; // 0x0C
    JStringList* midiFileList; // 0x10
    const JMidiList& operator=(const JMidiList&);
};
ASSERT_SIZE(JMidiList, 0x14);

int PlayMidi(HWND hWnd, const char* sFileName);
int PauseMidi();
int ResumeMidi(HWND hWnd);
int StopMidi();
int ReplayMidi(HWND hWnd);
